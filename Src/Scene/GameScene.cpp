#include <DxLib.h>
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

GameScene::GameScene(void)
{
	loadingTime_ = 0.0f;
	player_ = nullptr;
	enemy_ = nullptr;
	stage_ = nullptr;
	isFaseChange_ = false;
	//状態管理
	stateChanges_.emplace(STATE::LOADING, std::bind(&GameScene::ChangeStateLoading, this));
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
	SetUseASyncLoadFlag(true);

	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);


	//sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::BATTLE,
	//	ResourceManager::GetInstance().Load(ResourceManager::SRC::GAME_BGM).handleId_);
	//sound.AdjustVolume(SoundManager::SOUND::BATTLE, 25);

}

void GameScene::Init(void)
{

	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 25);

	// ポイントライト
	std::unique_ptr<PointLight>light;
	light = std::make_unique<PointLight>();
	light->Init();
	pointLight_.push_back(std::move(light));

	// スポットライト
	std::unique_ptr<SpotLight>slight;
	slight = std::make_unique<SpotLight>();
	slight->Init();
	spotLight_.push_back(std::move(slight));

	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init(GetPointLightPos(),GetSpotLightPos());

	//プレイヤー
	player_ = std::make_shared<Player>();
	player_->Init();

	//敵
	enemy_ = std::make_shared<Enemy>(*player_);
	enemy_->Init();

	//敵
	encountScene_ = std::make_unique<EncountScene>(*player_,*enemy_);
	encountScene_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	player_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetTransform().collider);

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
	ChangeState(STATE::EXPLORE);
}

void GameScene::Update(void)
{
	for (std::unique_ptr<PointLight>& light : pointLight_)
	{
		light->Update();
	}
	for (std::unique_ptr<SpotLight>& light : spotLight_)
	{
		light->Update();
	}
	//更新ステップ
	stateUpdate_();
	UpdateDebugImGui();
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
	//
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
			player_->SetRotateY(enemy_->GetTransform().quaRot);
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

void GameScene::ChangeStateLoading(void)
{
	stateUpdate_ = std::bind(&GameScene::LoadingUpdate, this);
	stateDraw_ = std::bind(&GameScene::LoadingDraw, this);
}

void GameScene::ChangeStateExplore(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 50);
	sound.Play(SoundManager::SOUND::EXPLORE);

	stateUpdate_ = std::bind(&GameScene::UpdateExplore, this);
	stateDraw_ = std::bind(&GameScene::DrawExplore, this);
}

void GameScene::ChangeStateEncount(void)
{
	stateUpdate_ = std::bind(&GameScene::UpdateEncount, this);
	stateDraw_ = std::bind(&GameScene::DrawEncount, this);
}

void GameScene::ChangeStateBattle(void)
{
	stage_->IsBattle();
	enemy_->ChangeState(Enemy::STATE::MOVE);
	player_->ChangeState(Player::STATE::PLAY);
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

void GameScene::UpdateExplore(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::PAUSE);
		return;
	}
	//更新
	player_->Update();
	stage_->Update();
	encountScene_->Update();

	//Z値850を超えるとエンカウント状態へ遷移
	//(ステージの手前端よりも奥)
	const float stagePosZ = 850.0f;
	if (player_->GetTransform().pos.z > stagePosZ)
	{
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();
	}
#ifdef _DEBUG
	if (ins.IsInputTriggered("Next"))
	{
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();
	}
#endif // _DEBUG

}

void GameScene::DrawExplore(void)
{
	//プレイヤー描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();

	DrawString(0, 0, L"探索ステージ", 0xffffff);
}

void GameScene::UpdateEncount(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (encountScene_->IsFinished() &&
		SceneManager::GetInstance().GetFader().lock()->IsEnd() &&
		SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_OUT)
	{
		enemy_->ChangeState(Enemy::STATE::WAIT);
		player_->ChangeState(Player::STATE::WAIT);
		//カメラ
		mainCamera->SetFollow(&player_->GetTransform());
		mainCamera->SetTarget(&enemy_->GetTransform());
		mainCamera->ChangeMode(Camera::MODE::FOLLOW);
		SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_IN);
	}

	if (encountScene_->IsFinished() &&
		SceneManager::GetInstance().GetFader().lock()->IsEnd() &&
		SceneManager::GetInstance().GetFader().lock()->GetState() == Fader::STATE::FADE_IN)
	{
		ChangeState(STATE::BATTLE);
	}

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

	if (enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	player_->DrawDead();

	encountScene_->Draw();

	DrawString(0, 0, L"エンカウント", 0xffffff);
}

void GameScene::UpdateBattle(void)
{
	InputManager& ins = InputManager::GetInstance();


	//敵の体力が半分以下でバックスタブ状態でなければ敵召喚状態へ遷移
	if (/*!enemy_->GetIsBackstab() &&*/
		enemy_->GetHP() <= enemy_->GetMaxHP() / 2.0f)
	{
		SceneManager::GetInstance().GetFader().lock()->SetFade(Fader::STATE::FADE_OUT);
		ChangeState(STATE::ENEMY_SUMMON);
		return;
	}

	player_->Update();
	enemy_->Update();
	stage_->Update();
	
#ifdef _DEBUG

	if (ins.IsInputTriggered("CameraShake"))
	{
		SceneManager::GetInstance().SetShakeScreen(true);
	}

#endif // _DEBUG

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

	DrawString(0, 0, L"バトル", 0xffffff);
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

	DrawString(0, 0, L"召喚", 0xffffff);
}

void GameScene::UpdateBattleSecond(void)
{
}

void GameScene::DrawBattleSecond(void)
{
}

void GameScene::DrawMessage(void)
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
	std::wstring str = L"静かに浮かぶ何かがある...。";
	int diff = GetDrawStringWidth(str.c_str(), str.size(), NULL);
	DrawString(Application::SCREEN_SIZE_X / 2 - diff / 2,
		Application::SCREEN_SIZE_Y / 2 - 32,
		str.c_str(), 0xFFFFFF);

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
		enemy_->ChangeState(Enemy::STATE::NONE);
		player_->ChangeState(Player::STATE::NONE);
		ChangeState(STATE::ENCOUNT);
		encountScene_->Start();
	}
	if (ImGui::Button("Battle"))
	{
		player_->SetPos({ 10.0, -217.0, 900.0 });
		ChangeState(STATE::BATTLE);
	}

	//終了処理
	ImGui::End();
}