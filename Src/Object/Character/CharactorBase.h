#pragma once
#include "../Common/ActorBase.h"

class AnimationController;

class CharactorBase : public ActorBase
{
public:

	//衝突判定種別
	enum class COLLIDER_TYPE
	{
		CAPSULE,//カプセル
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
	static constexpr float MAX_FALL_SPEED = -30.0f;	//最大落下速度
	// 衝突時の押し戻し試行回数
	static constexpr int CNT_TRY_COLLISION = 20;
	// 衝突時の押し戻し量
	static constexpr float COLLISION_BACK_DIS = 1.0f;

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

	// 衝突判定
	virtual void CollisionReserve(void) {}

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name=""></param>
	void Collision(void);

	/// <summary>
	/// カプセルによる衝突判定処理
	/// </summary>
	void CollisionCapsule(void);

	/// <summary>
	/// 重力による衝突判定処理
	/// </summary>
	void CollisionGravity(void);

	/// <summary>
	/// ジャンプアニメーションを途中から再生する処理
	/// </summary>
	/// <param name=""></param>
	virtual void JumpAnimationPlay(void) = 0;

	/// <summary>
	/// 影の描画処理
	/// </summary>
	void DrawShadow(void);
};

