#include "../../Libs/nlohmann/json.hpp"
#include "JsonManager.h"

JsonManager* JsonManager::instance_ = nullptr;

void JsonManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new JsonManager();
	}
	instance_->Init();
}

JsonManager& JsonManager::GetInstance(void)
{
	return *instance_;
}

nlohmann::json JsonManager::LoadData(const std::string& fileName, const std::string& dataName)
{
	std::ifstream ifs(fileName);
	if (!ifs)return nlohmann::json();

	//ファイルストリームからjsonオブジェクトに変換
	nlohmann::json data = nlohmann::json::parse(ifs);
	if (!data.contains(dataName))return{};
	return data;
}

void JsonManager::InitGame(void)
{
	//各種データの読み込み
	jsonDataMap_.emplace(JSON_DATA::PLAYER, LoadData("Data/Paramate/Player.json", "Player"));
}