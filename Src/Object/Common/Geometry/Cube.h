#pragma once

#include "GeometryBase.h"

class Cube : public	GeometryBase
{
public:

	//バウンディングボックス
	struct BoundingBox
	{
		VECTOR vMin;
		VECTOR vMax;
		VECTOR axis[3];
	};

	//デバッグ時の簡易カプセル表示の色
	static constexpr int COLOR = 0xffffff;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="pos">追従する親の座標</param>
	/// <param name="rot">追従する親の回転</param>
	/// <param name="min">親から見た、箱の最小地点</param>
	/// <param name="max">親から見た、箱の最大地点</param>
	Cube(const VECTOR& pos,
		const Quaternion& rot,
		const VECTOR min,
		const VECTOR max);

	/// <summary>
	/// コピーコンストラクタ
	/// </summary>
	/// <param name="_copyBase">コピー元</param>
	/// <param name="_pos">追従する親の座標</param>
	/// <param name="_rot">追従する親の回転</param>
	Cube(const Cube& copyBase,
		const VECTOR& pos,
		const Quaternion& rot);

	//デストラクタ
	~Cube(void);


	//各種当たり判定
	const bool IsHit(GeometryBase& geometry)override;
	const bool IsHit(Model& model)override;
	const bool IsHit(Cube& cube)override;
	const bool IsHit(Sphere& sphere)override;
	const bool IsHit(Capsule& capsule)override;
	const bool IsHit(Line& line)override;

	void Draw(void);

	//回転バウンティボックスの取得
	inline const BoundingBox& GetObb(void)const { return boudingBox_; }

	//箱の最小地点の取得
	inline const VECTOR GetVecMin(void)const { return boudingBox_.vMin; }

	//箱の最大地点の取得
	inline const VECTOR GetVecMax(void)const { return boudingBox_.vMax; }

	//回転バウンティボックスの設定
	inline void SetObb(const BoundingBox& _obb) { boudingBox_ = _obb; }

	//箱の最小地点の設定
	inline void SetVecMin(const VECTOR& _min) { boudingBox_.vMin = _min; }

	//箱の最大地点の設定
	inline void SetVecMax(const VECTOR& _max) { boudingBox_.vMax = _max; }

	//サイズの半分の設定
	inline void SetHalfSize(const VECTOR& _halfSize);

private:
	BoundingBox boudingBox_;

	//箱の回転情報の取得
	inline const VECTOR GetAxis(const int _num)const { return boudingBox_.axis[_num]; }

	// クォータニオンから回転軸を計算
	void UpdateObbAxis(void);

	// 各頂点の計算（ワールド座標）
	void CalculateVertices(VECTOR outVertices[8]) const;

	//線分とAABBの最短距離の二乗計算
	float ClosestSegmentAABB(const VECTOR& segA, const VECTOR& segB, const VECTOR& aabbMin, const VECTOR& aabbMax);
};

