#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "MovieScene.h"

MovieScene::MovieScene(void)
{
}

MovieScene::~MovieScene(void)
{
}

void MovieScene::Init(void)
{
}

void MovieScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("NextScene"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::SELECT);
	}

}

void MovieScene::Draw(void)
{
	DrawString(0, 0, L"オープニング", 0xFFFFFF);
}
