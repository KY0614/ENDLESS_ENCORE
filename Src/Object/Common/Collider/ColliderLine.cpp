#include "ColliderLine.h"
#include "ColliderCapsule.h"

namespace
{
	//デバッグ表示の球体半径
	const float RADIUS = 5.0f;
	//デバッグ表示の球体ポリゴン分割数
	const int DIV_NUM = 6;
}

ColliderLine::ColliderLine(
	TAG tag,
	const Transform* follow,
	const VECTOR& localPosStart,
	const VECTOR& localPosEnd):
	ColliderBase(SHAPE::LINE, tag, follow),
	localPosStart_(localPosStart),
	localPosEnd_(localPosEnd)
{
}

ColliderLine::~ColliderLine(void)
{
}

void ColliderLine::DrawDebug(int color)
{
	const VECTOR s = GetPosStart();
	const VECTOR e = GetPosEnd();
	// 線分を描画
	DrawLine3D(s, e, color);
	// 始点・終点を球体で補助表示
	DrawSphere3D(s, RADIUS, DIV_NUM, color, color, true);
	DrawSphere3D(e, RADIUS, DIV_NUM, color, color, true);
}