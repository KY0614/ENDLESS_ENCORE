#include <ranges>
#include <chrono>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../../Application.h"
#include "../../Utility/CommonUtility.h"
#include "../../Common/Fader.h"
#include "../../Scene/TitleScene.h"
#include "../../Scene/GameScene.h"
#include "../GameSystem/SoundManager.h"
#include "JsonManager.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "SceneManager.h"

namespace
{
	//画面揺らしのフレーム数
	const int SHAKE_FRAME = 60;

	//フォグの開始・終了距離
	const float FOG_START = 10000.0f;	//フォグ開始距離
	const float FOG_END = 20000.0f;		//フォグ終了距離

	//画面揺れの減衰率
	const float SHAKE_DECEL_RATE = 0.95f;
}

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{
	SoundManager::CreateInstance();
	JsonManager::CreateInstance();
	JsonManager::CreateInstance();

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	fader_ = std::make_shared<Fader>();
	fader_->Init();

	//カメラ
	camera_ = std::make_shared<Camera>();
	camera_->Init();

	isSceneChanging_ = false;

	//デルタタイム
	preTime_ = std::chrono::system_clock::now();

	//フォグ
	fogStart_ = FOG_START;
	fogEnd_ = FOG_END;

	//ライトの向き
	lightDir_ = LIGHT_DIR;

	mainScreen_ = MakeScreen(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y,true);
	shakeFrame_ = 0;
	shakeRate_ = 0.0f;

	//3D用の設定
	Init3D();

	//初期シーンの設定
	DoChangeScene(SCENE_ID::TITLE);
}

void SceneManager::Init3D(void)
{
	//背景色設定
	const int bgColorRed = 64;
	const int bgColorGreen = 64;
	const int bgColorBlue = 128;
	SetBackgroundColor(bgColorRed, bgColorGreen, bgColorBlue);

	//Zバッファを有効にする
	SetUseZBuffer3D(true);

	//Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);
	SetWriteZBufferFlag(true);

	//バックカリングを有効にする
	SetUseBackCulling(true);

	//ライトの設定
	SetUseLighting(true);
	
	//ライトの設定
	ChangeLightTypeDir(lightDir_);

	//フォグ設定
	ResetFog();
}

void SceneManager::Update(void)
{
	// 非同期読み込み中の処理数がゼロになるまで処理しない
	if (GetASyncLoadNum() != 0)
	{
		return;
	}
	//画面揺らし
	ShakeScreen();

	ChangeLightTypeDir(lightDir_);
	if (scenes_.empty())
	{
		return;
	}

	//デルタタイム
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	fader_->Update();
	if (isSceneChanging_)
	{
		Fade();
	}
	else
	{
		//scene_->Update();
		scenes_.back()->Update();
	}

	//カメラ更新
	camera_->Update();
}

void SceneManager::Draw(void)
{
	//描画先グラフィック領域の指定
	//(３Ｄ描画で使用するカメラの設定などがリセットされる)
	SetDrawScreen(mainScreen_);
	//画面を初期化
	ClearDrawScreen();

	//カメラ設定
	camera_->SetBeforeDraw();

	//Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	//描画
	for (auto& scene : scenes_)
	{
		scene->Draw();
	}

	//主にポストエフェクト用
	camera_->Draw();

	//Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();
	
	//暗転・明転
	fader_->Draw();

	SetDrawScreen(DX_SCREEN_BACK);

	if (shakeFrame_ == 0)
	{
		ClearDrawScreen();
		DrawGraph(0, 0, mainScreen_, false);
	}
	if (shakeFrame_ > 0)
	{
		Vector2 pos;
		//画面を揺らす
		const int shakeFrameRate = 3;
		const int shakePadding = 5;
		pos.x = (int)(((shakeFrame_ % shakeFrameRate) * shakePadding) * shakeRate_);
		pos.y = 0;
		DrawGraph(pos.x, 0, mainScreen_, false);
	}
}

void SceneManager::Destroy(void)
{
	SoundManager::GetInstance().Destroy();
	JsonManager::GetInstance().Destroy();
	DeleteGraph(mainScreen_);
	delete instance_;
}

void SceneManager::ChangeScene(SCENE_ID nextId)
{
	//フェード処理が終わってからシーンを変える場合もあるため、
	//遷移先シーンをメンバ変数に保持
	waitSceneId_ = nextId;

	//フェードアウト(暗転)を開始する
	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;
}

SceneManager::SCENE_ID SceneManager::GetSceneID(void)
{
	 return sceneId_;
}

float SceneManager::GetDeltaTime(void) const
{
	//return 1.0f / 60.0f;
	return deltaTime_;
}

std::weak_ptr<Camera> SceneManager::GetCamera(void) const
{
	return camera_;
}

std::weak_ptr<Fader> SceneManager::GetFader(void) const
{
	return fader_;
}

void SceneManager::PushScene(SCENE_ID _scene)
{
	sceneId_ = _scene;
	//新しく積むのでもともと入っている奴はまだ削除されない
	scenes_.push_back(std::move(CreateScene(_scene)));
	scenes_.back()->Init();
}

void SceneManager::PopScene(void)
{
	//積んであるものをけして、もともとあったものを末尾にする
	if (scenes_.size() > 1) 
	{
		scenes_.pop_back();
	}
}

void SceneManager::JumpScene(std::unique_ptr<SceneBase> scene)
{
	scenes_.clear();
	scenes_.push_back(std::move(scene));
}

void SceneManager::SetFog(const float fogStart, const float fogEnd)
{
	SetFogStartEnd(fogStart, fogEnd);
}

void SceneManager::ResetFog(void)
{
	//フォグ設定
	SetFogEnable(true);
	SetFogColor(5, 5, 5);
	SetFogStartEnd(fogStart_, fogEnd_);
}

void SceneManager::StartShakeScreen(void)
{
	//画面揺らしのフレームとレートを設定
	shakeFrame_ = SHAKE_FRAME;
	shakeRate_ = 1.0f;
}

bool SceneManager::IsFadeOutEnd(void)
{
	//true:フェードアウト終了 false:まだ終了していない
	return fader_->GetState() == Fader::STATE::FADE_OUT &&
		fader_->IsEnd();
}

bool SceneManager::IsFadeInEnd(void)
{
	//true:フェードイン終了 false:まだ終了していない
	return fader_->GetState() == Fader::STATE::FADE_IN &&
		fader_->IsEnd();
}

const float SceneManager::GetScreenAspectRatio(void) const
{
	return static_cast<float>(Application::SCREEN_SIZE_Y) /
		static_cast<float>(Application::SCREEN_MAX_SIZE_Y);
}

SceneManager::SceneManager(void)
{
	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	scenes_.clear();
	fader_ = nullptr;

	isSceneChanging_ = false;

	//デルタタイム
	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;
	lightDir_ = CommonUtility::VECTOR_ZERO;

	shakeRate_ = 0.0f;
	shakeFrame_ = 0;

	mainScreen_ = -1;
	fogStart_ = FOG_START;
	fogEnd_ = FOG_END;
}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{
	auto& resM = ResourceManager::GetInstance();
	auto& jsonM = JsonManager::GetInstance();
	//リソースの解放
	resM.Release();
	jsonM.Release();
	SoundManager::GetInstance().Release();

	//シーンを変更する
	sceneId_ = sceneId;

	//現在のシーンを解放
	if (scene_ != nullptr)
	{
		scene_.reset();
	}

	if (scenes_.empty())
	{
		//空だったら新しく入れる
		scenes_.push_back(std::move(CreateScene(sceneId)));
	}
	else
	{
		//末尾のものを新しい物に入れ替える
		scenes_.back() = std::move(CreateScene(sceneId));
	}

	scenes_.back()->Init();

	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;

}

void SceneManager::Fade(void)
{
	Fader::STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		//明転中
		if (fader_->IsEnd())
		{
			//明転が終了したら、フェード処理終了
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	case Fader::STATE::FADE_OUT:
		//暗転中
		if (fader_->IsEnd())
		{
			//完全に暗転してからシーン遷移
			DoChangeScene(waitSceneId_);
			//暗転から明転へ
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;
	}
}

void SceneManager::ShakeScreen(void)
{
	//画面揺らし
	if (shakeFrame_ > 0)
	{
		shakeFrame_--;
		shakeRate_ *= SHAKE_DECEL_RATE;
	}
	else 
	{
		shakeRate_ = 0.0f;
	}
}

template<typename T>
std::unique_ptr<T> SceneManager::CreateScene(SCENE_ID sceneId)
{

	auto& resM = ResourceManager::GetInstance();
	auto& jsonM = JsonManager::GetInstance();

	std::unique_ptr<SceneBase> scene;
	switch (sceneId)
	{
	case SceneManager::SCENE_ID::NONE:
		break;

	case SceneManager::SCENE_ID::TITLE:
		scene = std::make_unique<TitleScene>();
		resM.InitTitle();
		jsonM.InitTitle();
		break;

	case SceneManager::SCENE_ID::GAME:
		scene = std::make_unique<GameScene>();
		resM.InitGame();
		jsonM.InitGame();
		
		break;

	default:
		break;
	}
	return scene;
}
