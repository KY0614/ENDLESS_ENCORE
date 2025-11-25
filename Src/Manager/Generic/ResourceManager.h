#pragma once
#include <memory>
#include <map>
#include <string>
#include "Resource.h"

class ResourceManager
{

public:
	static constexpr int NUMBER_NUM_X = 5;
	static constexpr int NUMBER_NUM_Y = 2;
	static constexpr int NUMBER_SIZE_X = 71;
	static constexpr int NUMBER_SIZE_Y = 100;

	// リソース名
	enum class SRC
	{
		NONE,
		TITLE_LOGO,
		CAFE,
		GROUND,
		PUSH_SPACE,
		PUSH_SPACE_SE,

		PLAYER,
		PLAYER_SHADOW,
		SPHERE,
		FOOT_SMOKE,
		ENEMY,
		ENEMY_BULLET,
		SKY_DOME,

		COIN,

		//ステージオブジェクト
		FLOOR,
		STAGE,
		THEATER,
		ROCK,

		//音
		TITLE_BGM,
		EXPLORE_BGM,
		LIGHT_UP_SE,
		GAME_BGM,
		PARRY_SE,
	};

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(SRC src);

	// リソースの複製ロード(モデル用)
	int LoadModelDuplicate(SRC src);

	// stringからSRCに変換（ステージオブジェクトがstringで管理されているため）
	SRC StringToSRC(const std::string& name);

	//シーンごとにデータを読み込むことにする
	void InitTitle(void);
	void InitMovie(void);
	void InitSelect(void);
	void InitTutorial(void);
	void InitGame(void);
	void InitPause(void);
	void InitResult(void);

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	std::map<SRC, std::unique_ptr<Resource>> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, Resource&> loadedMap_;

	Resource dummy_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);

};
