#include <DxLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Common/Cube.h"
#include "PauseScene.h"
#include "GameScene.h"

GameScene::GameScene(void) : 
	update_(&GameScene::UpdateGame),
	draw_(&GameScene::DrawGame)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{

	//プレイヤー
	player_ = std::make_unique<Player>();
	player_->Init();

	//プレイヤー
	enemy_ = std::make_unique<Enemy>(*player_);
	enemy_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

#ifdef _DEBUG
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
#endif // _DEBUG

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

	if (CheckHitKey(KEY_INPUT_R))
	{
		this->Init();
	}

	if (ins.IsInputTriggered("pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(std::make_unique<PauseScene>());
	}

	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
	}

#ifdef _DEBUG
	floor_.Update();
#endif // _DEBUG
}

void GameScene::DrawGame(void)
{
#ifdef _DEBUG

	DrawString(0, 0, L"Game", 0xFFFFFF);

	DrawDebug();
#endif // _DEBUG

	enemy_->Draw();
	player_->Draw();

	if(enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}
}

void GameScene::DrawDebug(void)
{
	MV1DrawModel(floor_.modelId);
}
