#include "../../../Manager/Generic/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Common/Sphere.h"
#include "FollowingObject.h"

FollowingObject::FollowingObject(const std::string objId,
	const float width, const float height,
	const float depth, Player& player,
	StageObject& followObject) : 
	StageObject(objId, width, height, depth, player), followObj_(followObject)
{
}

void FollowingObject::Init(VECTOR pos, float rotY, VECTOR scale)
{
	StageObject::Init(pos, rotY, scale);

	//テーブルとオブジェクトの判定の邪魔にならないように
	rad_ = 0.0f;
	sphere_->SetLocalPos({0.0f,30.0f,0.0f});
	sphere_->SetRadius(rad_);
}

void FollowingObject::Update(void)
{
	transform_.pos = followObj_.GetTransform().pos;

	transform_.Update();
}
