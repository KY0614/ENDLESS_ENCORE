#include <DxLib.h>
#include<EffekseerForDXLib.h>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Common/Fader.h"
#include "../Utility/DrawUtiity.h"
#include "../Utility/CommonUtility.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Transition/FadeTransitor.h"
#include "../Object/Common/Geometry/Sphere.h"
#include "../Object/Common/Geometry/Cube.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Stage.h"
#include "../Object/PointLight.h"
#include "../Object/SpotLight.h"
#include "PauseScene.h"
#include "EncountScene.h"
#include "GameScene.h"

namespace
{
	const float SKIP_TIME = 2.0f; //スキップ判定時間
}

GameScene::GameScene(void)
{
	loadingTime_ = 0.0f;
	player_ = nullptr;
	enemy_ = nullptr;
	stage_ = nullptr;
	isFaseChange_ = false;
	postEffectScreen_ = -1;
	state_ = STATE::NONE;

	skipTimer_ = 0.0f;
	isSkip_ = false;

	//状態管理
	stateChanges_.emplace(STATE::LOADING, std::bind(&GameScene::ChangeStateLoading, this));
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&GameScene::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::EXPLORE, std::bind(&GameScene::ChangeStateExplore, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&GameScene::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&GameScene::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::ENEMY_SUMMON, std::bind(&GameScene::ChangeStateEnemySummon, this));
	stateChanges_.emplace(STATE::BATTLE_SECOND, std::bind(&GameScene::ChangeStateBattleSecond, this));

	//ChangeState(STATE::LOADING);
}

GameScene::~GameScene(void)
{
	DeleteGraph(postEffectScreen_);
}

void GameScene::LoadData(void)
{
	//非同期読み込みを有効にする
	//SetUseASyncLoadFlag(true);

	//SoundManager& sound = SoundManager::GetInstance();
	//sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
	//	ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);


	//sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::BATTLE,
	//	ResourceManager::GetInstance().Load(ResourceManager::SRC::GAME_BGM).handleId_);
	//sound.AdjustVolume(SoundManager::SOUND::BATTLE, 25)
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
	sound.AdjustVolume(SoundManager::SOUND::BATTLE, 50);

	// ポイントライト
	//std::unique_ptr<PointLight>light;
	//light = std::make_unique<PointLight>();
	//light->Init();
	//pointLight_.push_back(std::move(light));

	// スポットライト
	//std::unique_ptr<SpotLight>slight;
	//slight = std::make_unique<SpotLight>();
	//slight->Init();
	//spotLight_.push_back(std::move(slight));

	//ステージ
	stage_ = std::make_shared<Stage>();
	//stage_->Init(GetPointLightPos(),GetSpotLightPos());
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

	// ポストエフェクト用スクリーン
	postEffectScreen_ = MakeScreen(
		Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, true);

	// ポストエフェクト用(ブラー)
	blurMaterial_ = std::make_unique<PixelMaterial>("Blur.cso", 1);
	blurMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	blurMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	blurRenderer_ = std::make_unique<PixelRenderer>(*blurMaterial_);
	blurRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

	//初期状態設定
	ChangeState(STATE::WAKE_UP);
}

void GameScene::Update(void)
{
	//for (std::unique_ptr<PointLight>& light : pointLight_)
	//{
	//	light->Update();
	//}
	//for (std::unique_ptr<SpotLight>& light : spotLight_)
	//{
	//	light->Update();
	//};

	//更新ステップ
	stateUpdate_();

#ifdef _DEBUG

	UpdateDebugImGui();

#endif // _DEBUG
}

void GameScene::Draw(void)
{
	//更新ステップ
	stateDraw_();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();
	//for (auto& light : pointLight_)
	//{
	//	light->Draw();
	//}
	//for (auto& light : spotLight_)
	//{
	//	light->Draw();
	//}
	// ポストエフェクト(ブラー)
	//-----------------------------------------
	
	//SetDrawScreen(postEffectScreen_);

	//// 画面を初期化
	//ClearDrawScreen();

	//blurRenderer_->Draw();

	//// メインに戻す
	//SetDrawScreen(mainScreen);
	//DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------
}

VECTOR GameScene::GetPointLightPos()
{
	return pointLight_[0]->GetTransform().pos;
}

VECTOR GameScene::GetSpotLightPos()
{
	return spotLight_[0]->GetTransform().pos;
}

void GameScene::Backstab(void)
{
	VECTOR backDir = enemy_->GetTransform().GetBack();
	//バックスタブ位置
	const float distance = 60.0f;
	VECTOR target = VAdd(enemy_->GetTransform().pos, VScale(backDir, distance));
	//ダウン中のバックスタブ判定
	if (enemy_->GetIsDown() && enemy_->CheckBackstab())
	{
		if (player_->GetIsParry())
		{
			player_->SetPos(target);
			//player_->SetRotateY(enemy_->GetTransform().quaRot);
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

	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 60);
	sound.Play(SoundManager::SOUND::EXPLORE);
}

void GameScene::InitStateBattle(void)
{
	stage_->IsBattle();
	enemy_->ChangeState(Enemy::STATE::MOVE);
	enemy_->SetIsEncount(true);
	enemy_->Update(); //状態変更後すぐに更新しておく
	player_->ChangeState(Player::STATE::PLAY);
	player_->Update(); //状態変更後すぐに更新しておく
	player_->AddCollider(stage_->GetMistWallTransform().collider);
	//mainCamera->SetFollow(&player_->GetTransform());
	//mainCamera->ChangeMode(Camera::MODE::FOLLOW);
	SoundManager& sound = SoundManager::GetInstance();
	//sound.AdjustVolume(SoundManager::SOUND::BATTLE, 50);
	sound.Play(SoundManager::SOUND::BATTLE);
}

void GameScene::ChangeState(STATE state)
{
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void GameScene::ChangeStateLoading(void)
{
	stateUpdate_ = std::bind(&GameScene::LoadingUpdate, this);
	stateDraw_ = std::bind(&GameScene::LoadingDraw, this);
}

void GameScene::ChangeStateWakeUp(void)
{
	VECTOR pos = player_->GetTransform().pos;
	pos = VAdd(pos, VScale(
		VAdd(player_->GetTransform().GetRight(),
			player_->GetTransform().GetForward()), 100.0f));
	VECTOR targetPos = player_->GetTransform().pos;
	targetPos.y += 70.0f;
	const float craneUpSpeed = 0.13f;
	mainCamera->SetCraneUpPos(pos, 80.0f,targetPos, craneUpSpeed);
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

void GameScene::ChangeStateEnemySummon(void)
{
	stateUpdate_ = std::bind(&GameScene::UpdateEnemySummon, this);
	stateDraw_ = std::bind(&GameScene::DrawEnemySummon, this);
}

void GameScene::ChangeStateBattleSecond(void)
{
	stateUpdate_ = std::bind(&GameScene::UpdateBattleSecond, this);
	stateDraw_ = std::bind(&GameScene::DrawBattleSecond, this);
}

void GameScene::LoadingUpdate(void)
{
	bool loadTimeOver = CommonUtility::TimeOver(loadingTime_, 2.0f);

	//ロードが完了したか判断
	if (GetASyncLoadNum() == 0 && loadTimeOver)
	{
		//非同期処理を無効にする
		SetUseASyncLoadFlag(false);

		//カーソルモードの変更
		//input.ChangeCurrsolMode(false);

		//初期化処理
		Init();
		//フェードイン開始
		//sceneManager_.StartFadeIn();
		SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);

		ChangeState(STATE::EXPLORE);

		//更新関数のセット
		//updataFunc_ = [&](InputManager& input) {NormalUpdate(input); };
		//描画関数のセット
		//drawFunc_ = std::bind(&GameScene::NormalDraw, this);
	}
}

void GameScene::LoadingDraw(void)
{
	//ロード中
	float time = 5.0f;
	int count = static_cast<int>(time / 0.5f);
	count %= 5;

	std::wstring loadStr = L"now loading";
	std::wstring dotStr = L".";

	for (int i = 0; i < count; i++)
	{
		loadStr += dotStr;
	}
	//DrawStringToHandle(250, 250, loadStr.c_str(), 0xffffff);
	DrawFormatString(250, 250, 0xffffff, loadStr.c_str());

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

	//InputManager& ins = InputManager::GetInstance();
	//if (ins.IsInputTriggered("Pause"))
	//{
	//	//ポーズボタンが押されたらポーズシーンへ遷移
	//	SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::PAUSE);
	//	return;
	//}

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

	if(player_->GetTransform().pos.z < 938.0f)
	{
		player_->SetPosZ(938.0f);
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

void GameScene::UpdateEnemySummon(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//フェードアウトが完了したらフェードインへ切り替え
	if (fader.lock()->GetState() == Fader::STATE::FADE_OUT &&
		fader.lock()->IsEnd())
	{
		fader.lock()->SetFade(Fader::STATE::FADE_IN);
	}
	//フェードインが完了するまで更新しない
	if (!(fader.lock()->GetState() == Fader::STATE::FADE_IN &&
		fader.lock()->IsEnd()))
	{
		return;
	}

	player_->Update();
	enemy_->Update();
	stage_->Update();

	Backstab();
}

void GameScene::DrawEnemySummon(void)
{
	//プレイヤー描画
	stage_->Draw();
	//敵描画
	enemy_->Draw();
	//プレイヤー描画
	player_->Draw();


	if (enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	player_->DrawDead();
#ifdef _DEBUG
	DrawString(0, 0, L"召喚", 0xffffff);
#endif

}

void GameScene::UpdateBattleSecond(void)
{
}

void GameScene::DrawBattleSecond(void)
{
	enemy_->DrawHPBar();
	player_->DrawHPBar();
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

void GameScene::DrawMessage(const std::wstring& wStr)
{
	const int boxWidth = Application::SCREEN_SIZE_X - 100;
	const int boxHeight = 200;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(100,
		Application::SCREEN_SIZE_Y / 2 - boxHeight / 2,
		boxWidth,
		Application::SCREEN_SIZE_Y / 2 + boxHeight / 2,
		0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	int diff = GetDrawStringWidth(wStr.c_str(), wStr.size(), NULL);
	DrawString(Application::SCREEN_SIZE_X / 2 - diff / 2,
		Application::SCREEN_SIZE_Y / 2 - 32,
		wStr.c_str(), 0xFFFFFF);

	const int lineY = Application::SCREEN_SIZE_Y / 2 + 16;
	int lineX = (Application::SCREEN_SIZE_X / 2 - 150);

}

void GameScene::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("GameScene");

	//状態変更ボタン
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
	if (ImGui::Button("Battle"))
	{
		player_->SetPos({ 10.0, -217.0, 900.0 });
		InitStateBattle();
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		ChangeState(STATE::BATTLE);
	}

	//終了処理
	ImGui::End();
}