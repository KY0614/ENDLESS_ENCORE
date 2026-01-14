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
	//BGMの音量
	const int BGM_VOLUME = 60;
	//PushSpace画像のアルファ値最大
	const int PUSH_SPACE_IMG_ALPHA_MAX = 255;
	//PushSpaceSEの音量
	const int PUSH_SPACE_SE_VOLUME = 60;
	//スペースキー押下時のSE音量が下がるフレーム間隔
	const int PUSH_SPACE_SE_VOLUME_DECREASE_FRAME = 3;
	//SEフェードアウトにかける総時間
	const float SE_FADE_OUT_TOTALTIME = 6.0f;
	//フェードアウト開始までの間隔時間
	const float INTERVAL_TIME = 1.5f;
	//定点カメラの位置
	const VECTOR CAMERA_FIXED_POINT_POS = VGet(0.0f, -109.0f, -65.0f);			//カメラの位置
	const VECTOR CAMERA_FIXED_POINT_TARGET_POS = VGet(0.0f, -153.0f, 2863.0f);	//カメラの注視点位置
}

TitleScene::TitleScene(void)
{
	logoImg_ = -1;
	pushSpaceImg_ = -1;
	pushSpaceImgAlpha_ = 0;
	alphaChangeSpeed_ = 0;
	intervalTimer_ = 0.0f;
	isPushSpace_ = false;
	pushSpaceSEVolume_ = 0;
	seVolumeDecreaseFrame_ = 0;
	postEffectScreen_ = 0;
}

TitleScene::~TitleScene(void)
{
	DeleteGraph(postEffectScreen_);
}

void TitleScene::Init(void)
{
	//サウンド初期化
	InitSound();

	//ステージ
	stage_ = std::make_shared<Stage>();
	stage_->Init();

	//タイトルロゴ画像
	logoImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_LOGO).handleId_;

	//プッシュスペース画像
	pushSpaceImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PUSH_SPACE).handleId_;

	//プッシュスペース画像のアルファ値変化速度
	const int alphaChangeSpeed = 2;
	alphaChangeSpeed_ = alphaChangeSpeed;

	//定点カメラの座標設定
	mainCamera->SetFixedPointPos(CAMERA_FIXED_POINT_POS, CAMERA_FIXED_POINT_TARGET_POS);
	//定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);

	//ポストエフェクト用スクリーン
	postEffectScreen_ = MakeScreen(
		Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, true);

	//マテリアル初期化
	InitMaterial();
}

void TitleScene::Update(void)
{
	//画面に出す黒い線のノイズのX座標をランダムに更新
	const int randomValue = 100;
	const float random = 100.0f;
	float randomNoiseLineX = static_cast<float>(rand() % randomValue) / random;
	filmNoiseMaterial_->SetConstBuf(0, { randomNoiseLineX, 0.0f, 0.0f, 0.0f });

	//プッシュスペース画像をゆっくり点滅させる
	pushSpaceImgAlpha_ += alphaChangeSpeed_;
	//アルファ値が最大値か最小値になったら変化速度の符号を反転させる
	if (pushSpaceImgAlpha_ >= PUSH_SPACE_IMG_ALPHA_MAX || pushSpaceImgAlpha_ <= 0)
	{
		alphaChangeSpeed_ *= -1; //符号を反転
		pushSpaceImgAlpha_ = std::clamp(pushSpaceImgAlpha_, 0, PUSH_SPACE_IMG_ALPHA_MAX); //範囲外を補正
	}

	InputManager& ins = InputManager::GetInstance();
	SoundManager& sound = SoundManager::GetInstance();
	//スペースキーが押されたら効果音を鳴らす
	if (ins.IsInputTriggered("Parry") && !isPushSpace_)
	{
		isPushSpace_ = true;
		sound.Play(SoundManager::SOUND::PUSH_SPACE);
	}

	stage_->Update();

	if (!isPushSpace_)return;

	//インターバル時間を増やしていく
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	//SEフェードアウト処理(徐々に音量を下げていく)
	if (intervalTimer_ < INTERVAL_TIME / 2.0f)return;

	seVolumeDecreaseFrame_++;
	//SE音量を徐々に下げていく
	seVolumeDecreaseFrame_% PUSH_SPACE_SE_VOLUME_DECREASE_FRAME == 0 ? pushSpaceSEVolume_-- : 0;
	sound.AdjustVolume(SoundManager::SOUND::PUSH_SPACE, pushSpaceSEVolume_);

	//ゲームシーンへ遷移
	if (intervalTimer_ > INTERVAL_TIME)
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
	//画面の比率
	const float& screenAspectRatio = 
		SceneManager::GetInstance().GetScreenAspectRatio();

	stage_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();
	// ポストエフェクト(セピア)
	//-----------------------------------------

	SetDrawScreen(postEffectScreen_);

	// 画面を初期化
	ClearDrawScreen();
	sepiaRenderer_->Draw();

	// メインに戻す
	SetDrawScreen(mainScreen);
	DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------
	// ポストエフェクト(ビネット)
	//-----------------------------------------
	SetDrawScreen(postEffectScreen_);

	// 画面を初期化
	ClearDrawScreen();

	vignetteRenderer_->Draw();

	// メインに戻す
	SetDrawScreen(mainScreen);
	DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------
	// ポストエフェクト(線ノイズ)
	//-----------------------------------------
	SetDrawScreen(postEffectScreen_);

	// 画面を初期化
	ClearDrawScreen();

	filmNoiseRenderer_->Draw();

	// メインに戻す
	SetDrawScreen(mainScreen);
	DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------

	//ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	//ロゴ画像の大きさ
	float logoScale = 1.5f;
	//タイトルロゴ描画
	const int titleLogoOffsetY = 100;
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - titleLogoOffsetY,
		logoScale * screenAspectRatio, 0.0f,
		logoImg_, true);

	//点滅させるためのアルファ値設定
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, pushSpaceImgAlpha_);
	logoScale = 1.0f;
	//プッシュスペース描画
	const int pushSpaceOffsetY = 256;
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y - pushSpaceOffsetY,
		logoScale * screenAspectRatio, 0.0f,
		pushSpaceImg_, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::InitSound(void)
{
	//SE音量設定
	pushSpaceSEVolume_ = PUSH_SPACE_SE_VOLUME;
	//BGM
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::TITLE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::TITLE, BGM_VOLUME);
	sound.Play(SoundManager::SOUND::TITLE);
	//SE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PUSH_SPACE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PUSH_SPACE_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::PUSH_SPACE, pushSpaceSEVolume_);

}

void TitleScene::InitMaterial(void)
{
	int materialConstBufSize = 1;
	// ポストエフェクト用(セピア)
	sepiaMaterial_ = std::make_unique<PixelMaterial>("Sepiatone.cso", materialConstBufSize);
	//モデルカラー
	const FLOAT4 modelColor = { 1.0f,1.0f,1.0f,1.0f, };
	sepiaMaterial_->AddConstBuf(modelColor);
	sepiaMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	sepiaRenderer_ = std::make_unique<PixelRenderer>(*sepiaMaterial_);
	sepiaRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

	// ポストエフェクト用(ビネット)
	vignetteMaterial_ = std::make_unique<PixelMaterial>("Vignette.cso", materialConstBufSize);
	//ビネットの強さ
	const float vignettePower = 3.0f;
	vignetteMaterial_->AddConstBuf({ vignettePower, 0.0f, 0.0f, 0.0f });
	vignetteMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	vignetteRenderer_ = std::make_unique<PixelRenderer>(*vignetteMaterial_);
	vignetteRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);
	materialConstBufSize = 2;
	//ノイズを入れる線のX座標をランダムに決定
	const int randomValue = 100;
	const float random = 100.0f;
	float randomNoiseLineX = static_cast<float>(rand() % randomValue) / random;
	// ポストエフェクト用(線ノイズ)
	filmNoiseMaterial_ = std::make_unique<PixelMaterial>("FilmNoise.cso", materialConstBufSize);
	filmNoiseMaterial_->AddConstBuf({ randomNoiseLineX, 0.0f, 0.0f, 0.0f });
	filmNoiseMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	filmNoiseRenderer_ = std::make_unique<PixelRenderer>(*filmNoiseMaterial_);
	filmNoiseRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);
}