#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "ResultScene.h"

ResultScene::ResultScene(void)
{
}

ResultScene::~ResultScene(void)
{
}

void ResultScene::LoadData(void)
{
}

void ResultScene::Init(void)
{
}

void ResultScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void ResultScene::Draw(void)
{
	DrawString(0, 0, L"Result", 0xFFFFFF);
}
