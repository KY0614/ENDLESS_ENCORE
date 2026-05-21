#pragma once
#include <string>
#include "ImGuiComponentBase.h"

class ImGuiComponentCharacter : public ImGuiComponentBase
{
public:
	enum class CHARACTER_TYPE
	{
		NONE,
		PLAYER,
		ENEMY,
	};

	//コンストラクタ
	ImGuiComponentCharacter(CHARACTER_TYPE characterType);
	//デストラクタ
	~ImGuiComponentCharacter(void)override;

	/// <summary>
	/// Jsonデータを保存する
	/// </summary>
	/// <param name="variable">保存する値</param>
	/// <param name="jsonDataKey">保存するデータのキー</param>
	void SaveJsonData(
		const float* variable,
		const char* jsonDataKey) override;

private:
	//キャラクターの種類
	CHARACTER_TYPE characterType_;	

	//保存するJsonデータの種類
	JsonManager::JSON_DATA jsonDataType_;
};

