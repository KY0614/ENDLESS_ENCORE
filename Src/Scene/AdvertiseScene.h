#pragma once
#include "SceneBase.h"
class AdvertiseScene : public SceneBase
{
public:
	//コンストラクタ
	AdvertiseScene(void);
	//デストラクタ
	~AdvertiseScene(void);

	//初期化処理
	void Init(void) override;
	//更新処理
	void Update(void) override;
	//描画処理*
	void Draw(void) override;

private:
	//ムービー
	int movieHandle_;		//ムービーのハンドル
	int movieFrame_;		//現在のムービーフレーム
	int totalMovieFrame_;	//ムービーの総フレーム数

};

