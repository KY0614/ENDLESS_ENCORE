#pragma once
#include "../../Common/Vector2.h"
#include "UIBase.h"

class ParryBar : public UIBase
{
public:

	//パリィバー情報構造体
	struct ParryBarInfo
	{
		Vector2 pos_ = {};	//位置
		Vector2 size_ = {};	//サイズ
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="parryBarInfo">パリィのCDバーの情報（位置、サイズ）</param>
	/// <param name="parryCD">パリィのCD</param>
	/// <param name="parryCDMax">パリィの最大CD</param>
	ParryBar(const ParryBarInfo& parryBarInfo,
		const float& parryCD,
		const float& parryCDMax);
	//デストラクタ
	~ParryBar(void)override;

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
	//パリィバーの情報
	ParryBarInfo parryBarInfo_;

	//CDの時間
	const float& parryCD_;		//パリィクールダウン
	const float& parryCDMax_;	//パリィクールダウンの最大値

	//パリィクールダウンバーの画像
	int parryCDBarImg_;	

	//バー本体以外のUI画像
	int barFrameImg_;		//バーのフレーム画像
	int barBackImg_;		//バーの背景画像

	//フォントハンドル
	int fontHandle_;

	/// <summary>
	/// パリィバーの画像の初期化
	/// </summary>
	void InitImage(void);
};

