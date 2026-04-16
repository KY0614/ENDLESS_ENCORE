#pragma once
#include "../../Common/Vector2.h"
#include "UIBase.h"

class SkipBar : public UIBase
{
public:
	//バーUIの種類
	enum class TYPE
	{
		HP,			//HP
		PARRY,		//パリィ
		PARRY_CD	//パリィクールダウン
	};

	//バーUI情報構造体
	struct SkipBarUIInfo
	{
		Vector2 pos_;	//位置
		Vector2 size_;	//サイズ	
	};

	//コンストラクタ
	SkipBar(const SkipBarUIInfo skipBarInfo,
		const float& skipTime,
		const float& skipTimeMax);
	//デストラクタ
	~SkipBar(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void) override;

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:
	//バーUI情報
	SkipBarUIInfo skipBarInfo_;

	//バーの進行度を表す時間
	const float& skipTime_;		//スキップの時間
	const float& skipTimeMax_;	//スキップの最大時間

	//バーUI背景画像
	int barUIFrameImg_;	//フレーム画像
	int uiBackImg_;		//背景画像

	/// <summary>
	/// スキップバーの画像の初期化
	/// </summary>
	void InitImage(void);
};