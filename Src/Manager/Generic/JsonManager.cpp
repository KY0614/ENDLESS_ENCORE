#include <cassert>
#include <DxLib.h>
#include "../../Application.h"
#include "JsonManager.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace 
{
	const std::string JSON_PLAYER = "Player";
	const std::string JSON_ENEMY = "Enemy";
	const std::string JSON_FIGHTER = "Fighter";
	const std::string JSON_MAGE = "Mage";
	const std::string JSON_STAGE = "Stage";

	const int JSON_INDENT_NUM = 4;	//JSONのインデントスペース数
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
	jsonDataMap_.clear();
	delete instance_;
}

const nlohmann::json& JsonManager::GetJsonData(const JSON_DATA dataType) const
{
	return jsonDataMap_.at(dataType).jsonData_;
}

const nlohmann::json& JsonManager::GetJsonData(
	const JSON_DATA dataType,
	const std::string data)const
{
	return jsonDataMap_.at(dataType).jsonData_.at(data);
}

const nlohmann::json& JsonManager::GetJsonDataType(const JSON_DATA dataType) const
{
	return jsonDataMap_.at(dataType).jsonData_;
}

const std::string& JsonManager::GetJsonFileName(const JSON_DATA dataType) const
{
	return jsonDataMap_.at(dataType).fileName_;
}

const VECTOR JsonManager::GetParseVector(
	const nlohmann::json& jsonData,
	const std::string& key)
{
	//配列のサイズ
	const int arraySize = 3;	
	//配列のフォーマットチェック
	if (!jsonData.contains(key) || !jsonData[key].is_array() || jsonData[key].size() < arraySize)
	{
		//存在しなかったりしたらデフォルト値を返す
		return VGet(0.0f, 0.0f, 0.0f);
	}
	//配列の取得
	const json& arr = jsonData[key];
	return VGet(
		arr[0].get<float>(),//X座標
		arr[1].get<float>(),//Y座標
		arr[2].get<float>()	//Z座標	
	);
}

void JsonManager::InitTitle(void)
{
	static std::string PATH_JSON = Application::PATH_JSON;

	//ステージのデータ読み込み
	const std::string stagePath = "Stage.json";
	LoadJsonData(JSON_DATA::STAGE, PATH_JSON + stagePath, JSON_STAGE);
	//jsonDataMap_.emplace(JSON_DATA::STAGE, LoadJsonData(PATH_JSON + stagePath, JSON_STAGE));
}

void JsonManager::InitGame(void)
{
	//JSONデータが入っているフォルダのパス
	static std::string PATH_JSON = Application::PATH_JSON;

	//プレイヤーのデータ読み込み
	const std::string playerPath = "Player.json";
	LoadJsonData(JSON_DATA::PLAYER, PATH_JSON + playerPath, JSON_PLAYER);
	//敵のデータ読み込み
	const std::string enemyPath = "Enemy.json";
	LoadJsonData(JSON_DATA::ENEMY, PATH_JSON + enemyPath, JSON_ENEMY);
	//近接型のデータ読み込み
	const std::string summonEnemyPath = "SummonEnemy.json";
	LoadJsonData(JSON_DATA::FIGHTER_GHOST, PATH_JSON + summonEnemyPath, JSON_FIGHTER);
	//遠距離型のデータ読み込み
	LoadJsonData(JSON_DATA::MAGE_GHOST, PATH_JSON + summonEnemyPath, JSON_MAGE);
	//ステージのデータ読み込み
	const std::string stagePath = "Stage.json";
	LoadJsonData(JSON_DATA::STAGE, PATH_JSON + stagePath, JSON_STAGE);
}

void JsonManager::LoadJsonData(
	const JSON_DATA& dataType,
	const std::string& filepath,
	const std::string& dataName)
{
	jsonDataMap_[dataType].fileName_ = filepath;
	//jsonデータの読み込み
	std::ifstream ifs(filepath);
	if (!ifs)return;
	//ファイルストリームからjsonオブジェクトに変換
	nlohmann::json data = nlohmann::json::parse(ifs);
	if (!data.contains(dataName))return;
	jsonDataMap_[dataType].jsonData_ = data;
}

void JsonManager::UpdateJsonData(const JSON_DATA jsonDataType, const nlohmann::json jsonData)
{
	jsonDataMap_[jsonDataType].jsonData_ = jsonData;
}