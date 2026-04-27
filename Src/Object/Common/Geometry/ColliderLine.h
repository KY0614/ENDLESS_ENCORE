#pragma once
#include "ColliderBase.h"
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
		TAG tag, const Transform* follow,
		const VECTOR& localPosStart, const VECTOR& localPosEnd);
	// デストラクタ
	~ColliderLine(void) override;

	/// <summary>
	/// ローカル開始座標の設定
	/// </summary>
	/// <param name="pos">ローカル開始座標</param>
	void SetLocalPosStart(const VECTOR& pos);

	/// <summary>
	/// ローカル終了座標の設定
	/// </summary>
	/// <param name="pos">ローカル終了座標</param>
	void SetLocalPosEnd(const VECTOR& pos);

	/// <summary>
	/// ローカル開始座標の取得
	/// </summary>
	/// <returns>ローカル開始座標</returns>
	const VECTOR& GetLocalPosStart(void) const;

	/// <summary>
	/// ローカル終了座標の取得
	/// </summary>
	/// <returns>ローカル終了座標</returns>
	const VECTOR& GetLocalPosEnd(void) const;

	/// <summary>
	/// ワールド開始座標の取得
	/// </summary>
	/// <returns>ワールド開始座標</returns>
	VECTOR GetPosStart(void) const;

	/// <summary>
	/// ワールド終了座標の取得
	/// </summary>
	/// <returns>ワールド終了座標</returns>
	VECTOR GetPosEnd(void) const;

protected:
	// デバッグ用描画
	void DrawDebug(int color) override;
private:
	// デバッグ表示の球体半径
	static constexpr float RADIUS = 5.0f;
	// デバッグ表示の球体ポリゴン分割数
	static constexpr int DIV_NUM = 6;
	// 線分の開始座標(ローカル)
	VECTOR localPosStart_;
	// 線分の終了座標(ローカル)
	VECTOR localPosEnd_;
};

