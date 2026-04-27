#pragma once
#include "../Common/ActorBase.h"

class AnimationController;

class CharactorBase : public ActorBase
{
public:

	//衝突判定種別
	enum class COLLIDER_TYPE
	{
		SPHERE,	//球体
		LINE,	//線分
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
	virtual void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

protected:
	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//移動方向
	VECTOR moveDir_;

	//移動量
	VECTOR movePow_;

	//移動後の座標
	VECTOR movedPos_;

	// 移動前の座標
	VECTOR prevPos_;

	//ジャンプの力
	VECTOR jumpPow_;

	//ジャンプ判定
	bool isJump_;

	//丸影
	int imgShadow_;

	// 更新系
	virtual void UpdateProcess(void) = 0;
	virtual void UpdateProcessPost(void) = 0;

	// 移動方向に応じた遅延回転
	void DelayRotate(void);

	/// <summary>
	/// 重力計算処理
	/// </summary>
	void CalcGravityPower(void);

	void Collision(void);

	void CollisionGravity(void);

	/// <summary>
	/// 影の描画処理
	/// </summary>
	void DrawShadow(void);
};

