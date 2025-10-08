#include "ShotBase.h"

ShotBase::ShotBase(void)
{
}

ShotBase::~ShotBase(void)
{
}

const Transform& ShotBase::GetTransform(void) const
{
	return transform_;
}