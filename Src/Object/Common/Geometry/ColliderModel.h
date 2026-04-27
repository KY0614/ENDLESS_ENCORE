#pragma once
#include "ColliderBase.h"

class ColliderModel : public ColliderBase
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="tag">衝突種別</param>
	/// <param name="follow">追従先のTransform</param>
	ColliderModel(
		const TAG tag,
		const Transform* follow);
	//デストラクタ
	~ColliderModel(void);

	/// <summary>
	/// 座標の取得
	/// </summary>
	/// <returns>ワールド座標</returns>
	VECTOR GetPos(void) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;
};

