#include "../Manager/Generic/JsonManager.h"
#include "ImGuiComponentCharacter.h"

namespace
{
	//Jsonデータのキー
	//プレイヤーオブジェクトのキー
	const char* KEY_PLAYER = "Player";
	const char* KEY_ENEMY = "Enemy";
}

ImGuiComponentCharacter::ImGuiComponentCharacter(CHARACTER_TYPE characterType):
	characterType_(characterType)
{
	switch (characterType_)
	{;
	case ImGuiComponentCharacter::CHARACTER_TYPE::PLAYER:
		jsonDataType_ = JsonManager::JSON_DATA::PLAYER;
		break;
	case ImGuiComponentCharacter::CHARACTER_TYPE::ENEMY:
		jsonDataType_ = JsonManager::JSON_DATA::ENEMY;
		break;
	default:
		break;
	}
}

ImGuiComponentCharacter::~ImGuiComponentCharacter(void)
{
}

void ImGuiComponentCharacter::SaveJsonData(
	const float* variable,
	const char* jsonDataKey)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//保存（データを上書き）
	jsonM.OverWriteJsonDatas(
		jsonDataType_,
		*variable,
		hierarchyKeys_.front(),
		hierarchyKeys_.back(),
		jsonDataKey);
}
