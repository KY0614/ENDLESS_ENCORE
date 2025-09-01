#pragma once
#include "../StageObject.h"

class FollowingObject : public StageObject
{
public:

	FollowingObject(const std::string objId, const float width,
		const float height, const float depth, Player& player,
		StageObject& followObject);

	~FollowingObject(void) = default;

	void Init(VECTOR pos, float rotY = 0.0f, VECTOR scale = { 1.0f,1.0f,1.0f })override;

	void Update(void)override;

	StageObject& GetFollowedObj(void) const { return followObj_; }

private:

	StageObject& followObj_;
};

