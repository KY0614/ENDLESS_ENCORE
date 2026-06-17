#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Common/Fader.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/GameSystem/InputManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Common/Geometry/Capsule.h"
#include "../Object/Character/Player.h"
#include "../Object/Character/EventScene/EncountPlayer.h"
#include "../Object/Character/EventScene/EncountEnemy.h"
#include "../Object/Character/Enemy.h"
#include "../Object/Character/SummonEnemy/FighterEnemy.h"
#include "../Object/Character/SummonEnemy/MageEnemy.h"
#include "../Object/Stage.h"
#include "../Object/Tutorial.h"
#include "../Object/UI/SkipBar.h"
#include "EncountScene.h"
#include "GameScene.h"

namespace
{
	//スキップ判定時間
	const float SKIP_TIME = 1.5f; 
	//バトル中のZ位置制限
	const float BATTLE_STAGE_Z = 938.0f;
	//BGM音量
	const int EXPLORE_BGM_VOLUME = 60;
	const int BATTLE_BGM_VOLUME = 50;

	//カメラの各注視点オフセット値	
	const float CAMERA_PLAYER_HEAD_OFFSET_Y = 100.0f;	//プレイヤーの頭の高さ
	const float CAMERA_PLAYER_CHEST_OFFSET_Y = 70.0f;	//プレイヤーの胸の高さ

	//UIの座標
	const Vector2 SKIP_BAR_POS = { Application::SCREEN_SIZE_X - 200, Application::SCREEN_SIZE_Y - 120 };

	//UIの大きさ
	const Vector2 SKIP_BAR_SIZE = { 100, 20 };
}

GameScene::GameScene(void)
{
	player_ = nullptr;
	encountPlayer_ = nullptr;
	enemy_ = nullptr;
	encountEnemy_ = nullptr;
	stage_ = nullptr;
	isFaseChange_ = false;
	state_ = STATE::NONE;

	skipTimer_ = 0.0f;
	isSkip_ = false;
	fontHandle_ = -1;
	//状態管理
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&GameScene::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::EXPLORE, std::bind(&GameScene::ChangeStateExplore, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&GameScene::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&GameScene::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&GameScene::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::LAST_BATTEL, std::bind(&GameScene::ChangeStateLastBattle, this));
}

GameScene::~GameScene(void)
{
	DeleteFontToHandle(fontHandle_);
}

void GameScene::Init(void)
{
	//サウンド初期化
	InitSound();

	//プレイヤー
	player_ = std::make_shared<Player>();
	player_->Init();

	//エンカウント演出用のプレイヤー
	encountPlayer_ = std::make_shared<EncountPlayer>();
	encountPlayer_->Init();

	//敵
	enemy_ = std::make_shared<Enemy>(*player_);
	enemy_->Init();

	//エンカウント演出用の敵
	encountEnemy_ = std::make_shared<EncountEnemy>();
	encountEnemy_->Init();

	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();
	//チュートリアル
	InitTutorial();

	//演出シーン
	encountScene_ = std::make_unique<EncountScene>(*encountEnemy_,*encountPlayer_);
	encountScene_->Init();

	//スキップUI
	skipBarUI_ = std::make_unique<SkipBar>(
		SkipBar::SkipBarUIInfo{
			SKIP_BAR_POS,	//位置
			SKIP_BAR_SIZE,	//サイズ
		},skipTimer_,SKIP_TIME );
	skipBarUI_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	//コライダー登録
	mainCamera->AddCollider(stage_->GetTransform().collider);
	player_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetTransform().collider);

	//フォントハンドルの取得
	fontHandle_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TUTORIAL_FONT).handleId_;

	//初期状態設定
	ChangeState(STATE::WAKE_UP);
}

void GameScene::Update(void)
{
	//更新ステップ
	stateUpdate_();

#ifdef _DEBUG
	//各オブジェクトのImGui更新
	ObjectUpdateImGui();
#endif // _DEBUG
}

void GameScene::Draw(void)
{
	//更新ステップ
	stateDraw_();
}

void GameScene::InitTutorial(void)
{
	//表示位置
	const Vector2 tutorialPos = { 50,Application::SCREEN_SIZE_Y / 2 - 100 };
	//移動
	const float stickInputTime = 1.5f;	//スティック操作の説明を表示する時間
	Tutorial::TutorialStep firstStep = {
		Tutorial::STATE::MOVE,
		tutorialPos,
		"WASDで移動",
		"左スティックで移動",
		stickInputTime,
		0
	};
	Tutorial::TutorialStep cameraStep = {
		Tutorial::STATE::CAMERA,
		tutorialPos,
		"矢印キーでカメラ操作",
		"右スティックでカメラ操作",
		stickInputTime,
		0
	};
	//ボタン入力回数
	const int buttonInputNum = 1;	
	Tutorial::TutorialStep jumpStep = {
		Tutorial::STATE::JUMP,
		tutorialPos,
		"Eキーでジャンプ",
		"Yボタンでジャンプ",
		0.0f,
		buttonInputNum
	};
	Tutorial::TutorialStep DodgeStep = {
		Tutorial::STATE::DODGE,
		tutorialPos,
		"左Shiftキーで回避",
		"Aボタンで回避",
		0.0f,
		buttonInputNum
	};
	Tutorial::TutorialStep ParryStep = {
		Tutorial::STATE::PARRY,
		tutorialPos,
		"Spaceキーでパリィ",
		"Bボタンでパリィ",
		0.0f,
		buttonInputNum
	};
	Tutorial::TutorialStep StageStep = {
		Tutorial::STATE::STAGE,
		tutorialPos,
		"ステージへ行ってみよう",
		"ステージへ行ってみよう",
		0.0f,
		0
	};
	//チュートリアル追加
	tutorial_ = std::make_shared<Tutorial>(firstStep);
	tutorial_->Init();
	tutorial_->AddTutorialStep(cameraStep);
	tutorial_->AddTutorialStep(jumpStep);
	tutorial_->AddTutorialStep(DodgeStep);
	tutorial_->AddTutorialStep(ParryStep);
	tutorial_->AddTutorialStep(StageStep);
}

void GameScene::InitSound(void)
{
	//探索BGM
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);

	//戦闘BGM
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::BATTLE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::BATTLE_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::BATTLE, BATTLE_BGM_VOLUME);
}

void GameScene::InitStateExplore(void)
{
	//敵とプレイヤーの状態設定
	enemy_->ChangeState(Enemy::STATE::NONE);
	player_->Play();
	player_->Update(); //状態変更後すぐに更新しておく

	//フェードイン開始
	SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);
}

void GameScene::InitStaeEncount(void)
{
	//敵とプレイヤーの状態設定
	enemy_->ChangeState(Enemy::STATE::NONE);
	player_->Wait();
	player_->Update(); //状態変更後すぐに更新しておく
	//スキップ関連初期化
	skipTimer_ = 0.0f;
	isSkip_ = false;
	//フェードイン開始
	SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);
}

void GameScene::InitStateBattle(void)
{
	//霧の壁有効化
	stage_->IsBattle();
	//敵とプレイヤーの状態設定
	enemy_->ChangeState(Enemy::STATE::MOVE);
	enemy_->SetIsEncount(true);
	enemy_->Update(); //状態変更後すぐに更新しておく
	enemy_->AddCollider(stage_->GetMistWallTransform().collider);
	player_->Play();
	player_->Update(); //状態変更後すぐに更新しておく
	player_->AddCollider(stage_->GetMistWallTransform().collider);
	SoundManager& sound = SoundManager::GetInstance();
	sound.Play(SoundManager::SOUND::BATTLE);
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
			player_->Backstab();
			enemy_->ChangeState(Enemy::STATE::BACKSTAB);
			VECTOR right = player_->GetTransform().GetRight();
			VECTOR back = player_->GetTransform().GetBack();
			VECTOR rightBackDir = VNorm(VAdd(back, right));
			VECTOR pos = VAdd(player_->GetTransform().pos, VScale(VNorm(rightBackDir), 100.0f));
			pos.y += 50.0f;
			VECTOR targetpos = enemy_->GetFramePos(L"mixamorig:Spine");
			mainCamera->SetBackstabCamera(pos, targetpos);
			mainCamera->ChangeMode(Camera::MODE::BACKSTAB);
		}
	}
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
	//探索状態の初期化
	InitStateExplore();

	//BGM再生
	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, EXPLORE_BGM_VOLUME);
	sound.Play(SoundManager::SOUND::EXPLORE);

	stateUpdate_ = std::bind(&GameScene::UpdateExplore, this);
	stateDraw_ = std::bind(&GameScene::DrawExplore, this);
}

void GameScene::ChangeStateEncount(void)
{
	//エンカウント状態の初期化
	InitStaeEncount();
	stateUpdate_ = std::bind(&GameScene::UpdateEncount, this);
	stateDraw_ = std::bind(&GameScene::DrawEncount, this);
}

void GameScene::ChangeStateBattle(void)
{
	InitStateBattle();
	stateUpdate_ = std::bind(&GameScene::UpdateBattle, this);
	stateDraw_ = std::bind(&GameScene::DrawBattle, this);
}

void GameScene::ChangeStateSummon(void)
{
	//敵の状態初期化
	enemy_->Init();
	enemy_->ChangeState(Enemy::STATE::WAIT);
	//近接型の敵
	fighterEnemy_ = std::make_unique<FighterEnemy>(*player_);
	fighterEnemy_->Init();
	fighterEnemy_->AddCollider(stage_->GetTransform().collider);
	//魔法型の敵
	mageEnemy_ = std::make_unique<MageEnemy>(*player_);
	mageEnemy_->Init();
	mageEnemy_->AddCollider(stage_->GetTransform().collider);
	//召喚位置設定
	VECTOR summonPos = enemy_->GetTransform().pos;
	//敵の下へ設定
	summonPos = VAdd(enemy_->GetTransform().pos, VScale(enemy_->GetTransform().GetDown(), 50.0f));
	VECTOR summonPos1 = VAdd(summonPos, VScale(enemy_->GetTransform().GetRight(), 100.0f));	//敵の右側
	VECTOR summonPos2 = VAdd(summonPos, VScale(enemy_->GetTransform().GetLeft(), 100.0f));	//敵の右側
	fighterEnemy_->SetSummonPos(summonPos1);
	fighterEnemy_->Summon();
	mageEnemy_->SetSummonPos(summonPos2);
	mageEnemy_->Summon();
	stateUpdate_ = std::bind(&GameScene::UpdateSummon, this);
	stateDraw_ = std::bind(&GameScene::DrawSummon, this);
}

void GameScene::ChangeStateLastBattle(void)
{
	//enemy_->ChangeState(Enemy::STATE::MOVE);
	stateUpdate_ = std::bind(&GameScene::UpdateLastBattle, this);
	stateDraw_ = std::bind(&GameScene::DrawLastBattle, this);
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
	tutorial_->Update();
	encountScene_->Update();
}

void GameScene::DrawExplore(void)
{
	//ステージ描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();

	//チュートリアル描画
	tutorial_->Draw();
	//UI描画
	player_->DrawBarUI();
}

void GameScene::UpdateEncount(void)
{
	InputManager& ins = InputManager::GetInstance();
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//スペースキー長押しでスキップ
	if (ins.IsInputPressed("Parry") && skipTimer_ < SKIP_TIME)
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

	//エンカウントシーンが終了もしくはスキップ時間を満たしていて、
	//かつフェード処理が完了している場合の処理
	if ((encountScene_->IsFinished() || skipTimer_ >= SKIP_TIME) &&
		SceneManager::GetInstance().GetFader().lock()->IsEnd())
	{
		//フェードアウトが完了したらバトル状態へ遷移
		if(SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_OUT)
		{
			//プレイヤーと敵を待機状態にする
			enemy_->ChangeState(Enemy::STATE::WAIT);
			player_->Wait();
			//カメラをプレイヤーに追従させる
			mainCamera->SetFollow(&player_->GetTransform());
			mainCamera->ChangeMode(Camera::MODE::FOLLOW);
			//フェードイン開始
			SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
		}
		//エンカウントシーンが終了したらバトル状態へ遷移
		//(フェードインが終わった後に遷移)	
		if(SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_IN)
		{
			ChangeState(STATE::BATTLE);
			return;
		}
	}

	//エンカウントシーン更新
	encountScene_->Update();
	//各オブジェクト更新
	player_->Update();
	encountPlayer_->Update();
	enemy_->Update();
	encountEnemy_->Update();
	stage_->Update();
}

void GameScene::DrawEncount(void)
{
	//ステージ描画
	stage_->Draw();
	//エンカウントシーンのフェード中は通常のプレイヤー描画を行う
	if (encountScene_->GetState() == EncountScene::STATE::FADE)
	{
		//プレイヤー描画
		player_->Draw();
	}
	//エンカウント演出用のプレイヤー描画
	encountPlayer_->Draw();
	//敵描画
	enemy_->Draw();
	//エンカウント演出用の敵描画
	encountEnemy_->Draw();

	//スキップUI描画
	if (!isSkip_)return;
	SkipBarDraw();
}

void GameScene::UpdateBattle(void)
{	
	if (enemy_->GetIsDead())
	{
		ChangeState(STATE::SUMMON);
	}

	//Z位置制限（霧の壁の外にでないように)
	if(player_->GetTransform().pos.z < BATTLE_STAGE_Z)
	{
		player_->SetPosZ(BATTLE_STAGE_Z);
	}

	//プレイヤーと敵のカプセルによる押し出し
	CollisionCupsule();

	//各オブジェクト更新
	player_->Update();	//プレイヤー
	enemy_->Update();	//敵
	stage_->Update();	//ステージ

	//バックスタブ判定
	Backstab();
}

void GameScene::DrawBattle(void)
{
	//ステージ描画
	stage_->Draw();
	//プレイヤー描画
	player_->Draw();
	//敵描画
	enemy_->Draw();

	//霧の壁描画
	stage_->DrawTranslucent();

	//UI描画
	enemy_->DrawBarUI();
	//UI描画
	player_->DrawBarUI();
}

void GameScene::UpdateSummon(void)
{
	//敵が両方とも召喚されたらラストバトルへ遷移
	if(fighterEnemy_->GetIsSummoned() &&
		mageEnemy_->GetIsSummoned())
	{
		ChangeState(STATE::LAST_BATTEL);
	}

	//各オブジェクト更新
	player_->Update();	//プレイヤー
	enemy_->Update();	//敵
	fighterEnemy_->Update();	//敵
	mageEnemy_->Update();	//敵
	stage_->Update();	//ステージ
}

void GameScene::DrawSummon(void)
{
	//ステージ描画
	stage_->Draw();
	//プレイヤー描画
	player_->Draw();
	//敵描画
	enemy_->Draw();
	fighterEnemy_->Draw();
	mageEnemy_->Draw();
}

void GameScene::UpdateLastBattle(void)
{
	//プレイヤーと敵のカプセルによる押し出し
	CollisionCupsule();

	//各オブジェクト更新
	player_->Update();	//プレイヤー
	//enemy_->Update();	//敵
	fighterEnemy_->Update();	//敵
	mageEnemy_->Update();	//敵
	stage_->Update();	//ステージ
}

void GameScene::DrawLastBattle(void)
{
	//ステージ描画
	stage_->Draw();
	//プレイヤー描画
	player_->Draw();
	//敵描画
	//enemy_->Draw();
	fighterEnemy_->Draw();	//近接型の敵描画
	mageEnemy_->Draw();		//遠距離型の敵描画

	//霧の壁描画
	stage_->DrawTranslucent();

	//敵が死んでいたらVictory描画
	if (enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	//YouDied描画
	player_->DrawDead();

	//召喚された敵のUI描画
	fighterEnemy_->DrawUI();	//近接型のUI描画
	mageEnemy_->DrawUI();		//遠距離型のUI描画
	//敵UI描画
	enemy_->DrawBarUI();
	//プレイヤーUI描画
	player_->DrawBarUI();
}

void GameScene::SkipBarDraw(void)
{
	//スキップの進行度を0.0f～1.0fで表す
	const float progressRatio = skipTimer_ / SKIP_TIME;

	//画面座標
	const int GAUGEX_OFFEST_X = 250; //ゲージのX座標オフセット
	const int GAUGEX_OFFEST_Y = 180; //ゲージのY座標オフセット
	const int GAUGE_X = Application::SCREEN_SIZE_X - GAUGEX_OFFEST_X;  // ゲージの左上のX座標
	const int GAUGE_Y = Application::SCREEN_SIZE_Y - GAUGEX_OFFEST_Y;  // ゲージの左上のY座標
	const int GAUGE_W = 100; // ゲージの最大幅
	const int GAUGE_H = 20;  // ゲージの高さ

	// 現在のクールダウンゲージの幅
	const int currentGaugeWidth = (int)(GAUGE_W * progressRatio);

	std::wstring str = L"";
	if (skipTimer_ >= SKIP_TIME)
	{
		str = L"スキップ完了";
	}
	else 
	{
		str = L"スキップ中...";
	}
	//文字影描画
	const int strShadowOffset = 2;
	DrawStringToHandle(
		GAUGE_X + strShadowOffset, GAUGE_Y + strShadowOffset,
		str.c_str(),
		0x000000,
		fontHandle_);
	//文字描画
	DrawStringToHandle(
		GAUGE_X, GAUGE_Y,
		str.c_str(),
		0xFFFFFF,
		fontHandle_);

	//ゲージ描画
	skipBarUI_->Draw();
}

void GameScene::UpdateImGui(void)
{
	ImGui::Text("GameScene");
	std::string stateStr = "STATE : ";
	switch (state_)
	{
	case GameScene::STATE::NONE:
		break;
	case GameScene::STATE::WAKE_UP:
		stateStr += "WAKE_UP";
		break;
	case GameScene::STATE::EXPLORE:
		stateStr += "EXPLORE";
		break;
	case GameScene::STATE::ENCOUNT:
		stateStr += "ENCOUNT";
		break;
	case GameScene::STATE::BATTLE:
		stateStr += "BATTLE";
		break;
	case GameScene::STATE::SUMMON:
		stateStr += "SUMMON";
		break;
	case GameScene::STATE::LAST_BATTEL:
		stateStr += "LAST_BATTLE";
		break;
	default:
		break;
	}
	ImGui::Text(stateStr.c_str());
	//状態遷移ボタン
	//探索
	if (ImGui::Button("Explore"))
	{
		InitStateExplore();
		ChangeState(STATE::EXPLORE);
	}
	//エンカウント演出
	if (ImGui::Button("Encount"))
	{
		enemy_->ChangeState(Enemy::STATE::NONE);
		encountPlayer_->Init();
		encountEnemy_->Init();
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();
	}
	//バトル開始位置
	const VECTOR battlePos = { 10.0, -217.0, 900.0 };
	//戦闘
	if (ImGui::Button("Battle"))
	{
		player_->SetPos(battlePos);
		InitStateBattle();
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		ChangeState(STATE::BATTLE);
	}
	//召喚
	if (ImGui::Button("Summon"))
	{
		player_->SetPos(battlePos);
		InitStateBattle();
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		ChangeState(STATE::SUMMON);
	}
	//ラストバトル
	if (ImGui::Button("LastBattle"))
	{
		player_->SetPos(battlePos);
		InitStateBattle();
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		ChangeState(STATE::LAST_BATTEL);
	}
}

void GameScene::ObjectUpdateImGui(void)
{
	ImGui::Begin("Object");

	if (ImGui::BeginTabBar("TabBar"))
	{
		//プレイヤーのImGui
		if (ImGui::BeginTabItem("Player"))
		{
			player_->UpdateImGui();
			ImGui::EndTabItem();
		}
		//プレイヤーのImGui
		if (ImGui::BeginTabItem("EncountPlayer"))
		{
			encountPlayer_->UpdateImGui();
			ImGui::EndTabItem();
		}
		if(state_ == STATE::BATTLE ||
			state_ == STATE::LAST_BATTEL)
		{
			//敵のImGui
			if (ImGui::BeginTabItem("Enemy"))
			{
				enemy_->UpdateImGui();
				ImGui::EndTabItem();
			}
		}
		//チュートリアルのImGui
		if (ImGui::BeginTabItem("Tutorial"))
		{
			tutorial_->UpdateImGui();
			ImGui::EndTabItem();
		}
		//エンカウントのImGui
		if (ImGui::BeginTabItem("Encount"))
		{
			encountScene_->UpdateImGui();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void GameScene::CollisionCupsule(void)
{
	//敵とプレイヤーの距離を作成
	VECTOR player2Enemy = VSub(enemy_->GetTransform().pos,player_->GetTransform().pos);
	float distance = VSize(player2Enemy);
	float hitDistance = enemy_->GetCapsule().GetRadius() + player_->GetCapsule().GetRadius();
	//距離が当たり判定距離より大きい場合は処理しない
	//if (distance > hitDistance)return;
	//プレイヤーの下端が敵の上端より高い場合は処理しない
	if (player_->GetCapsule().GetPosDown().y > enemy_->GetCapsule().GetPosTop().y)return;

	float sqDistance = VSquareSize(player2Enemy);
	if (sqDistance > 0.001f &&
		sqDistance < hitDistance * hitDistance)
	{
		float dis = sqrtf(sqDistance);
		float pushBackLength = hitDistance - distance;
		VECTOR pushBackDir = VScale(player2Enemy, -1.0f / dis);
		VECTOR pushBack = VScale(pushBackDir, pushBackLength);
		//敵を押し戻す
		VECTOR movedPos = VAdd(player_->GetTransform().pos, pushBack);
		player_->SetPos(movedPos);
	}
}