#include "../Transform.h"
#include "ColliderCapsule.h"

ColliderCapsule::ColliderCapsule(
	const TAG tag,
	const Transform* follow,
	const VECTOR& localPosTop,
	const VECTOR& localPosDown,
	const float& radius):
	ColliderBase(SHAPE::CAPSULE, tag, follow),
	localPosTop_(localPosTop),
	localPosDown_(localPosDown),
	radius_(radius)
{
}

ColliderCapsule::~ColliderCapsule(void)
{
}

const VECTOR ColliderCapsule::GetPosCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();

	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}

void ColliderCapsule::DrawDebug(int color)
{
	const int divNum = 5;
	color += 0x0000ff;
	//ã‚Ì‹…‘Ì
	VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, divNum, color, color, false);

	//‰º‚Ì‹…‘Ì
	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, divNum, color, color, false);
	
	VECTOR dir;
	VECTOR s;
	VECTOR e;
	//‹…‘Ì‚ğŒq‚®ü(X+)
	dir = follow_->GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	//‹…‘Ì‚ğŒq‚®ü(X-)
	dir = follow_->GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	//‹…‘Ì‚ğŒq‚®ü(Z+)
	dir = follow_->GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	//‹…‘Ì‚ğŒq‚®ü(Z-)
	dir = follow_->GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	//ƒJƒvƒZƒ‹‚Ì’†S
	DrawSphere3D(GetPosCenter(), 5.0f, 10, color, color, true);
}
