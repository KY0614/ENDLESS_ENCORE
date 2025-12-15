#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/Stage.h"
#include "TitleScene.h"

namespace
{
	const int ADVERTISE_TIME = 1000000;
}

TitleScene::TitleScene(void)
{
	toAdvertiseLoopTimer_ = 0;
}

TitleScene::~TitleScene(void)
{

}

void TitleScene::LoadData(void)
{
}

void TitleScene::Init(void)
{
	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();

	//タイトルロゴ
	logoImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_LOGO).handleId_;

	toAdvertiseLoopTimer_ = ADVERTISE_TIME;

	mainCamera->SetFixedPointPos(VGet(0.0f, -109.0f, -65.0f),VGet(0.0f, -153.0f, 2863.0f));
	//定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Enter"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::SELECT);
	}

	if(--toAdvertiseLoopTimer_ <= 0)
	{
		toAdvertiseLoopTimer_ = ADVERTISE_TIME;
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::ADVERTISE);
		return;
	}

	stage_->Update();
}

void TitleScene::Draw(void)
{
	DrawString(0, 0, L"Title", 0xFFFFFF);
	DrawString(Application::SCREEN_SIZE_X/2, 
		Application::SCREEN_SIZE_Y / 2, L"Push Enter or B", 0xFFFFFF);

	stage_->Draw();

	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - 100,
		1.0f, 0.0f,
		logoImg_, true);

	////ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	//SetDrawMode(DX_DRAWMODE_BILINEAR);
}