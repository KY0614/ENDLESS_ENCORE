#include "../Transform.h"
#include "Box.h"

Box::Box(const Transform& parent) : 
	transformParent_(parent)
{
}

Box::Box(const Box& base, const Transform& parent) :
	parentPos_(base.parentPos_),
	transformParent_(parent)
{
}

Box::~Box(void)
{
}
// 描画 (ワイヤーフレーム)
void Box::Draw(void)
{
	Draw(COLOR, false);
}

// 描画 (色と塗りつぶしを指定)
void Box::Draw(int col, bool fill)
{
	// Bounding Box (AABB) の描画関数がないため、DxLibのDrawBox3Dを使用します。
	// DrawBox3Dは中心座標と各軸方向の長さを取るのではなく、Min/Max座標を必要とします。

	VECTOR center = GetCenter();

	// ワールド座標での8頂点を計算
	// 立方体の中心から、サイズ分だけローカル座標でオフセットした位置を回転させてワールド座標に変換します。

	// ローカル座標でのオフセット (size_ は半分の長さ)
	VECTOR offsets[8] = {
		{ size_.x, size_.y, size_.z },   // 奥右上0
		{ -size_.x, size_.y, size_.z },  // 奥左上1
		{ -size_.x, -size_.y, size_.z }, // 奥左下2
		{ size_.x, -size_.y, size_.z },  // 奥右下3
		{ size_.x, size_.y, -size_.z },  // 手前右上4
		{ -size_.x, size_.y, -size_.z }, // 手前左上5
		{ -size_.x, -size_.y, -size_.z },// 手間左下6
		{ size_.x, -size_.y, -size_.z }	 // 手前右下7
	};

	VECTOR corners[8];
	for (int i = 0; i < 8; ++i)
	{
		// localCenter_ にオフセットを加えた位置を回転させ、親の位置に加算
		VECTOR localPos = VAdd(localCenter_, offsets[i]);
		corners[i] = GetRotPos(localPos);
	}

	// DrawBox3Dを使って描画するには、AABBのMin/Maxが必要ですが、
	// 立方体が回転している場合はDrawBox3Dは使えません。
	// 代わりに、8頂点を使って12本の線を描画します。

	// ワイヤーフレーム描画 (12辺)
	// 底面
	DrawLine3D(corners[2], corners[3], col);
	DrawLine3D(corners[3], corners[7], col);
	DrawLine3D(corners[7], corners[6], col);
	DrawLine3D(corners[6], corners[2], col);

	// 天井
	DrawLine3D(corners[0], corners[1], col);
	DrawLine3D(corners[1], corners[5], col);
	DrawLine3D(corners[5], corners[4], col);
	DrawLine3D(corners[4], corners[0], col);

	// 側面
	DrawLine3D(corners[0], corners[3], col);
	DrawLine3D(corners[1], corners[2], col);
	DrawLine3D(corners[4], corners[7], col);
	DrawLine3D(corners[5], corners[6], col);
}

// ワールド座標での中心位置を取得
VECTOR Box::GetCenter(void) const
{
	return GetRotPos(localCenter_);
}

// 相対座標を回転させてワールド座標で取得する
VECTOR Box::GetRotPos(const VECTOR& localPos) const
{
	// Sphere/Capsule の実装と同じロジック
	VECTOR localRotPos = transformParent_.quaRot.PosAxis(localPos);
	return VAdd(transformParent_.pos, localRotPos);
}