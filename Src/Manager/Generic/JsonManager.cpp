#include <DxLib.h>
#include "../../Application.h"
#include "JsonManager.h"

namespace 
{
	const std::string JSON_PLAYER = "Player";
	const std::string JSON_ENEMY = "Enemy";
}

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

void JsonManager::Init(void)
{
}

void JsonManager::Release(void)
{
	//読み込んだデータの解放
	jsonDataMap_.clear();
}

void JsonManager::Destroy(void)
{
	//インスタンスの解放
	Release();
	delete instance_;
}

nlohmann::json JsonManager::GetJsonData(JSON_DATA data)
{
	nlohmann::json jsonData = jsonDataMap_[data];
	return jsonData;
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

const VECTOR JsonManager::GetParseVector(const nlohmann::json& jsonData, const std::string& key)
{
	//配列のサイズ
	const int arraySize = 3;	
	//配列のフォーマットチェック
	if (!jsonData.contains(key) || !jsonData[key].is_array() || jsonData[key].size() < arraySize)
	{
		//存在しなかったりしたらデフォルト値を返す
		assert(L"%s のフォーマットが不正です。\n", key.c_str());
		return VGet(0.0f, 0.0f, 0.0f);
	}
	//配列の取得
	const auto& arr = jsonData[key];
	return VGet(
		arr[0].get<float>(),//X座標
		arr[1].get<float>(),//Y座標
		arr[2].get<float>()	//Z座標	
	);
}

void JsonManager::InitGame(void)
{
	static std::string PATH_JSON = Application::PATH_JSON;

	//プレイヤーデータの読み込み
	const std::string playerPath = "Player.json";
	jsonDataMap_.emplace(JSON_DATA::PLAYER, LoadData(PATH_JSON + playerPath, JSON_PLAYER));
	const std::string enemyPath = "Enemy.json";
	jsonDataMap_.emplace(JSON_DATA::ENEMY, LoadData(PATH_JSON + enemyPath, JSON_ENEMY));
}