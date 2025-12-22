#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class PixelMaterial;
class PixelRenderer;
class Stage;

class TitleScene : public SceneBase
{

public:
	static constexpr float HIGH_LIGHT_INTERVAL = 1.2f;

	//UVスケール
	static constexpr float TILLING_SIZE = 35.0f;

	static constexpr float AMBIENT_COLOR = 0.2f;

	static constexpr int LOGO_OFFSET_Y = 80;
	static constexpr int LOGO_HEIGHT = 1024;
	static constexpr int PUSHIMG_OFFSET_Y = 100;

	//コンストラクタ
	TitleScene(void);

	//デストラクタ
	~TitleScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:

	//ステージ
	std::shared_ptr<Stage> stage_;

	// ポストエフェクト用スクリーン
	int postEffectScreen_;

	// ポストエフェクト用(セピア調)
	std::unique_ptr<PixelMaterial> sepiaMaterial_;
	std::unique_ptr<PixelRenderer> sepiaRenderer_;

	// ポストエフェクト用(ビネット)周りを暗くするやつ
	std::unique_ptr<PixelMaterial> vignetteMaterial_;
	std::unique_ptr<PixelRenderer> vignetteRenderer_;

	// ポストエフェクト用(線ノイズ)
	std::unique_ptr<PixelMaterial> filmNoiseMaterial_;
	std::unique_ptr<PixelRenderer> filmNoiseRenderer_;

	//タイトルロゴ画像ハンドル
	int logoImg_;
	//プッシュスペース画像ハンドル
	int pushSpaceImg_;
	int pushSpaceImgAlpha_;
	bool isIncreaseAlpha_;

	//宣伝シーンへ遷移する用のタイマー
	int toAdvertiseLoopTimer_;

	//フェードアウト開始するまでのインターバル時間
	float intervalTimer_;

	//スペースキーを押したかどうか
	bool isPushSpace_;

	//スペースキーを押したときの効果音のボリューム
	int pushSpaceSEVolume_;
	int seVolumeDecreaseFrame_;

	void InitSound(void);

	void InitMaterial(void);
};
