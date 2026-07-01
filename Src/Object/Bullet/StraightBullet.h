#pragma once
#include "BulletBase.h"

class StraightBullet : public BulletBase
{
public:
	//コンストラクタ
	StraightBullet(Transform& parent, VECTOR targetPos);

	//デストラクタ
	~StraightBullet(void)override;

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void) override;

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// ターゲット座標を設定
	/// </summary>
	/// <param name="targetPos">ターゲット座標</param>
	void SetTargetPos(const VECTOR targetPos) { targetPos_ = targetPos; }

protected:

	/// <summary>
	/// モデルの基本情報を初期化
	/// </summary>
	/// <param name=""></param>
	void InitTransform(void)override;

	/// <summary>
	/// 当たり判定用のコライダーを初期化
	/// </summary>
	void InitCollider(void)override;

	//状態遷移処理--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void)override;
	/// <summary>
	/// 状態遷移：READY
	/// </summary>
	void ChangeStateReady(void)override;
	/// <summary>
	/// 状態遷移：SHOT
	/// </summary>
	void ChangeStateShot(void)override;
	/// <summary>
	/// 状態遷移：REVERSE
	/// </summary>
	void ChangeStateReverse(void)override;
	/// <summary>
	/// 状態遷移：DESTROY
	/// </summary>
	void ChangeStateDestroy(void)override;

	//状態更新処理------------------------------------------------------

	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void)override;
	/// <summary>
	/// 更新：READY
	/// </summary>
	void UpdateReady(void)override;
	/// <summary>
	/// 更新：SHOT
	/// </summary>
	void UpdateShot(void)override;
	/// <summary>
	///	 更新：REVERSE
	/// </summary>
	void UpdateReverse(void)override;
	/// <summary>
	/// 更新：DESTROY
	/// </summary>
	void UpdateDestroy(void)override;

	//座標制御------------------------------------------------------

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(void)override;

	/// <summary>
	/// 反射移動処理
	/// </summary>
	void ReserveMove(void);

private:
	//ターゲット座標
	VECTOR targetPos_;

	//移動方向
	VECTOR moveDir_;
};

