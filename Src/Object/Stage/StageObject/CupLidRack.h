#pragma once
#include "../StageObject.h"

class GaugeUI;

class CupLidRack : public StageObject
{
public:
	static constexpr float LID_PRODUCES_TIME = 3.0f;

	CupLidRack(const std::string objId, const float width,
		const float height, const float depth, Player& player,
		std::vector<std::unique_ptr<StageObject>>& object);

	~CupLidRack(void) = default;

	void Interact(const std::string& objId) override;

	void Init(VECTOR pos, float rotY = 0.0f, VECTOR scale = { 1.0f,1.0f,1.0f })override;
	void Update(void)override;
	void Draw(void)override;

private:

	std::vector<std::unique_ptr<StageObject>>& objects_;

	//ÉQÅ[ÉWÇÃUI
	std::unique_ptr<GaugeUI> gaugeUI_;
};

