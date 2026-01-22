#include <DxLib.h>
#include <random>
#include "../Application.h"
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
	//フィルム回転の音量
	const int FILM_SCROLL_VOLUME = 40;
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
	//ビネットの強さ
	const float VIGNETTE_POWER = 3.0f; 
}

TitleScene::TitleScene(void)
{
	logoImg_ = -1;
	pushSpaceImg_ = -1;
	noiseTextureId_ = -1;
	pushSpaceImgAlpha_ = 0;
	alphaChangeSpeed_ = 0;
	intervalTimer_ = 0.0f;
	isPushSpace_ = false;
	pushSpaceSEVolume_ = 0;
	seVolumeDecreaseFrame_ = 0;
	postEffectScreen_ = 0;
	filmScrollTime_ = 0.0f;
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
	logoImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::TITLE_LOGO).handleId_;

	//プッシュスペース画像
	pushSpaceImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PUSH_SPACE).handleId_;

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
	//0.0～1.0の範囲でランダムに決定(uv座標用)
	float randomNoiseLineX = static_cast<float>(rand() % 100) / 100.0f;//0.0～1.0fにおさめるので100.0fで割る
	//白の強さ（0.0～0.08fにおさめるので81で割った余りを1000.0fで割る）
	float whitePow = static_cast<float>(rand() % 81) / 1000.0f;
	std::uniform_real_distribution<float> uv(0.0f, 1.0f);
	// 乱数生成器の初期化
	std::random_device rd; //非決定的な乱数生成器
	std::mt19937 engine(rd()); //メルセンヌ・ツイスタ法による乱数生成器
	//画面に出す黒い線のノイズのX座標をランダムに更新
	retroTheaterMaterial_->SetConstBuf(1, { randomNoiseLineX, uv(engine), uv(engine), whitePow });
	const float scrollSpeed = 0.5f;
	filmScrollTime_ += SceneManager::GetInstance().GetDeltaTime() * scrollSpeed;
	retroTheaterMaterial_->SetConstBuf(2, { VIGNETTE_POWER, filmScrollTime_, 0.0f, 0.0f });

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
	//ステージ描画
	stage_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();
	// ポストエフェクト
	//-----------------------------------------

	SetDrawScreen(postEffectScreen_);

	// 画面を初期化
	ClearDrawScreen();
	retroTheaterRenderer_->Draw();

	// メインに戻す
	SetDrawScreen(mainScreen);
	DrawGraph(0, 0, postEffectScreen_, false);
	//-----------------------------------------
	
	//画面の比率
	const float& screenAspectRatio =
		SceneManager::GetInstance().GetScreenAspectRatio();

	//ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	//ロゴ画像の大きさ
	float logoScale = 1.5f;
	//ロゴのY座標(画面を10分割したうちの4/10の位置)
	int logoPosY = (Application::SCREEN_SIZE_Y / 10) * 4;
	//タイトルロゴ描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		logoPosY,
		logoScale * screenAspectRatio, 0.0f,
		logoImg_, true);

	//点滅させるためのアルファ値設定
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, pushSpaceImgAlpha_);
	//ロゴのY座標(画面を10分割したうちの7/10の位置)
	logoPosY = (Application::SCREEN_SIZE_Y / 10) * 7;
	//プッシュスペース描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
		logoPosY,
		screenAspectRatio, 0.0f,
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
	//フィルムスクロールSE
	sound.Add(SoundManager::TYPE::LOOP_SE, SoundManager::SOUND::FILM_SCROLL,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::FILM_SCROLL_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::FILM_SCROLL, FILM_SCROLL_VOLUME);
	sound.Play(SoundManager::SOUND::FILM_SCROLL);
	//ゲーム開始ボタンを押した時のSE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PUSH_SPACE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PUSH_SPACE_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::PUSH_SPACE, pushSpaceSEVolume_);
}

void TitleScene::InitMaterial(void)
{
	//マテリアルの定数バッファサイズ
	int materialConstBufSize = 3;
	// ポストエフェクト用
	retroTheaterMaterial_ = std::make_unique<PixelMaterial>(
		"RetroTheater.cso", materialConstBufSize);	
	retroTheaterMaterial_->SetTextureAddress(PixelMaterial::TEXADDRESS::WRAP);
	//モデルカラー
	const FLOAT4 modelColor = { 1.0f,1.0f,1.0f,1.0f, };
	retroTheaterMaterial_->AddConstBuf(modelColor);
	//ノイズを入れる線のX座標をランダムに決定
	const int randomValue = 100;
	const float random = 100.0f;
	float randomNoiseLineX = static_cast<float>(rand() % randomValue) / random;
	retroTheaterMaterial_->AddConstBuf({ randomNoiseLineX,randomNoiseLineX,0.0f,0.0f, });
	//ビネットの強さ
	retroTheaterMaterial_->AddConstBuf({ VIGNETTE_POWER, 0.0f, 0.0f, 0.0f });
	retroTheaterMaterial_->AddTextureBuf(SceneManager::GetInstance().GetMainScreen());
	
	//ノイズテクスチャ読み込み
	noiseTextureId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FILM_NOISE).handleId_;
	//ノイズテクスチャ設定
	retroTheaterMaterial_->AddTextureBuf(noiseTextureId_);
	retroTheaterRenderer_ = std::make_unique<PixelRenderer>(*retroTheaterMaterial_);
	retroTheaterRenderer_->MakeSquereVertex(
		Vector2(0, 0),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);
}