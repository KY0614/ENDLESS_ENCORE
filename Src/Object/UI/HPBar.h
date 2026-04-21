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

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="hpBarInfo">HPバー情報（種類、位置、サイズ、最大幅）</param>
	HPBar(const HPBarInfo hpBarInfo,const float& hp);
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

private:
	//HPバーの情報
	HPBarInfo hpBarInfo_;

	//HP(プレイヤーもしくは敵のHPを参照)
	const float& hp_;

	//バー本体以外のUI画像
	int barFrameImg_;		//バーのフレーム画像
	int barBackImg_;		//バーの背景画像

	/// <summary>
	/// HPバーの画像の初期化
	/// </summary>
	void InitImage(void);
};

