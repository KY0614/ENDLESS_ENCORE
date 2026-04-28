#pragma once
#include "ColliderBase.h"

class Transform;

class ColliderCapsule : public ColliderBase
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="tag">衝突種別</param>
	/// <param name="follow">追従先のTransform</param>
	/// <param name="localPosTop">ローカル座標(上側)</param>
	/// <param name="localPosDown">ローカル座標(下側)</param>
	/// <param name="radius">半径</param>
	ColliderCapsule(
		const TAG tag,
		const Transform* follow,
		const VECTOR& localPosTop,
		const VECTOR& localPosDown,
		const float& radius);
	//デストラクタ
	~ColliderCapsule(void);

	/// <summary>
	/// ローカル開始座標の設定
	/// </summary>
	/// <param name="pos">ローカル開始座標</param>
	void SetLocalPosTop(const VECTOR& pos) { localPosTop_ = pos; }

	/// <summary>
	/// ローカル終了座標の設定
	/// </summary>
	/// <param name="pos">ローカル終了座標</param>
	void SetLocalPosDown(const VECTOR& pos) { localPosDown_ = pos; }

	/// <summary>
	/// 半径を設定
	/// </summary>
	/// <param name="radius">半径</param>
	void SetRadius(const float& radius) { radius_ = radius; }

	/// <summary>
	/// 上側のローカル座標の取得
	/// </summary>
	/// <returns>ローカル開始座標</returns>
	const VECTOR& GetLocalPosTop(void) const { return localPosTop_; }

	/// <summary>
	/// 下側のローカル座標の取得
	/// </summary>
	/// <returns>ローカル終了座標</returns>
	const VECTOR& GetLocalPosDown(void) const { return localPosDown_; }

	/// <summary>
	/// 半径を取得
	/// </summary>
	/// <returns>カプセルの半径</returns>
	const float& GetRadius(void) const { return radius_; }

	/// <summary>
	/// 上側のワールド座標の取得
	/// </summary>
	/// <returns>ワールド開始座標</returns>
	const VECTOR GetPosTop(void) const { return GetRotPos(localPosTop_); }

	/// <summary>
	/// 下側のワールド座標の取得
	/// </summary>
	/// <returns>ワールド終了座標</returns>
	const VECTOR GetPosDown(void) const { return GetRotPos(localPosDown_); }

	/// <summary>
	/// 高さを取得する
	/// </summary>
	/// <returns>カプセルの高さ</returns>
	const float& GetHeight(void) const { return localPosTop_.y; }

	/// <summary>
	/// カプセルの中心座標の取得
	/// </summary>
	/// <returns>カプセルの中心座標</returns>
	const VECTOR GetPosCenter(void) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:

	//親Transformからの相対位置(上側)
	VECTOR localPosTop_;

	//親Transformからの相対位置(下側)
	VECTOR localPosDown_;

	//半径
	float radius_;
};