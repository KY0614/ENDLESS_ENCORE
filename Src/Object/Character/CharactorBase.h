#pragma once
#include "ActorBase.h"

class CharactorBase : public ActorBase
{
public:

	//衝突判定種別
	enum class COLLIDER_TYPE
	{
		SPHERE,	//球体
		MAX,
	};

	//コンストラクタ
	CharactorBase(void);
	//デストラクタ
	virtual ~CharactorBase(void) override;

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

protected:
	//モデル制御の基本情報
	Transform transform_;

	//ジャンプの力
	VECTOR jumpPow_;

	//丸影
	int imgShadow_;

	/// <summary>
	/// 重力計算処理
	/// </summary>
	void CalcGravityPower(void);

	/// <summary>
	/// 影の描画処理
	/// </summary>
	void DrawShadow(void);
};

