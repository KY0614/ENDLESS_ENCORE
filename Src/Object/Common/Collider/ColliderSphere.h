#pragma once
#include "ColliderBase.h"

class ColliderSphere :  public ColliderBase
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="tag">衝突種別</param>
	/// <param name="follow">追従先のTransform</param>
	/// <param name="pos">ローカル座標</param>
	/// <param name="radius">半径</param>
	ColliderSphere(
		const TAG tag,
		const Transform* follow,
		const VECTOR& localPos,
		const float& radius);
	//デストラクタ
	~ColliderSphere(void);

	/// <summary>
	/// ローカル座標を取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	VECTOR GetLocalPos(void) const { return localPos_; }

	/// <summary>
	/// ローカル座標を設定
	/// </summary>
	/// <param name="localPos">追従先からの相対座標</param>
	void SetLocalPos(const VECTOR& localPos) { localPos_ = localPos; }

	/// <summary>
	/// 座標の取得
	/// </summary>
	/// <returns>ワールド座標</returns>
	VECTOR GetPos(void) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:
	// デバッグ表示の球体半径
	static constexpr float RADIUS = 5.0f;
	// デバッグ表示の球体ポリゴン分割数
	static constexpr int DIV_NUM = 6;

	// 線分の開始座標(ローカル)
	VECTOR localPos_;

	//半径
	float radius_;
};

