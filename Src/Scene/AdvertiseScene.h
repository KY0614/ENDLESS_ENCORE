#pragma once
#include "SceneBase.h"
class AdvertiseScene : public SceneBase
{
public:
	//コンストラクタ
	AdvertiseScene(void);
	//デストラクタ
	~AdvertiseScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

	//初期化処理
	void Init(void) override;
	//更新処理
	void Update(void) override;
	//描画処理*
	void Draw(void) override;

private:
	//更新関数ポインタ型
	using UpdateFunc_t = void (AdvertiseScene::*)(void);	
	using DrawFunc_t = void (AdvertiseScene::*)(void);
	//更新関数テーブル
	UpdateFunc_t update_;
	DrawFunc_t draw_;

	//ムービー
	int movieHandle_;		//ムービーのハンドル
	int movieFrame_;		//現在のムービーフレーム
	int totalMovieFrame_;	//ムービーの総フレーム数

	void PlayDemoUpdate(void);	//デモムービー再生更新
	void RankingUpdate(void);	//ランキング表示更新

	void PlayDemoDraw(void);	//デモムービー再生描画
	void RankingDraw(void);		//ランキング表示描画
};

