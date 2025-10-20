#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include "../../Libs/nlohmann/json.hpp"

class JsonManager
{
public:
	//JSONキー名を定義
	static constexpr const char* KEY_PLAYER = "Player";
	static constexpr const char* KEY_TRANSFORM = "Transform";
	static constexpr const char* KEY_POSITION = "position";
	static constexpr const char* KEY_SCALE = "scale";
	static constexpr const char* KEY_ROT_Y = "localRotY";
	static constexpr const char* KEY_ANIMATION = "Animation";

	//jsonデータの種類
	enum class JSON_DATA
	{
		PLAYER,		//プレイヤー
		ENEMY,
	};

	//インスタンスの生成
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static JsonManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	nlohmann::json GetJsonData(JSON_DATA data);

	/// <summary>
	/// 指定されたオブジェクト名に基づいてデータを読み込む
	/// </summary>
	/// <param name="objName">データを読み込む対象のオブジェクト名</param>
	/// <returns>オブジェクト名と対応するパラメータ</returns>
	nlohmann::json LoadData(const std::string& fileName, const std::string& dataName);

	static const VECTOR GetParseVector(const nlohmann::json& jsonData, const std::string& key);

	//シーンごとにデータを読み込むことにする
	void InitTitle(void);
	void InitMovie(void);
	void InitSelect(void);
	void InitTutorial(void);
	void InitGame(void);
	void InitPause(void);
	void InitResult(void);

private:

	//静的インスタンス
	static JsonManager* instance_;

	// リソース管理の対象
	std::unordered_map<JSON_DATA,nlohmann::json> jsonDataMap_;

};