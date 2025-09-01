#include "../Application.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include"../Utility/StringUtility.h"
#include "SelectScene.h"

SelectScene::SelectScene(void)
{
	currentIdx_ = 0;

	selectList_ = {
	L"EASY",
	L"NORMAL",
	L"HARD"
	};
}

SelectScene::~SelectScene(void)
{
}

void SelectScene::Init(void)
{
}

void SelectScene::Update(void)
{
	//シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TUTORIAL);
	}

	if (ins.IsInputTriggered("Down"))
	{
		currentIdx_ = (currentIdx_ + 1) % selectList_.size();
	}
	else if (ins.IsInputTriggered("Up"))
	{
		currentIdx_ = (currentIdx_ + selectList_.size() - 1) % selectList_.size();
	}

}

void SelectScene::Draw(void)
{
	DrawString(0, 0, L"難易度選択", 0xFFFFFF);

#ifdef _DEBUG
	SetFontSize(32);
	DebugDraw();
	SetFontSize(16);
#endif // _DEBUG
	
}

void SelectScene::DebugDraw(void)
{
	constexpr int line_start_X = Application::SCREEN_SIZE_X / 2 - 50;
	constexpr int line_start_Y = Application::SCREEN_SIZE_Y / 2 - 100;

	int lineY = line_start_Y;

	auto currentStr = selectList_[currentIdx_];

	for (auto& row : selectList_) {
		int lineX = line_start_X;
		unsigned int col = 0x4444ff;
		if (row == currentStr) {
			DrawString(lineX - 20, lineY, L"⇒", 0xff0000);
			col = 0xff00ff;
			lineX += 25;
		}

		DrawFormatString(lineX + 1, lineY + 1, 0x000000, L"%s", row.c_str());
		DrawFormatString(lineX, lineY, col, L"%s", row.c_str());
		lineY += 50;
	}
}