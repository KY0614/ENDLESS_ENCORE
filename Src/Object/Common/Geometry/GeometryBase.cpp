#include "GeometryBase.h"

GeometryBase::~GeometryBase(void)
{
}

GeometryBase::GeometryBase(
	const VECTOR& pos,
	const Quaternion& rot) : 
	parentPos_(pos),
	parentQuaRot_(rot)
{
}

const VECTOR GeometryBase::GetRotPos(const VECTOR& localPos) const
{
	VECTOR localRotPos = parentQuaRot_.PosAxis(localPos);
	return VAdd(parentPos_, localRotPos);
}

void GeometryBase::HitAfter(void)
{
}

