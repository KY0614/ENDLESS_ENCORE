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

	/// <summary>
	/// jsonデータの階層を指定する(Jsonオブジェクトの名前を指定する)
	/// </summary>
	/// <param name="hierarchyKeys">指定するJsonオブジェクトの名前</param>
	void SetTargetHierarchy(
		const std::vector<const char*>& hierarchyKeys
	);

protected:

	/// <summary>
	/// Jsonデータを保存する
	/// </summary>
	/// <param name="variable">保存する値</param>
	/// <param name="jsonDataKey">保存するJsonデータのキー</param>
	virtual void SaveJsonData(
		const float* variable,
		const char* jsonDataKey) = 0;

	//階層のキー（Jsonオブジェクトの名前）
	std::vector<const char*> hierarchyKeys_;
};

