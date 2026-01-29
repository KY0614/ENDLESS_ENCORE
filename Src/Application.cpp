#include <DxLib.h>
#include "Libs/ImGuiWrapper.h"
#include <EffekseerForDXLib.h>
#include "Manager/Generic/InputManager.h"
#include "Manager/Generic/ResourceManager.h"
#include "Manager/Generic/SceneManager.h"
#include "Common/FpsController.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SHADER = "Data/Shader/";
const std::string Application::PATH_SOUND = "Data/Sound/";
const std::string Application::PATH_SCORE = "Data/Score/";
const std::string Application::PATH_JSON = "Data/Json/";

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
	SetWindowText(L"ENDLESS ENCORE");

	//ウィンドウサイズ
	windowSize_ = { SCREEN_SIZE_X ,SCREEN_SIZE_Y };
	SetGraphMode(windowSize_.width_, windowSize_.height_, 32);
	ChangeWindowMode(true);

	const int FPS_RATE = 60;	//フレームレート固定
	fps_ = std::make_unique<FpsController>(FPS_RATE);

	//DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}
	AddFontResourceExA("Data/Font/cinecaption226.ttf", FR_PRIVATE, NULL);
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

}

void Application::Run(void)
{

	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();
	auto& imGuiWrapper = ImGuiWrapper::GetInstance();

	//ゲームループ
	while (ProcessMessage() == 0 && !isEnd_ )
	{
		inputManager.Update();
		imGuiWrapper.Update();
		sceneManager.Update();

		sceneManager.Draw();

		RenderVertex();

		imGuiWrapper.Draw();

		ScreenFlip();

		fps_->Wait();
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
	// ウィンドウズに一時的に保持していたフォントデータを削除
	RemoveFontResourceExA("", FR_PRIVATE, NULL);

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
	isEnd_ = false;
	windowSize_ = {};
	fps_ = nullptr;
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		// ウィンドウズに一時的に保持していたフォントデータを削除
		RemoveFontResourceExA("", FR_PRIVATE, NULL);

	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}
