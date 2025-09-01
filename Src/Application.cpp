#include <DxLib.h>
#include "Libs/ImGuiWrapper.h"
#include <EffekseerForDXLib.h>
#include "Manager/Generic/InputManager.h"
#include "Manager/Generic/ResourceManager.h"
#include "Manager/Generic/SceneManager.h"
#include "FpsControl.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SHADER = "Data/Shader/";
const std::string Application::PATH_SOUND = "Data/Sound/";
const std::string Application::PATH_SCORE = "Data/Score/";

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	isEnd_ = false;

	//アプリケーションの初期設定
	SetWindowText(L"BARISTA BLITZ");

	//ウィンドウサイズ
	windowSize_ = { SCREEN_SIZE_X ,SCREEN_SIZE_Y };
	SetGraphMode(windowSize_.width_, windowSize_.height_, 32);
	ChangeWindowMode(true);

	//DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	ImGuiWrapper::CreateInstance();

	//Effekseerの初期化
	InitEffekseer();

	//キー制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	//リソース管理初期化
	ResourceManager::CreateInstance();

	//シーン管理初期化
	SceneManager::CreateInstance();

	fps_ = std::make_unique<FpsControl>();
	fps_->Init();
}

void Application::Run(void)
{

	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();
	auto& imGuiWrapper = ImGuiWrapper::GetInstance();

	//ゲームループ
	while (ProcessMessage() == 0 /*&& CheckHitKey(KEY_INPUT_ESCAPE) == 0 */&& !isEnd_)
	{

		//フレームレート更新
		//1/60秒経過していないなら再ループ
		if (!fps_->UpdateFrameRate())continue;

		inputManager.Update();
		imGuiWrapper.Update();
		sceneManager.Update();

		sceneManager.Draw();

		RenderVertex();
		imGuiWrapper.Draw();

		fps_->CalcFrameRate();	//フレームレート計算

		ScreenFlip();

	}

}

void Application::Destroy(void)
{

	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SceneManager::GetInstance().Destroy();
	ImGuiWrapper::GetInstance().Destroy();

	//Effekseerを終了する。
	Effkseer_End();

	//DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	delete instance_;

}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
{
	isInitFail_ = false;
	isReleaseFail_ = false;
	fps_ = nullptr;
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}
