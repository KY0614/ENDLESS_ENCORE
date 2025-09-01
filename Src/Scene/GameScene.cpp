#include <DxLib.h>
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Object/Player.h"
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
	//カメラ
	//mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::TOP_FIXED);
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

	if (ins.IsInputTriggered("pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(std::make_unique<PauseScene>());
	}

	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
	}
}

void GameScene::DrawGame(void)
{
	DrawString(0, 0,L"Game", 0xFFFFFF);
}
