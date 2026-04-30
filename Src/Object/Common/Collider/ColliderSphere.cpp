#include "../Transform.h"
#include "ColliderSphere.h"

namespace
{
	//デバッグ表示の球体半径
	const float RADIUS = 5.0f;
	//デバッグ表示の球体ポリゴン分割数
	const int DIV_NUM = 6;
}

ColliderSphere::ColliderSphere(
	const TAG tag,
	const Transform* follow,
	const VECTOR& localPos,
	const float& radius):
	ColliderBase(SHAPE::SPHERE, tag, follow),
	localPos_(localPos),
	radius_(radius)
{
}

ColliderSphere::~ColliderSphere(void)
{
}

const VECTOR& ColliderSphere::GetPos(void) const
{
	return GetRotPos(localPos_);
}

void ColliderSphere::DrawDebug(int color)
{
	//デバック表示は半径とポリゴン分割数を固定して描画
	DrawSphere3D(GetPos(), radius_, DIV_NUM, color, color, false);
}
