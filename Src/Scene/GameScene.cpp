#include <DxLib.h>
#include "../Application.h"
#include "../Utility/DrawUtiity.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/ItemPrevew.h"
#include "PauseScene.h"
#include "GameScene.h"

GameScene::GameScene(void) : 
	update_(&GameScene::UpdateGame),
	draw_(&GameScene::DrawGame)
{
	shakeFrame_ = 0;
	shakeRate_ = 0.0f;
	RT_ = -1;
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	//プレイヤー
	player_ = std::make_unique<Player>();
	player_->Init();

	//敵
	enemy_ = std::make_unique<Enemy>(*player_);
	enemy_->Init();

	//プレイヤー
	item_ = std::make_unique<ItemPrevew>();
	item_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);


	floor_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::FLOOR));
	float scale = 1.0f;
	floor_.scl = { scale,scale,scale };
	floor_.pos = { 0.0f, 0.0f, 0.0f };
	floor_.quaRot = Quaternion();
	floor_.quaRotLocal =
		Quaternion::Euler({ 0.0f,0.0f, 0.0f });
	floor_.MakeCollider(Collider::TYPE::STAGE);
	floor_.Update();
	player_->AddCollider(floor_.collider);

}

void GameScene::Update(void)
{
	(this->*update_)();
}

void GameScene::Draw(void)
{
	(this->*draw_)();
}

void GameScene::UpdateGame(void)
{
	InputManager& ins = InputManager::GetInstance();

	player_->Update();
	enemy_->Update();

#ifdef _DEBUG
	if (ins.IsInputTriggered("Reset"))
	{
		this->Init();
	}

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

	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
	}

	floor_.Update();
}

void GameScene::DrawGame(void)
{
	MV1DrawModel(floor_.modelId);

	//敵描画
	enemy_->Draw();	
	//プレイヤー描画
	player_->Draw();

	if(enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}
	item_->Draw();
}

void GameScene::DrawDebug(void)
{
	
}
