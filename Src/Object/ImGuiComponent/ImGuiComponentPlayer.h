#pragma once
#include "ImGuiComponentBase.h"

class ImGuiComponentPlayer : public ImGuiComponentBase
{
public:
	//コンストラクタ
	ImGuiComponentPlayer(void);
	//デストラクタ
	~ImGuiComponentPlayer(void)override;

	/// <summary>
	/// Jsonデータを保存する
	/// </summary>
	/// <param name="variable">保存する値</param>
	/// <param name="jsonDataKey">保存するデータのキー</param>
	void SaveJsonData(
		const float* variable,
		const char* jsonDataKey) override;
};

