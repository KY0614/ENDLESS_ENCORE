#include "../../Application.h"
#include "../../Manager/Generic/SceneManager.h"
#include "../../Manager/Generic/InputManager.h"
#include "../PauseScene.h"
#include "OptionScene.h"

namespace
{
	const int APPEAR_INTERVAL = 15;
	//メニューリスト関連
	const int MENU_LIST_HEIGHT = 70;	//メニューリストの高さ(１行)
	const int MENU_LIST_WIDTH = 200;	//メニューリストの幅(１行)
	const int MENU_START_X = 200;		//メニューリストの開始X座標
	const int MENU_START_Y = 100;		//メニューリストの開始Y座標
}

OptionScene::OptionScene(void)
{
}

OptionScene::~OptionScene(void)
{
}

void OptionScene::LoadData(void)
{
}

void OptionScene::Init(void)
{
}

void OptionScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Pause"))
	{
		SceneManager::GetInstance().PopScene();
		return;
	}

	if (ins.IsInputTriggered("Decide"))
	{
		SceneManager::GetInstance().PopScene();
		return;
	}
}

void OptionScene::Draw(void)
{
	const Application::Size& wSize = Application::GetInstance().GetWindowSize();
	const int margine = 50;

	//灰色っぽいセロファン
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 168);
	DrawBox(PauseScene::MARGINE_SIZE, PauseScene::MARGINE_SIZE + margine,
		wSize.width_ - PauseScene::MARGINE_SIZE, wSize.height_ - PauseScene::MARGINE_SIZE,
		0x888888, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	//灰色枠
	DrawBoxAA(PauseScene::MARGINE_SIZE, PauseScene::MARGINE_SIZE + margine,
		wSize.width_ - PauseScene::MARGINE_SIZE, wSize.height_ - PauseScene::MARGINE_SIZE,
		0x888888, false, 3.0f);
	DrawString(PauseScene::MARGINE_SIZE + 10, PauseScene::MARGINE_SIZE + 40, L"OptionScene", 0x000000, true);
}
