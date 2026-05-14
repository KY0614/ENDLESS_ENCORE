#pragma once
#include "../Libs/nlohmann/json.hpp"

class ImGuiComponentBase
{
public:
	//コンストラクタ
	ImGuiComponentBase(void);

	//デストラクタ
	virtual ~ImGuiComponentBase(void);

	/// <summary>
	/// ImGuiのfloat型のスライダー
	/// </summary>
	/// <param name="label">スライダーのラベル(名前)</param>
	/// <param name="variable">変数</param>
	/// <param name="min">下限値</param>
	/// <param name="max">上限値</param>
	/// <param name="jsonData">上限値</param>
	/// <param name="jsonKey">上限値</param>
	
	void SliderFloatWithSave(
		const char* label,
		float* variable,
		const float min,
		const float max,
		const nlohmann::json& jsonData,
		const char* jsonKey);

protected:


};

