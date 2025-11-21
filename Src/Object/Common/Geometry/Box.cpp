#include "../Transform.h"
#include "Box.h"

Box::Box(const Transform& parent) : 
	transformParent_(parent)
{
	localCenter_ = { 0.0f,0.0f,0.0f };
	size_ = { 0.0f,0.0f,0.0f };

	UpdateAxis();
}

Box::Box(const Box& base, const Transform& parent) :
	transformParent_(parent)
{
	localCenter_ = base.GetLocalCenter();
	size_ = base.GetSize();
	UpdateAxis();
}

Box::~Box(void)
{
}

void Box::Draw(void)
{
	VECTOR pos = GetCenter();
	DrawBox(COLOR, pos);
}

void Box::DrawBox(int col, VECTOR center)
{
	//回転行列を更新
	UpdateAxis();
	// ワールド座標での8頂点を計算
	// 立方体の中心から、サイズ分だけローカル座標でオフセットした位置を回転させてワールド座標に変換します。
	// OBBの半分のサイズ (obb_.vMaxを使用)
	VECTOR extents = obb_.vMax;

	MATRIX rotMat;
	rotMat = transformParent_.quaRot.ToMatrix();
	VECTOR outVertices[8];
	int idx = 0;
	for (int x = 0; x <= 1; ++x)
	{
		for (int y = 0; y <= 1; ++y)
		{
			for (int z = 0; z <= 1; ++z)
			{
				VECTOR local;
				local.x = (x == 0) ? obb_.vMin.x : obb_.vMax.x;
				local.y = (y == 0) ? obb_.vMin.y : obb_.vMax.y;
				local.z = (z == 0) ? obb_.vMin.z : obb_.vMax.z;

				VECTOR world = VTransform(local, rotMat);
				VECTOR pos = GetCenter();
				world = VAdd(world, pos);

				outVertices[idx++] = world;
			}
		}
	}
	// 12本のエッジのインデックス
	static const int edges[12][2] = {
		{0,1},{0,2},{0,4}, {1,3},{1,5},
		{2,3},{2,6}, {3,7},
		{4,5},{4,6}, {5,7},{6,7}
	};
	for (int i = 0; i < 12; ++i)
	{
		DrawLine3D(outVertices[edges[i][0]], outVertices[edges[i][1]], col);
	}
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

void Box::SetSize(const VECTOR& size)
{
	size_ = size;
	obb_.vMin = VGet(-size_.x, -size_.y, -size_.z);
	obb_.vMax = VGet(size_.x, size_.y, size_.z);
}

VECTOR Box::GetParetPos(void) const
{ 
	return transformParent_.pos; 
}

void Box::UpdateAxis(void)
{
	MATRIX rotMat;
	rotMat = transformParent_.quaRot.ToMatrix();

	obb_.axis[0] = VTransform(VGet(1, 0, 0), rotMat); // Right
	obb_.axis[1] = VTransform(VGet(0, 1, 0), rotMat); // Up
	obb_.axis[2] = VTransform(VGet(0, 0, 1), rotMat); // Forward
}
