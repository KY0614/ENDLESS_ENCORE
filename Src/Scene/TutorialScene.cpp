#include <DxLib.h>
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "TutorialScene.h"

TutorialScene::TutorialScene(void)
{
}

TutorialScene::~TutorialScene(void)
{

}

void TutorialScene::Init(void)
{
}

void TutorialScene::Update(void)
{
	//(this->*update_)();
	auto& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TutorialScene::Draw(void)
{
	//(this->*draw_)();
	DrawString(0, 0, L"Tutorial", 0xFFFFFF);
}