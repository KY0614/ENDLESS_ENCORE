#include "../../Application.h"
#include "../../Manager/Generic/SceneManager.h"
#include "../../Manager/Generic/InputManager.h"
#include "../PauseScene.h"
#include "CharacterScene.h"

CharacterScene::CharacterScene(void)
{
}

CharacterScene::~CharacterScene(void)
{
}

void CharacterScene::Init(void)
{
}

void CharacterScene::Update(void)
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

void CharacterScene::Draw(void)
{
	const Application::Size& wSize = Application::GetInstance().GetWindowSize();
	const int margine = 50;

	//ê¬Ç¡Ç€Ç¢ÉZÉçÉtÉ@Éì
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 168);
	DrawBox(PauseScene::MARGINE_SIZE, PauseScene::MARGINE_SIZE + margine,
		wSize.width_ - PauseScene::MARGINE_SIZE, wSize.height_ - PauseScene::MARGINE_SIZE,
		0x00ff00, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	//ê¬òg
	DrawBoxAA(PauseScene::MARGINE_SIZE, PauseScene::MARGINE_SIZE + margine,
		wSize.width_ - PauseScene::MARGINE_SIZE, wSize.height_ - PauseScene::MARGINE_SIZE,
		0x00ff00, false, 3.0f);
	DrawString(PauseScene::MARGINE_SIZE + 10, PauseScene::MARGINE_SIZE + 40, L"CharacterScene", 0x000000, true);
}
