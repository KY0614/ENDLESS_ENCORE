#include <chrono>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../../Libs/ImGui/imgui.h"
#include "../../Utility/AsoUtility.h"
#include "../../Common/Fader.h"
#include "../../Scene/TitleScene.h"
#include "../../Scene/MovieScene.h"
#include "../../Scene/SelectScene.h"
#include "../../Scene/PauseScene.h"
#include "../../Scene/TutorialScene.h"
#include "../../Scene/GameScene.h"
#include "../../Scene/ResultScene.h"
#include "Camera.h"
#include "../GameSystem/SoundManager.h"
#include "../../Object/UI/UIManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"

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
	UIManager::CreateInstance();

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	fader_ = std::make_unique<Fader>();
	fader_->Init();

	//カメラ
	camera_ = std::make_shared<Camera>();
	camera_->Init();

	isSceneChanging_ = false;

	//デルタタイム
	preTime_ = std::chrono::system_clock::now();

	//ライトの向き
	lightDir_ = LIGHT_DIR;

	//3D用の設定
	Init3D();

	//初期シーンの設定
	DoChangeScene(SCENE_ID::GAME);

}

void SceneManager::Init3D(void)
{
	//SetBackgroundColor(0, 139, 139);
	//背景色設定
	SetBackgroundColor(64, 64, 128);

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
	//ChangeLightTypeDir({ 0.3f, -0.7f, 0.8f });
	ChangeLightTypeDir(lightDir_);

	//フォグ設定
	SetFogEnable(true);
	SetFogColor(5, 5, 5);
	SetFogStartEnd(10000.0f, 20000.0f);
}

void SceneManager::Update(void)
{
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
	SetDrawScreen(DX_SCREEN_BACK);

	//画面を初期化
	ClearDrawScreen();

	//カメラ設定
	camera_->SetBeforeDraw();

	//Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	//描画
	//scene_->Draw();
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
}

void SceneManager::Destroy(void)
{
	SoundManager::GetInstance().Destroy();
	UIManager::GetInstance().Destroy();
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

void SceneManager::ChangeScene(std::unique_ptr<SceneBase> _scene)
{
	if (scenes_.empty()) {
		//空だったら新しく入れる
		scenes_.push_back(std::move(_scene));
	}
	else {
		//末尾のものを新しい物に入れ替える
		scenes_.back() = std::move(_scene);
	}
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

void SceneManager::PushScene(std::unique_ptr<SceneBase> _scene)
{
	//新しく積むのでもともと入っている奴はまだ削除されない
	scenes_.push_back(std::move(_scene));
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
	lightDir_ = AsoUtility::VECTOR_ZERO;
}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{
	auto& resM = ResourceManager::GetInstance();
	//リソースの解放
	resM.Release();
	SoundManager::GetInstance().Release();
	UIManager::GetInstance().Release();

	//シーンを変更する
	sceneId_ = sceneId;

	//現在のシーンを解放
	if (scene_ != nullptr)
	{
		scene_.reset();
	}
	
	MakeScene(sceneId);

	scenes_.back()->Init();
	//scene_->Init();

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

void SceneManager::MakeScene(SCENE_ID sceneId)
{
	auto& resM = ResourceManager::GetInstance();
	std::unique_ptr<SceneBase> scene;
	switch (sceneId)
	{
	case SceneManager::SCENE_ID::NONE:
		break;
	
	case SceneManager::SCENE_ID::TITLE:
		scene = std::make_unique<TitleScene>();
		resM.InitTitle();
		break;
	
	case SceneManager::SCENE_ID::MOVIE:
		scene = std::make_unique<MovieScene>();
		break;
	
	case SceneManager::SCENE_ID::SELECT:
		scene = std::make_unique<SelectScene>();
		break;
	
	case SceneManager::SCENE_ID::TUTORIAL:
		scene = std::make_unique<TutorialScene>();
		resM.InitTutorial();
		break;
	
	case SceneManager::SCENE_ID::GAME:
		scene = std::make_unique<GameScene>();
		resM.InitGame();
		break;
	
	case SceneManager::SCENE_ID::PAUSE:
		scene = std::make_unique<PauseScene>();
		break;

	case SceneManager::SCENE_ID::RESULT:
		scene = std::make_unique<ResultScene>();
		resM.InitResult();
		break;
	
	default:
		break;
	}

	if (scenes_.empty())
	{
		//空だったら新しく入れる
		scenes_.push_back(std::move(scene));
	}
	else
	{
		//末尾のものを新しい物に入れ替える
		scenes_.back() = std::move(scene);
	}
}

void SceneManager::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Camera");

	//位置
	ImGui::Text("position");
	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("Pos", &lightDir_.x);
	ImGui::SliderFloat("PosX", &lightDir_.x, -10.0f, 10.0f);
	ImGui::SliderFloat("PosY", &lightDir_.y, -10.0f, 10.0f);
	ImGui::SliderFloat("PosZ", &lightDir_.z, -10.0f, 10.0f);

	//終了処理
	ImGui::End();
}