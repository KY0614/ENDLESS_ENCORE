#include "../Transform.h"
#include "ColliderSphere.h"

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

VECTOR ColliderSphere::GetPos(void) const
{
	return GetRotPos(localPos_);
}

void ColliderSphere::DrawDebug(int color)
{
	//デバック表示は半径とポリゴン分割数を固定して描画
	DrawSphere3D(GetPos(), radius_, DIV_NUM, color, color, false);
}
