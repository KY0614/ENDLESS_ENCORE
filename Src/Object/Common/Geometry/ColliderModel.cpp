#include "ColliderModel.h"

ColliderModel::ColliderModel(
	const TAG tag,
	const Transform* follow):
	ColliderBase(SHAPE::MODEL, tag, follow)
{
}

ColliderModel::~ColliderModel(void)
{
}

void ColliderModel::DrawDebug(int color)
{
}
