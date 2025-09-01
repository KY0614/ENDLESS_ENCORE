#pragma once
#include "../StageObject.h"

class GaugeUI;
class IconUI;

class RackObject : public StageObject
{
public:

	static constexpr float SWEETS_UI_OFFSET_Y = 110.0f;
	static constexpr float CUP_UI_OFFSET_Y = 30.0f;

	RackObject(const std::string objId, const float width,
		const float height, const float depth, Player& player);

	~RackObject(void) = default;

	void PickUp(std::string rackName,std::vector<std::unique_ptr<StageObject>>& object) override;

	void AddStock(int addStockNum) override;

	void Init(VECTOR pos, float rotY = 0.0f, VECTOR scale = { 1.0f,1.0f,1.0f }) override;

	void Update(void) override;

	void Draw(void) override;

private:

	//オブジェクトの残り個数
	int sweetsStockCnt_;	//スイーツ用
	int cupsStockCnt_;		//カップ用

	float addInterval_;	//追加される間隔

	//ゲージのUI
	std::unique_ptr<GaugeUI> gaugeUI_;

	//アイコンUI
	std::unique_ptr<IconUI> iconUI_;

	Transform sweetsOfRack_[SWEETS_STOCK_MAX];
	Transform cupesOfRack_[CUP_STOCK_MAX];
};

