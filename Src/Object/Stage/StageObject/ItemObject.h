#pragma once
#include "../StageObject.h"

class ItemObject : public StageObject
{
public:

	ItemObject(const std::string objId, const float width,
		const float height, const float depth, Player& player);

	~ItemObject(void) = default;

	void ItemCarry(void) override;

	void ItemPlaced(VECTOR pos)override;

	void PouredIce(void);

	bool IsIce(void)const { return isIced_; }

	void Draw(void) override;

private:

	bool isIced_;

	void UpdatePlaced(void)override;
	void UpdateHold(void)override;
};

