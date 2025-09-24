#include <DxLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
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

	//
	cube_ = std::make_unique<Cube>();

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
	DrawDebug();
#endif // _DEBUG

	player_->Draw();

	/*cube_->MakeBox(CommonUtility::VECTOR_ZERO,
		200.0f, 30.0f, 200.0f, GetColorU8(255, 255, 255, 255));*/

	DrawString(0, 0,L"Game", 0xFFFFFF);

}

void GameScene::DrawDebug(void)
{
	MV1DrawModel(floor_.modelId);
}
