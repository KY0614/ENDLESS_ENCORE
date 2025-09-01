#include "../Application.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "KeyConfigScene.h"

namespace {
	const int MARGINE_SIZE = 30;
	const int EXPLAIN_SIZE = 60;
}

KeyConfigScene::KeyConfigScene(void)
{
	backImg_ = -1;
	keyConfImg_ = -1;
	//リソースの初期化
	auto& resM = ResourceManager::GetInstance();
	resM.InitPause();
}

KeyConfigScene::~KeyConfigScene(void)
{
}

void KeyConfigScene::Init(void)
{
	auto& sound = SoundManager::GetInstance();
	//カーソルSE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::MENU_BACK,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::MENU_BACK).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::MENU_BACK, 256 / 2);

	backImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::TUTORIAL_BACK).handleId_;

	keyConfImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::MENU_KEY_CONFIG).handleId_;

	menuBackImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::MENU_BACK_LOGO).handleId_;
}

void KeyConfigScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	auto& sound = SoundManager::GetInstance();
	if (ins.IsInputTriggered("Interact"))
	{
		sound.Play(SoundManager::SOUND::MENU_BACK);
		SceneManager::GetInstance().PopScene();
		return;
	}
}

void KeyConfigScene::Draw(void)
{
	//画面の大きさに合わせて拡大率を変える
	float scale = static_cast<float>(Application::SCREEN_SIZE_Y) /
		static_cast<float>(Application::SCREEN_MAX_SIZE_Y);
	float size = 0.8f;

	DrawExtendGraph(MARGINE_SIZE,
		MARGINE_SIZE,
		Application::SCREEN_SIZE_X - MARGINE_SIZE,
		Application::SCREEN_SIZE_Y - MARGINE_SIZE,
		backImg_, true);


	//説明画像
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - (EXPLAIN_SIZE * scale),
		scale * size, 0.0f,
		keyConfImg_,
		true);

	//メニューへ戻る画像
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y - MARGINE_SIZE - (90  * scale),
		scale * size, 0.0f,
		menuBackImg_,
		true);
}
