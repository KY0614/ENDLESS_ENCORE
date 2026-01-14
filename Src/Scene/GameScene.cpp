#include <DxLib.h>
#include<EffekseerForDXLib.h>
#include "../Application.h"
#include "../Common/Fader.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Common/Geometry/Sphere.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Stage.h"
#include "EncountScene.h"
#include "GameScene.h"

namespace
{
	//スキップ判定時間
	const float SKIP_TIME = 2.0f; 
	//バトル中のZ位置制限
	const float BATTLE_STAGE_Z = 938.0f;
	//BGM音量
	const int EXPLORE_BGM_VOLUME = 60;
	const int BATTLE_BGM_VOLUME = 50;

	//カメラの各注視点オフセット値	
	const float CAMERA_PLAYER_HEAD_OFFSET_Y = 100.0f;	//プレイヤーの頭の高さ
	const float CAMERA_PLAYER_CHEST_OFFSET_Y = 70.0f;	//プレイヤーの胸の高さ
}

GameScene::GameScene(void)
{
	player_ = nullptr;
	enemy_ = nullptr;
	stage_ = nullptr;
	isFaseChange_ = false;
	state_ = STATE::NONE;

	skipTimer_ = 0.0f;
	isSkip_ = false;

	//状態管理
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&GameScene::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::EXPLORE, std::bind(&GameScene::ChangeStateExplore, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&GameScene::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&GameScene::ChangeStateBattle, this));
}

GameScene::~GameScene(void)
{
	
}

void GameScene::Init(void)
{
	//探索BGM
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);

	//戦闘BGM
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::BATTLE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::GAME_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::BATTLE, BATTLE_BGM_VOLUME);

	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();

	//プレイヤー
	player_ = std::make_shared<Player>();
	player_->Init();

	//敵
	enemy_ = std::make_shared<Enemy>(*player_);
	enemy_->Init();

	//演出シーン
	encountScene_ = std::make_unique<EncountScene>(*player_,*enemy_);
	encountScene_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	player_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetMistWallTransform().collider);

	//初期状態設定
	ChangeState(STATE::WAKE_UP);
}

void GameScene::Update(void)
{
	//更新ステップ
	stateUpdate_();
}

void GameScene::Draw(void)
{
	//更新ステップ
	stateDraw_();
}

void GameScene::Backstab(void)
{
	VECTOR backDir = enemy_->GetTransform().GetBack();
	//バックスタブ位置
	const float distance = 60.0f;	//敵から少し離す
	VECTOR target = VAdd(enemy_->GetTransform().pos, VScale(backDir, distance));
	//ダウン中のバックスタブ判定
	if (enemy_->GetIsDown() && enemy_->CheckBackstab())
	{
		if (player_->GetIsParry())
		{
			player_->SetPos(target);
			player_->SetBackstabRotY(enemy_->GetTransform().quaRot);
			player_->ChangeState(Player::STATE::BACKSTAB);
			enemy_->ChangeState(Enemy::STATE::BACKSTAB);
		}
	}
}

void GameScene::InitStateExplore(void)
{
	//敵とプレイヤーの状態設定
	enemy_->ChangeState(Enemy::STATE::NONE);
	player_->ChangeState(Player::STATE::PLAY);
	player_->Update(); //状態変更後すぐに更新しておく

	//フェードイン開始
	SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);
	//BGM再生
	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, EXPLORE_BGM_VOLUME);
	sound.Play(SoundManager::SOUND::EXPLORE);
}

void GameScene::InitStateBattle(void)
{
	//霧の壁有効化
	stage_->IsBattle();
	//敵とプレイヤーの状態設定
	enemy_->ChangeState(Enemy::STATE::MOVE);
	enemy_->SetIsEncount(true);
	enemy_->Update(); //状態変更後すぐに更新しておく
	player_->ChangeState(Player::STATE::PLAY);
	player_->Update(); //状態変更後すぐに更新しておく
	player_->AddCollider(stage_->GetMistWallTransform().collider);
	SoundManager& sound = SoundManager::GetInstance();
	sound.Play(SoundManager::SOUND::BATTLE);
}

void GameScene::ChangeState(STATE state)
{
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void GameScene::ChangeStateWakeUp(void)
{
	//カメラセットアップ
	//プレイヤーの右前方へ設定
	VECTOR pos = player_->GetTransform().pos;
	const float cameraDistance = 100.0f;
	pos = VAdd(pos, VScale(
		VAdd(player_->GetTransform().GetRight(),
			player_->GetTransform().GetForward()), cameraDistance));
	//注視点はプレイヤーの胸あたり
	VECTOR targetPos = player_->GetTransform().pos;
	targetPos.y += CAMERA_PLAYER_CHEST_OFFSET_Y;
	//クレーンアップ速度
	const float craneUpSpeed = 0.13f;
	mainCamera->SetCraneUpPos(pos, CAMERA_PLAYER_CHEST_OFFSET_Y,targetPos, craneUpSpeed);
	mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
	stateUpdate_ = std::bind(&GameScene::UpdateWakeUp, this);
	stateDraw_ = std::bind(&GameScene::DrawWakeUp, this);
}

void GameScene::ChangeStateExplore(void)
{
	InitStateExplore();
	stateUpdate_ = std::bind(&GameScene::UpdateExplore, this);
	stateDraw_ = std::bind(&GameScene::DrawExplore, this);
}

void GameScene::ChangeStateEncount(void)
{
	skipTimer_ = 0.0f;
	isSkip_ = false;
	stateUpdate_ = std::bind(&GameScene::UpdateEncount, this);
	stateDraw_ = std::bind(&GameScene::DrawEncount, this);
}

void GameScene::ChangeStateBattle(void)
{
	InitStateBattle();
	stateUpdate_ = std::bind(&GameScene::UpdateBattle, this);
	stateDraw_ = std::bind(&GameScene::DrawBattle, this);
}

void GameScene::UpdateWakeUp(void)
{
	InputManager& ins = InputManager::GetInstance();
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//スペースキー長押しでスキップ
	if (ins.IsInputPressed("Parry"))
	{
		isSkip_ = true;
		skipTimer_ += SceneManager::GetInstance().GetDeltaTime();
		if(skipTimer_ >= SKIP_TIME)
		{
			skipTimer_ = SKIP_TIME;
			fader.lock()->SetFade(Fader::STATE::FADE_OUT);
		}
	}
	else if (skipTimer_ >= SKIP_TIME)	//スキップ完了後、キーを離しても表示
	{
		skipTimer_ = SKIP_TIME;
		isSkip_ = true;
	}
	else //キーを離したらタイマーリセット
	{
		isSkip_ = false;
		skipTimer_ = 0.0f;
	}

	//プレイヤーの行動が終了したらフェードアウト開始
	if(player_->IsActionEnd() &&
		fader.lock()->GetState() != Fader::STATE::FADE_OUT)
	{
		fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	}
	//フェードアウトが完了したら探索状態へ遷移
	if (SceneManager::GetInstance().IsFadeOutEnd())
	{
		ChangeState(STATE::EXPLORE);
		return;
	}

	//更新
	stage_->Update();
	player_->Update();
}

void GameScene::DrawWakeUp(void)
{
	//ステージ描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();

	if (!isSkip_)return;
	SkipBarDraw();

#ifdef _DEBUG
	DrawString(0, 0, L"ゲーム開始", 0xffffff);
#endif
	
}

void GameScene::UpdateExplore(void)
{
	if (!SceneManager::GetInstance().IsFadeInEnd())return;
	
	//Z値850を超えるとエンカウント状態へ遷移
	//(ステージの手前端よりも奥)
	const float stagePosZ = 850.0f;
	if (player_->GetTransform().pos.z > stagePosZ)
	{
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();	//エンカウントシーン開始
		return;
	}

	//更新
	player_->Update();
	stage_->Update();
	encountScene_->Update();
}

void GameScene::DrawExplore(void)
{
	//ステージ描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();
#ifdef _DEBUG
	DrawString(0, 0, L"探索ステージ", 0xffffff);
#endif
	
}

void GameScene::UpdateEncount(void)
{
	InputManager& ins = InputManager::GetInstance();
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//スペースキー長押しでスキップ
	if (ins.IsInputPressed("Parry"))
	{
		isSkip_ = true;
		skipTimer_ += SceneManager::GetInstance().GetDeltaTime();
		if (skipTimer_ >= SKIP_TIME)
		{
			skipTimer_ = SKIP_TIME;
			fader.lock()->SetFade(Fader::STATE::FADE_OUT);
		}
	}
	else if (skipTimer_ >= SKIP_TIME)	//スキップ完了後、キーを離しても表示
	{
		skipTimer_ = SKIP_TIME;
		isSkip_ = true;
	}
	else //キーを離したらタイマーリセット
	{
		isSkip_ = false;
		skipTimer_ = 0.0f;
	}
	//エンカウントシーンが終了し、
	// フェードアウトが完了したらバトル状態へ遷移
	if ((encountScene_->IsFinished() || skipTimer_ >= SKIP_TIME) &&
		SceneManager::GetInstance().GetFader().lock()->IsEnd() &&
		SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_OUT)
	{
		enemy_->ChangeState(Enemy::STATE::WAIT);
		player_->ChangeState(Player::STATE::WAIT);
		//カメラ
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->SetTarget(&enemy_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		SceneManager::GetInstance().ResetFog();
		SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
	}

	//エンカウントシーンが終了したらバトル状態へ遷移
	//(フェードインが終わった後に遷移)
	if ((encountScene_->IsFinished() || skipTimer_ >= SKIP_TIME) &&
		SceneManager::GetInstance().GetFader().lock()->IsEnd() &&
		SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_IN)
	{
		ChangeState(STATE::BATTLE);
		return;
	}

	//各オブジェクト更新
	encountScene_->Update();
	player_->Update();
	enemy_->Update();
	stage_->Update();
}

void GameScene::DrawEncount(void)
{
	stage_->Draw();

	player_->Draw();
	enemy_->Draw();

	encountScene_->Draw();

	if (!isSkip_)return;
	SkipBarDraw();
#ifdef _DEBUG
	DrawString(0, 0, L"エンカウント", 0xffffff);
#endif
	
}

void GameScene::UpdateBattle(void)
{
	InputManager& ins = InputManager::GetInstance();

	//敵の体力が半分以下で敵召喚状態へ遷移
	//if (/*!enemy_->GetIsBackstab() &&*/
	//	enemy_->GetHP() <= enemy_->GetMaxHP() / 2.0f)
	//{
	//	SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_OUT);
	//	ChangeState(STATE::ENEMY_SUMMON);
	//	return;
	//}

	//Z位置制限（霧の壁の外にでないように)
	if(player_->GetTransform().pos.z < BATTLE_STAGE_Z)
	{
		player_->SetPosZ(BATTLE_STAGE_Z);
	}

	player_->Update();
	enemy_->Update();
	stage_->Update();

	if (ins.IsInputTriggered("Pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::PAUSE);
		return;
	}

	Backstab();
}

void GameScene::DrawBattle(void)
{
	//プレイヤー描画
	stage_->Draw();
	//敵描画
	enemy_->Draw();
	//プレイヤー描画
	player_->Draw();


	if(enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	player_->DrawDead();

	enemy_->DrawHPBar();
	player_->DrawHPBar();

#ifdef _DEBUG
	DrawString(0, 0, L"バトル", 0xffffff);
#endif

}

void GameScene::SkipBarDraw(void)
{
	const float progressRatio = skipTimer_ / SKIP_TIME;

	//画面座標
	const int GAUGE_X = Application::SCREEN_SIZE_X - 150;  // ゲージの左上のX座標
	const int GAUGE_Y = Application::SCREEN_SIZE_Y - 150;  // ゲージの左上のY座標
	const int GAUGE_W = 100; // ゲージの最大幅
	const int GAUGE_H = 20;  // ゲージの高さ

	// 現在のクールダウンゲージの幅
	const int currentGaugeWidth = (int)(GAUGE_W * progressRatio);

	// ゲージの色
	const int bgColor = 0x333333; // 背景色（灰色）
	int skipColor = 0x00FFFF; // スキップゲージの色
	std::wstring str = L"";
	if (skipTimer_ >= SKIP_TIME)
	{
		skipColor = 0x00FF00; str = L"スキップ完了";
	}
	else 
	{
		str = L"スキップ中...";
	}

	DrawFormatString(GAUGE_X + 2, GAUGE_Y - 28, 0x000000, str.c_str());
	DrawFormatString(GAUGE_X, GAUGE_Y - 30, 0xFFFFFF, str.c_str());
	// ゲージの背景を描画
	DrawBox(GAUGE_X, GAUGE_Y, GAUGE_X + GAUGE_W, GAUGE_Y + GAUGE_H, bgColor, true);
	// 進行中のゲージを描画
	DrawBox(GAUGE_X, GAUGE_Y, GAUGE_X + (int)(GAUGE_W * progressRatio), GAUGE_Y + GAUGE_H,
		skipColor, TRUE);

}