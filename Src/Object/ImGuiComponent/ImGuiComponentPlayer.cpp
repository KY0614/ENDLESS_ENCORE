#include "../Manager/Generic/JsonManager.h"
#include "ImGuiComponentPlayer.h"

namespace
{
	//Jsonデータのキー
	//プレイヤーオブジェクトのキー
	const char* KEY_PLAYER = "Player";
}

ImGuiComponentPlayer::ImGuiComponentPlayer(void)
{
}

ImGuiComponentPlayer::~ImGuiComponentPlayer(void)
{
}

void ImGuiComponentPlayer::SaveJsonData(
	const float* variable,
	const char* jsonDataKey)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//保存（データを上書き）
	jsonM.OverWriteJsonDatas(
		JsonManager::JSON_DATA::PLAYER,
		*variable,
		hierarchyKeys_.front(),
		hierarchyKeys_.back(),
		jsonDataKey);
}
