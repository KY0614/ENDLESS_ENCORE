#pragma once
#include "../StageObject.h"

class GaugeUI;
class IconUI;

class IceDispenser : public StageObject
{
public:

	static constexpr float ICE_PRODUCES_TIME = 3.0f;

	IceDispenser(const std::string objId, const float width,
		const float height, const float depth, Player& player,
		std::vector<std::unique_ptr<StageObject>>& object);
	~IceDispenser(void) = default;

	void Init(VECTOR pos, float rotY = 0.0f, VECTOR scale = { 1.0f,1.0f,1.0f })override;

	void Draw(void) override;

	void Interact(const std::string& objId) override;

	void UpdateInActive(void)override;
	void UpdateActive(void)override;

private:

	//
	std::vector<std::unique_ptr<StageObject>>& objects_;

	std::unique_ptr<GaugeUI> gaugeUI_;
	//ÉAÉCÉRÉìÇÃUI
	std::unique_ptr<IconUI> iconUI_;
};

