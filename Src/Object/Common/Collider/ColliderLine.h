#pragma once
#include "ColliderBase.h"
#include <DxLib.h>

class Transform;

class ColliderLine : public ColliderBase
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="tag">形状</param>
	/// <param name="follow">追従先</param>
	/// <param name="localPosStart">ローカル座標での開始位置</param>
	/// <param name="localPosEnd">ローカル座標での終了位置</param>
	ColliderLine(
		TAG tag,
		const Transform* follow,
		const VECTOR& localPosStart,
		const VECTOR& localPosEnd);
	// デストラクタ
	~ColliderLine(void) override;

	/// <summary>
	/// ローカル開始座標の設定
	/// </summary>
	/// <param name="pos">ローカル開始座標</param>
	void SetLocalPosStart(const VECTOR& pos) { localPosStart_ = pos; }

	/// <summary>
	/// ローカル終了座標の設定
	/// </summary>
	/// <param name="pos">ローカル終了座標</param>
	void SetLocalPosEnd(const VECTOR& pos) { localPosEnd_ = pos; }

	/// <summary>
	/// ローカル開始座標の取得
	/// </summary>
	/// <returns>ローカル開始座標</returns>
	const VECTOR& GetLocalPosStart(void) const { return localPosStart_; }

	/// <summary>
	/// ローカル終了座標の取得
	/// </summary>
	/// <returns>ローカル終了座標</returns>
	const VECTOR& GetLocalPosEnd(void) const { return localPosEnd_; }

	/// <summary>
	/// ワールド開始座標の取得
	/// </summary>
	/// <returns>ワールド開始座標</returns>
	const VECTOR GetPosStart(void) const { return GetRotPos(localPosStart_); }

	/// <summary>
	/// ワールド終了座標の取得
	/// </summary>
	/// <returns>ワールド終了座標</returns>
	const VECTOR GetPosEnd(void) const { return GetRotPos(localPosEnd_); }

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:
	// 線分の開始座標(ローカル)
	VECTOR localPosStart_;
	// 線分の終了座標(ローカル)
	VECTOR localPosEnd_;
};

