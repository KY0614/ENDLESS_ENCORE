#include <memory>
#include "../Application.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Scene/TitleScene.h"
#include "../Scene/PauseScene/InventoryScene.h"
#include "../Scene/PauseScene/OptionScene.h"
#include "../Scene/PauseScene/CharacterScene.h"
#include "../Scene/PauseScene/ExplainScene.h"
#include "../Scene/PauseScene/KeyConfigScene.h"
#include "PauseScene.h"

namespace 
{
	const int BACK_IMG_SCALE = 1080;
	const int APPEAR_INTERVAL = 15;
	//メニューリスト関連
	const int MENU_LIST_HEIGHT = 70;	//メニューリストの高さ(１行)
	const int MENU_LIST_WIDTH = 200;	//メニューリストの幅(１行)
	const int MENU_START_X = 300;		//メニューリストの開始X座標
	const int MENU_START_Y = 100;		//メニューリストの開始Y座標
}

PauseScene::PauseScene(void) :
	update_(&PauseScene::UpdateAppear),
	draw_(&PauseScene::DrawProcess)
{
	menuList_ = {
		L"インベントリ",
		L"キャラクター",
		L"オプション",
		L"ゲーム終了"
	};

	menuFuncTable_ = {
	{L"インベントリ",[this]()
		{
			//std::unique_ptr<InventoryScene> scene = std::make_unique<InventoryScene>();
			//SceneManager::GetInstance().PushScene(std::move(scene));
			SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::INVENTORY);
		}
	},
	{ L"キャラクター",[this]()
		{
			//std::unique_ptr<CharacterScene> scene = std::make_unique<CharacterScene>();
			//SceneManager::GetInstance().PushScene(std::move(scene));
			SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::CHARACTER);
		}
	},
	{ L"オプション",[this]()
		{
			//std::unique_ptr<OptionScene> scene = std::make_unique<OptionScene>();
			//SceneManager::GetInstance().PushScene(std::move(scene));

			SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::OPTION);

			//SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
			//return;
		}
	},
	{ L"ゲーム終了",[this]()
		{
			Application::GetInstance().EndGame();
			return;
		}
	}
	};

	menuDrawTable_ = {
	{ L"インベントリ",[this]()
		{
			std::unique_ptr<InventoryScene> scene = std::make_unique<InventoryScene>();
			scene->Draw();
		}
	},
	{ L"キャラクター",[this]()
		{
			
		}
	},
	{ L"オプション",[this]()
		{
			std::unique_ptr<OptionScene> scene = std::make_unique<OptionScene>();
			scene->Draw();
		}
	},
	{ L"ゲーム終了",[this]()
		{
			
		}
	}
	};

	backImg_ = -1;
	menuListImg_ = nullptr;
	menuCursorImg_ = -1;
	//リソースの初期化
	auto& resM = ResourceManager::GetInstance();
	resM.InitPause();

	frame_ = 0;
	cursorIdx_ = 0;
}

PauseScene::~PauseScene(void)
{
}

void PauseScene::LoadData(void)
{
}

void PauseScene::Init(void)
{
}

void PauseScene::Update(void)
{
	(this->*update_)();
}

void PauseScene::Draw(void)
{
	(this->*draw_)();
}

void PauseScene::UpdateAppear(void)
{
	if (++frame_ >= APPEAR_INTERVAL) 
	{
		update_ = &PauseScene::UpdateNormal;
		draw_ = &PauseScene::DrawNormal;
	}
}

void PauseScene::UpdateDisappear(void)
{
	if (--frame_ <= 0) 
	{
		SceneManager::GetInstance().PopScene();
		SceneManager::GetInstance().SceneID2Game();
		return;
	}
}

void PauseScene::DrawProcess(void)
{
	//中心から徐々に広がる
	const Application::Size& wSize = Application::GetInstance().GetWindowSize();
	int centerY = wSize.height_ / 2;		//画面中心Y
	int frameHalfHeight = (wSize.height_ - MARGINE_SIZE * 2) / 2;	//枠の高さの半分(上下に広がるので半分でOK)

	//フェードしつつ0～1までの範囲の値の割合を計算
	//出現・消滅時の高さ変化率(0.0～1.0)
	float rate = static_cast<float>(frame_) /
		static_cast<float>(APPEAR_INTERVAL);

	frameHalfHeight *= rate;

	//白っぽいセロファン
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 168);
	DrawBox(MARGINE_SIZE, centerY - frameHalfHeight, wSize.width_ - MARGINE_SIZE, centerY + frameHalfHeight, 0xffffff, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawBoxAA(MARGINE_SIZE, centerY - frameHalfHeight, wSize.width_ - MARGINE_SIZE, centerY + frameHalfHeight, 0xffffff, false);

	//DrawExtendGraph(MARGINE_SIZE,
	//	centerY - frameHalfHeight,
	//	Application::SCREEN_SIZE_X - MARGINE_SIZE,
	//	centerY + frameHalfHeight,
	//	backImg_, true);
}

void PauseScene::UpdateNormal(void)
{	
	auto& sound = SoundManager::GetInstance();
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Pause"))
	{
		sound.Play(SoundManager::SOUND::MENU_CLOSE);
		update_ = &PauseScene::UpdateDisappear;
		draw_ = &PauseScene::DrawProcess;
		return;
	}
	if (ins.IsInputTriggered("Right"))
	{
		sound.Play(SoundManager::SOUND::NEXT_PAGE);
		cursorIdx_ = (cursorIdx_ + 1) % menuList_.size();
	}
	else if (ins.IsInputTriggered("Left"))
	{
		sound.Play(SoundManager::SOUND::NEXT_PAGE);
		cursorIdx_ = (cursorIdx_ + menuList_.size() - 1) % menuList_.size();
	}

	if (ins.IsInputTriggered("Decide"))
	{
		sound.Play(SoundManager::SOUND::RETURN_PAGE);
		auto selectedName = menuList_[cursorIdx_];
		menuFuncTable_[selectedName]();
		return;
	}
}

void PauseScene::DrawNormal(void)
{
	const Application::Size& wSize = Application::GetInstance().GetWindowSize();
	//白っぽいセロファン
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 168);
	DrawBox(MARGINE_SIZE, MARGINE_SIZE,
		wSize.width_ - MARGINE_SIZE, wSize.height_ - MARGINE_SIZE,
		0xffffff, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//白枠
	DrawBoxAA(MARGINE_SIZE, MARGINE_SIZE,
		wSize.width_ - MARGINE_SIZE, wSize.height_ - MARGINE_SIZE,
		0xffffff, false, 3.0f);
	DrawString(MARGINE_SIZE + 10, MARGINE_SIZE - 20, L"PauseScene", 0xFFFFFF, true);
	//メニューリストの描画
	DrawMenuList();
}

void PauseScene::DrawMenuList(void)
{
	//画面の大きさに合わせて拡大率を変える
	float aspectRatio = static_cast<float>(Application::SCREEN_SIZE_Y) /
		static_cast<float>(Application::SCREEN_MAX_SIZE_Y);
	
	int lineX = MENU_START_X * aspectRatio;

	const int lineY = MENU_START_Y * aspectRatio;

	//現在選択している行をずらす幅
	const int currentLineOffset = 20;
	//現在選択している行の文字列
	std::wstring currentStr = menuList_[cursorIdx_];
	for (auto& row : menuList_)
	{
		//文字列の幅を取得
		int stringWidth = GetDrawStringWidth(row.c_str(), row.size());
		//int lineX = line_start_X;
		unsigned int col = 0xFFFFFF;
		if(row == currentStr)
		{
			DrawString(lineX , lineY,L"⇒",0xFF0000);
			col = 0xFF00FF;
			lineX += currentLineOffset;
		}

		DrawFormatString(lineX + 1, lineY + 1, 0x000000, L"%s", row.c_str());
		DrawFormatString(lineX, lineY, col, L"%s", row.c_str());
		lineX += (MENU_LIST_WIDTH + stringWidth) * aspectRatio;
	}
}
