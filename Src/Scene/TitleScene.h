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

	//コンストラクタ
	TitleScene(void);

	//デストラクタ
	~TitleScene(void);

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

	//ポストエフェクト用スクリーン
	int postEffectScreen_;

	//ポストエフェクト用(映写機風)マテリアルとレンダラー
	std::unique_ptr<PixelMaterial> retroTheaterMaterial_;
	std::unique_ptr<PixelRenderer> retroTheaterRenderer_;

	//タイトルロゴ画像ハンドル
	int logoImg_;
	//プッシュスペース画像ハンドル
	int pushSpaceImg_;
	//プッシュスペース画像アルファ値
	int pushSpaceImgAlpha_;
	//プッシュスペース画像アルファ値
	int alphaChangeSpeed_;

	//ノイズテクスチャID（黒いシミっぽい画像)
	int noiseTextureId_;

	float filmScrollTime_;

	//フェードアウト開始するまでのインターバル時間
	float intervalTimer_;

	//スペースキーを押したかどうか
	bool isPushSpace_;

	//スペースキーを押したときの効果音のボリューム
	int pushSpaceSEVolume_;
	int seVolumeDecreaseFrame_;

	/// <summary>
	/// サウンド初期化処理
	/// </summary>
	void InitSound(void);

	/// <summary>
	/// マテリアル初期化処理
	/// </summary>
	void InitMaterial(void);
};
