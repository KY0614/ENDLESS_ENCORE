#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <DxLib.h>
#include "../../Libs/nlohmann/json.hpp"

class JsonManager
{
public:
	//JSONキー名を定義
	static constexpr const char* KEY_TRANSFORM = "Transform";
	static constexpr const char* KEY_POSITION = "position";
	static constexpr const char* KEY_POSITION_X = "positionX";
	static constexpr const char* KEY_POSITION_Y = "positionY";
	static constexpr const char* KEY_POSITION_Z = "positionZ";
	static constexpr const char* KEY_SCALE = "scale";
	static constexpr const char* KEY_ROT_Y = "localRotY";
	static constexpr const char* KEY_ANIMATION = "Animation";
	static constexpr const char* KEY_ANIM_SPEED = "defaultSpeed";
	static constexpr const char* KEY_PARAMETER = "Parameter";
	static constexpr const char* KEY_HP = "hp";				//体力
	static constexpr const char* KEY_MAX_HP = "maxHp";		//最大体力

	static constexpr int JSON_INDENT_NUM = 4;	//JSONのインデントスペース数

	struct JsonFileInfo
	{
		std::string fileName_;		//Jsonファイル名
		nlohmann::json jsonData_;	//Jsonデータ
	};	

	//jsonデータの種類
	enum class JSON_DATA
	{
		PLAYER,		//プレイヤー
		ENEMY,		//敵
		FIGHTER_GHOST,	//近接型の敵
		MAGE_GHOST,		//遠距離型の敵
		STAGE,		//ステージ
	};

	//インスタンスの生成
	static void CreateInstance(void);

	//静的インスタンスの取得
	static JsonManager& GetInstance(void);

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(void);

	/// <summary>
	/// 解放（シーン切替時に一旦解放）
	/// </summary>
	void Release(void);

	/// <summary>
	/// リソースの完全破棄
	/// </summary>
	void Destroy(void);
		
	/// <summary>
	/// Jsonデータを取得
	/// </summary>
	/// <param name="dataType">データの種類</param>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json& GetJsonData(
		const JSON_DATA dataType)const;
		
		
	/// <summary>
	/// Jsonデータを取得
	/// </summary>
	/// <param name="dataType">データの種類</param>
	/// <param name="data">Jsonオブジェクト</param>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json& GetJsonData(
		const JSON_DATA dataType,
		const std::string data)const;
		
	/// <summary>
	/// Jsonデータを取得
	/// </summary>
	/// <param name="dataType">データの種類</param>
	/// <param name="data">Jsonオブジェクト</param>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json& GetJsonDataType(
		const JSON_DATA dataType)const;

	const std::string& GetJsonFileName(const JSON_DATA dataType)const;

	/// <summary>
	/// 既存のJsonデータを上書きして保存する
	/// </summary>
	/// <param name="fileName">保存するファイル名</param>
	/// <param name="objectKeys">指定するJsonオブジェクト名({}で囲われているもの）</param>
	/// <param name="jsonData">上書きするJsonデータ</param>
	/// <param name="value">保存したいデータ</param>
	template <typename Value,typename... ObjectKeys>
	void OverWriteJsonDatas(
		const JSON_DATA jsonDataType,
		const Value value,
		const ObjectKeys... objectKey);

	/// <summary>
	/// 既存のJsonデータを上書きして保存する
	/// </summary>
	/// <param name="fileName">保存するファイル名</param>
	/// <param name="jsonObjectA">指定するJsonオブジェクト名({}で囲われているもの）</param>
	/// <param name="jsonObjectB">指定するJsonオブジェクト名(jsonObjectAオブジェクト内で{}で囲われているもの）</param>
	/// <param name="jsonData">上書きするJsonデータ</param>
	/// <param name="value">保存したいデータ</param>
	template <typename Value>
	void OverWriteJsonData(const std::string fileName,
		const std::string jsonObjectA,
		const std::string jsonObjectB,
		const std::string jsonData,
		const Value value)
	{
		std::ifstream ifs(fileName);
		if (!ifs)
		{
			assert(0 && "ファイルが見つかりませんでした");
			return;
		}
		//ファイルストリームからjsonオブジェクトに変換
		nlohmann::json JsonData = nlohmann::json::parse(ifs);
		//上書き
		JsonData[jsonObjectA][jsonObjectB][jsonData] = value;
		//上書きしたjsonオブジェクトをファイルに保存
		std::ofstream writing_file;
		writing_file.open(fileName, std::ios::out);
		writing_file << JsonData.dump(JSON_INDENT_NUM) << std::endl;
	}

	/// <summary>
	/// JSONデータからVECTOR型へ変換して取得
	/// </summary>
	/// <param name="jsonData">変換するJSONデータ</param>
	/// <param name="key">取得するキーの文字列</param>
	/// <returns>取得したVECTOR型データ</returns>
	static const VECTOR GetParseVector(
		const nlohmann::json& jsonData,
		const std::string& key);

	//シーンごとにデータを読み込むことにする
	
	/// <summary>
	/// タイトル用のデータを初期化する
	/// </summary>
	/// <param name=""></param>
	void InitTitle(void);
	
	/// <summary>
	/// ゲームシーン用のデータを初期化する
	/// </summary>
	/// <param name=""></param>
	void InitGame(void);

private:

	//静的インスタンス
	static JsonManager* instance_;

	// リソース管理の対象
	std::unordered_map<JSON_DATA, JsonFileInfo> jsonDataMap_;

	/// <summary>
	/// 指定されたオブジェクト名に基づいてデータを読み込む
	/// </summary>
	/// <param name="objName">データを読み込む対象のオブジェクト名</param>
	/// <returns>オブジェクト名と対応するパラメータ</returns>
	void LoadJsonData(
		const JSON_DATA& dataType,
		const std::string& filepath,
		const std::string& dataName);

	/// <summary>
	/// JSONオブジェクトに値を代入する（再帰の終わり用）
	/// </summary>
	/// <typeparam name="Value"></typeparam>
	/// <param name="j"></param>
	/// <param name="value"></param>
	template <typename Value>
	void SetJsonValue(nlohmann::json& j, const Value& value);

	// 再帰：キーを一つずつ進めていく

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="Value"></typeparam>
	/// <typeparam name="Key"></typeparam>
	/// <typeparam name="...Rest"></typeparam>
	/// <param name="jsonData"></param>
	/// <param name="value"></param>
	/// <param name="key"></param>
	/// <param name="...rest"></param>
	template <typename Value, typename Key, typename... Rest>
	void SetJsonValue(
		nlohmann::json& jsonData,
		const Value& value,
		const Key& key, Rest... rest);

	/// <summary>
	/// 登録しているJsonデータを上書きして保存する
	/// </summary>
	/// <param name="jsonDataType">JSON_DATAの種類</param>
	/// <param name="jsonData">jsonデータ</param>
	void UpdateJsonData(const JSON_DATA jsonDataType, const nlohmann::json jsonData);
};

// 終端：キーがなくなった時に値を代入する
template <typename Value>
void JsonManager::SetJsonValue(nlohmann::json& j, const Value& value)
{
	j = value;
}

template<typename Value, typename Key, typename ...Rest>
inline void JsonManager::SetJsonValue(
	nlohmann::json& jsonData,
	const Value& value,
	const Key& key, Rest ...rest)
{
	SetJsonValue(jsonData[key], value, rest...);
}

template<typename Value, typename ...ObjectKeys>
void JsonManager::OverWriteJsonDatas(
	const JSON_DATA jsonDataType,
	const Value value,
	const ObjectKeys... objectKey)
{
	std::ifstream ifs(GetJsonFileName(jsonDataType).c_str());
	nlohmann::json root = nlohmann::json::parse(ifs);
	if (!ifs)
	{
		assert(0 && "ファイルが見つかりませんでした");
		return;
	}
	SetJsonValue(root, value, objectKey...);

	std::ofstream ofs(GetJsonFileName(jsonDataType).c_str());
	ofs << root.dump(JSON_INDENT_NUM) << std::endl;
}
