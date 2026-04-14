#include <DxLib.h>
#include<EffekseerForDXLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Common/Fader.h"
#include "../Common/Easing.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Stage.h"
#include "../Object/Tutorial.h"
#include "../Object/UI/BarUI.h"
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

	//スローモーション関連
	const float SLOW_MOTION_SPEED = 0.1f; //スローモーションの速度
	const float SLOW_MOTION_SPEED_ACCEL = 1.2f; //スローモーションの加速度
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
	slowMotionFrameCount_ = 0.0f;
	slowMotionFrame_ = 0.1f;
	fontHandle_ = -1;
	//状態管理
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&GameScene::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::EXPLORE, std::bind(&GameScene::ChangeStateExplore, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&GameScene::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&GameScene::ChangeStateBattle, this));
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

	//敵
	enemy_ = std::make_shared<Enemy>(*player_);
	enemy_->Init();

	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();
	//チュートリアル
	InitTutorial();

	//演出シーン
	encountScene_ = std::make_unique<EncountScene>(*player_,*enemy_);
	encountScene_->Init();

	//画面座標
	const int GAUGEX_OFFEST_X = 200; //ゲージのX座標オフセット
	const int GAUGEX_OFFEST_Y = 120; //ゲージのY座標オフセット
	const int GAUGE_X = Application::SCREEN_SIZE_X - GAUGEX_OFFEST_X;  // ゲージの左上のX座標
	const int GAUGE_Y = Application::SCREEN_SIZE_Y - GAUGEX_OFFEST_Y;  // ゲージの左上のY座標
	skipBarUI_ = std::make_unique<BarUI>();
	skipBarUI_->SetBarUISrc(ResourceManager::SRC::PLAYER_PARYY_BAR, ResourceManager::SRC::PLAYER_HP_BACK_BAR);
	skipBarUI_->Init();
	skipBarUI_->SetBarPos({ GAUGE_X, GAUGE_Y });
	skipBarUI_->SetActive(true);

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	//コライダー登録
	mainCamera->AddCollider(stage_->GetTransform().collider);
	player_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetTransform().collider);

	//画面比率に応じたフォントサイズ設定
	float screenAspect = SceneManager::GetInstance().GetScreenAspectRatio();
	const int fontSize = 32 * static_cast<int>(screenAspect);	//フォントサイズ(画面比率に合わせる)
	const int fontThick = 3;				//フォントの太さ
	fontHandle_ = CreateFontToHandle(L"しねきゃぷしょん", fontSize, fontThick, DX_FONTTYPE_ANTIALIASING);

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
	//チュートリアル
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
	enemy_->AddCollider(stage_->GetMistWallTransform().collider);
	player_->ChangeState(Player::STATE::PLAY);
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
			player_->ChangeState(Player::STATE::BACKSTAB);
			enemy_->ChangeState(Enemy::STATE::BACKSTAB);
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
#ifdef _DEBUG

	//スローモーション処理
	//if (encountScene_->IsSlowMotion() &&
	//	slowMotionFrameCount_++ > 60.0f)slowMotionFrameCount_ = 0.0f;

	//スローモーション中でなければ通常更新
	if (encountScene_->IsSlowMotion())
	{
		slowMotionFrame_ *= 1.02f; // 徐々に遅くする
		if (slowMotionFrame_ > 60.0f) // 完全停止
		{
			//終了処理など
			return;
		}
		slowMotionFrameCount_++;
		if (slowMotionFrame_ < 1.0f ||
			static_cast<int>(slowMotionFrameCount_) %
			static_cast<int>(slowMotionFrame_) != 0)
			return; // このフレームは処理しない
	}
#endif // _DEBUG
	//エンカウントシーン更新
	encountScene_->Update();
	//各オブジェクト更新
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
}

void GameScene::UpdateBattle(void)
{
	InputManager& ins = InputManager::GetInstance();

	//Z位置制限（霧の壁の外にでないように)
	if(player_->GetTransform().pos.z < BATTLE_STAGE_Z)
	{
		player_->SetPosZ(BATTLE_STAGE_Z);
	}

	//各オブジェクト更新
	player_->Update();
	enemy_->Update();
	stage_->Update();

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

	if(enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}
	//YouDied描画
	player_->DrawDead();

	//霧の壁描画
	stage_->DrawTranslucent();

	//UI描画
	enemy_->DrawBarUI();
	//UI描画
	player_->DrawBarUI();
}

void GameScene::SkipBarDraw(void)
{
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

	skipBarUI_->SetBarSize({ currentGaugeWidth, GAUGE_H });
	skipBarUI_->SetBarMaxWidth(GAUGE_W);
	skipBarUI_->Draw();
}

void GameScene::UpdateImGui(void)
{
	ImGui::Text("GameScene");
	ImGui::Text("slowFrameCnt : %d", slowMotionFrameCount_);
	ImGui::Text("slowMotionFrame : %.2f", slowMotionFrame_);

	if (ImGui::Button("Explore"))
	{
		InitStateExplore();
		ChangeState(STATE::EXPLORE);
	}
	if (ImGui::Button("Encount"))
	{
		enemy_->ChangeState(Enemy::STATE::NONE);
		player_->ChangeState(Player::STATE::NONE);
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();
	}
	//バトル開始位置
	const VECTOR battlePos = { 10.0, -217.0, 900.0 };
	if (ImGui::Button("Battle"))
	{
		player_->SetPos(battlePos);
		InitStateBattle();
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		ChangeState(STATE::BATTLE);
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
		//敵のImGui
		if (ImGui::BeginTabItem("Enemy"))
		{
			enemy_->UpdateImGui();
			ImGui::EndTabItem();
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
