#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include "../../Libs/nlohmann/json.hpp"

class JsonManager
{
public:
	//JSONキー名を定義
	static constexpr const char* KEY_TRANSFORM = "Transform";
	static constexpr const char* KEY_POSITION = "position";
	static constexpr const char* KEY_SCALE = "scale";
	static constexpr const char* KEY_ROT_Y = "localRotY";
	static constexpr const char* KEY_ANIMATION = "Animation";
	static constexpr const char* KEY_ANIM_SPEED = "defaultSpeed";
	static constexpr const char* KEY_PARAMETER = "Parameter";
	static constexpr const char* KEY_HP = "hp";
	static constexpr const char* KEY_MAX_HP = "maxHp";

	//jsonデータの種類
	enum class JSON_DATA
	{
		PLAYER,		//プレイヤー
		ENEMY,		//敵
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
	/// <param name="data">Jsonオブジェクト</param>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json& GetJsonData(
		const JSON_DATA dataType,
		const std::string data)const;

	/// <summary>
	/// 指定されたオブジェクト名に基づいてデータを読み込む
	/// </summary>
	/// <param name="objName">データを読み込む対象のオブジェクト名</param>
	/// <returns>オブジェクト名と対応するパラメータ</returns>
	nlohmann::json LoadData(const std::string& fileName, const std::string& dataName);

	/// <summary>
	/// JSONデータからVECTOR型へ変換して取得
	/// </summary>
	/// <param name="jsonData">変換するJSONデータ</param>
	/// <param name="key">取得するキーの文字列</param>
	/// <returns>取得したVECTOR型データ</returns>
	static const VECTOR GetParseVector(const nlohmann::json& jsonData, const std::string& key);

	//シーンごとにデータを読み込むことにする
	
	/// <summary>
	/// ゲームシーン用のデータを初期化する
	/// </summary>
	/// <param name=""></param>
	void InitGame(void);

private:

	//静的インスタンス
	static JsonManager* instance_;

	// リソース管理の対象
	std::unordered_map<JSON_DATA,nlohmann::json> jsonDataMap_;

};