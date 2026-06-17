#pragma once
#include "../../Common/Vector2.h"
#include "UIBase.h"

class HPBar : public UIBase
{
public:
	//HPバーの種類
	enum class TYPE
	{
		PLAYER,	//プレイヤーのHPバー
		ENEMY	//敵のHPバー
	};
	
	//HPバー情報構造体
	struct HPBarInfo
	{
		TYPE type_ = TYPE::PLAYER;		//HPバーの種類
		Vector2 pos_ = {};	//位置
		Vector2 size_ = {};	//サイズ
	};

	struct BillboardInfo
	{
		TYPE type_ = TYPE::PLAYER;		//HPバーの種類
		VECTOR* pos_ = {};	//位置
		VECTOR ofsset_ = {};//位置のオフセット
		float scaleX_ = 0.0f;	//拡大率
		float scaleY_ = 0.0f;	//拡大率
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="hpBarInfo">HPバー情報（種類、位置、サイズ、最大幅）</param>
	HPBar(const HPBarInfo& hpBarInfo,const float& hp);

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="billboardInfo">HPバー情報（ビルボードバージョン）</param>
	HPBar(const BillboardInfo& billboardInfo,const float& hp, const float& maxHp);
	//デストラクタ
	~HPBar(void)override = default;

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

	/// <summary>
	/// ワールド座標からスクリーン座標に変換して描画する
	/// </summary>
	/// <param name="">ビルボード描画てきな</param>
	void DrawBillboard(void);

private:
	//HPバーの情報
	HPBarInfo hpBarInfo_;
	BillboardInfo billboardInfo_;

	//HP(プレイヤーもしくは敵のHPを参照)
	const float& hp_;
	const float& maxHp_;

	//バー本体以外のUI画像
	int barFrameImg_;		//バーのフレーム画像
	int barBackImg_;		//バーの背景画像

	/// <summary>
	/// HPバーの画像の初期化
	/// </summary>
	void InitImage(void);
};

