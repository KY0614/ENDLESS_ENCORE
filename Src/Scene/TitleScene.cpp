#include <DxLib.h>
#include "../Application.h"
#include "../Common/Easing.h"
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Object/Stage.h"
#include "TitleScene.h"

namespace
{
	//宣伝シーンへ遷移する時間
	const int ADVERTISE_TIME = 1500;
	//PushSpaceSEの音量
	const int PUSH_SPACE_SE_VOLUME = 40;
	//スペースキー押下時のSE音量が下がるフレーム間隔
	const int PUSH_SPACE_SE_VOLUME_DECREASE_FRAME = 3;
	//SEフェードアウトにかける総時間
	const float SE_FADE_OUT_TOTALTIME = 6.0f;
	//フェードアウト開始までの間隔時間
	const float INTERVAL_TIME = 1.5f;
}

TitleScene::TitleScene(void)
{
	toAdvertiseLoopTimer_ = 255;
	logoImg_ = -1;
	pushSpaceImg_ = -1;
	pushSpaceImgAlpha_ = 0;
	intervalTimer_ = 0.0f;
	isPushSpace_ = false;
	isIncreaseAlpha_ = false;
	pushSpaceSEVolume_ = 0;
	seVolumeDecreaseFrame_ = 0;
	postEffectScreen_ = 0;
}

TitleScene::~TitleScene(void)
{
	DeleteGraph(postEffectScreen_);
}

void TitleScene::LoadData(void)
{
}

void TitleScene::Init(void)
{
	//SE音量設定
	pushSpaceSEVolume_ = PUSH_SPACE_SE_VOLUME;	
	//BGM
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::TITLE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::TITLE, 35);
	sound.Play(SoundManager::SOUND::TITLE);
	//SE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PUSH_SPACE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PUSH_SPACE_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::PUSH_SPACE, pushSpaceSEVolume_);

	//テージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();

	//タイトルロゴ
	logoImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_LOGO).handleId_;

	//プッシュスペース画像
	pushSpaceImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PUSH_SPACE).handleId_;

	toAdvertiseLoopTimer_ = ADVERTISE_TIME;

	mainCamera->SetFixedPointPos(VGet(0.0f, -109.0f, -65.0f),VGet(0.0f, -153.0f, 2863.0f));
	//定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);

	// ポストエフェクト用スクリーン
	postEffectScreen_ = MakeScreen(
		Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, true);

	// ポストエフェクト用(セピア)
	sepiaMaterial_ = std::make_unique<PixelMaterial>("Sepiatone.cso", 1);
	sepiaMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	sepiaMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	sepiaRenderer_ = std::make_unique<PixelRenderer>(*sepiaMaterial_);
	sepiaRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

	// ポストエフェクト用(ビネット)
	vignetteMaterial_ = std::make_unique<PixelMaterial>("Vignette.cso", 1);
	vignetteMaterial_->AddConstBuf({ 3.0f, 0.0f, 0.0f, 0.0f });
	vignetteMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	vignetteRenderer_ = std::make_unique<PixelRenderer>(*vignetteMaterial_);
	vignetteRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);
	float randomNoiseLineX = static_cast<float>(rand() % 100) / 100.0f;
	// ポストエフェクト用(線ノイズ)
	filmNoiseMaterial_ = std::make_unique<PixelMaterial>("FilmNoise.cso", 2);
	filmNoiseMaterial_->AddConstBuf({ randomNoiseLineX, 0.0f, 0.0f, 0.0f });
	filmNoiseMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	filmNoiseRenderer_ = std::make_unique<PixelRenderer>(*filmNoiseMaterial_);
	filmNoiseRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

}

void TitleScene::Update(void)
{
	//画面に出す黒い線のノイズのX座標をランダムに更新
	float randomNoiseLineX = static_cast<float>(rand() % 100) / 100.0f;
	filmNoiseMaterial_->SetConstBuf(0, { randomNoiseLineX, 0.0f, 0.0f, 0.0f });

	if (!isIncreaseAlpha_)
	{
		pushSpaceImgAlpha_+=2;
		if (pushSpaceImgAlpha_ >= 255)
		{
			pushSpaceImgAlpha_ = 255;
			isIncreaseAlpha_ = !isIncreaseAlpha_;
		}
	}
	else
	{
		pushSpaceImgAlpha_-= 2;
		if (pushSpaceImgAlpha_ <= 0)
		{
			pushSpaceImgAlpha_ = 0;
			isIncreaseAlpha_ = !isIncreaseAlpha_;
		}
	}

	InputManager& ins = InputManager::GetInstance();
	SoundManager& sound = SoundManager::GetInstance();
	//スペースキーが押されたら効果音を鳴らす
	if (ins.IsInputTriggered("Parry") && !isPushSpace_)
	{
		isPushSpace_ = true;
		sound.Play(SoundManager::SOUND::PUSH_SPACE);
	}

	//一定時間経過で宣伝シーンへ遷移
	//if (--toAdvertiseLoopTimer_ <= 0)
	//{
	//	toAdvertiseLoopTimer_ = ADVERTISE_TIME;
	//	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::ADVERTISE);
	//	return;
	//}

	stage_->Update();

	if (!isPushSpace_)return;
	//宣伝シーンに遷移しないようにタイマーをリセット
	toAdvertiseLoopTimer_ = ADVERTISE_TIME;
	//インターバル時間を増やしていく
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	if (intervalTimer_ < INTERVAL_TIME / 2.0f)return;

	seVolumeDecreaseFrame_++;
	//SE音量を徐々に下げていく
	seVolumeDecreaseFrame_% PUSH_SPACE_SE_VOLUME_DECREASE_FRAME == 0 ? pushSpaceSEVolume_-- : 0;
	sound.AdjustVolume(SoundManager::SOUND::PUSH_SPACE, pushSpaceSEVolume_);


	if (intervalTimer_ > INTERVAL_TIME)
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
	DrawString(Application::SCREEN_SIZE_X/2, 
		Application::SCREEN_SIZE_Y / 2, L"Push Enter or B", 0xFFFFFF);

	stage_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();
	// ポストエフェクト(セピア)
	//-----------------------------------------

	//SetDrawScreen(postEffectScreen_);

	//// 画面を初期化
	//ClearDrawScreen();
	//sepiaRenderer_->Draw();

	//// メインに戻す
	//SetDrawScreen(mainScreen);
	//DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------
	// ポストエフェクト(ビネット)
	//-----------------------------------------
	//SetDrawScreen(postEffectScreen_);

	//// 画面を初期化
	//ClearDrawScreen();

	//vignetteRenderer_->Draw();

	//// メインに戻す
	//SetDrawScreen(mainScreen);
	//DrawGraph(0, 0, postEffectScreen_, false);
	////-----------------------------------------
	//// ポストエフェクト(線ノイズ)
	////-----------------------------------------
	//SetDrawScreen(postEffectScreen_);

	//// 画面を初期化
	//ClearDrawScreen();

	//filmNoiseRenderer_->Draw();

	//// メインに戻す
	//SetDrawScreen(mainScreen);
	//DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------

	//ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	//タイトルロゴ描画
	const int titleLogoOffsetY = 100;
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - titleLogoOffsetY,
		1.5f, 0.0f,
		logoImg_, true);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, pushSpaceImgAlpha_);

	//プッシュスペース描画
	const int pushSpaceOffsetY = 256;
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y - pushSpaceOffsetY,
		1.0f, 0.0f,
		pushSpaceImg_, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}