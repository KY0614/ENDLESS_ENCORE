#include "../Transform.h"
#include "Capsule.h"

Capsule::Capsule(const Transform& parent) : transformParent_(parent)
{
	radius_ = 0.0f;
	localPosTop_ = { 0.0f, 0.0f, 0.0f };
	localPosDown_ = { 0.0f, 0.0f, 0.0f };
}

Capsule::Capsule(const Capsule& base, const Transform& parent) : transformParent_(parent)
{
	radius_ = base.GetRadius();
	localPosTop_ = base.GetLocalPosTop();
	localPosDown_ = base.GetLocalPosDown();
}

Capsule::~Capsule(void)
{
}

void Capsule::Draw(void)
{
	//Zバッファを有効にして描画(球体同士の前後関係を正しく描画するため)
	SetUseZBufferFlag(true);
	const int divNum = 5;
	//上の球体
	VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, divNum, COLOR, COLOR, false);

	//下の球体
	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, divNum, COLOR, COLOR, false);

	VECTOR dir;
	VECTOR s;
	VECTOR e;

	//球体を繋ぐ線(X+)
	dir = transformParent_.GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	//球体を繋ぐ線(X-)
	dir = transformParent_.GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	//球体を繋ぐ線(Z+)
	dir = transformParent_.GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	//球体を繋ぐ線(Z-)
	dir = transformParent_.GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	//カプセルの中心
	DrawSphere3D(GetCenter(), 5.0f, 10, COLOR, COLOR, true);
	SetUseZBufferFlag(false);	//戻す
}

VECTOR Capsule::GetLocalPosTop(void) const
{
	return localPosTop_;
}

VECTOR Capsule::GetLocalPosDown(void) const
{
	return localPosDown_;
}

void Capsule::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}

void Capsule::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}

VECTOR Capsule::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}

VECTOR Capsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

VECTOR Capsule::GetRotPos(const VECTOR& localPos) const
{
	VECTOR localRotPos = transformParent_.quaRot.PosAxis(localPos);
	return VAdd(transformParent_.pos, localRotPos);
}

float Capsule::GetRadius(void) const
{
	return radius_;
}

void Capsule::SetRadius(float radius)
{
	radius_ = radius;
}

float Capsule::GetHeight(void) const
{
	return localPosTop_.y;
}

VECTOR Capsule::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();

	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}
