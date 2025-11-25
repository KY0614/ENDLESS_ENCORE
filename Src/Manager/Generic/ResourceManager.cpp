#include <DxLib.h>
#include <unordered_map>
#include <map>
#include "../../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{

}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
	resourcesMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::SRC ResourceManager::StringToSRC(const std::string& name)
{
	static const std::unordered_map<std::string, SRC> map = 
	{
		//{ "Counter", SRC::COUNTER },
		//{ "Table",   SRC::TABLE },
		//{ "Sweets_Choco_Rack",  SRC::CHOCO_RACK },
		//{ "Sweets_Choco",		SRC::SWEETS_CHOCO },
		//{ "Sweets_Strawberry_Rack",   SRC::BERRY_RACK },
		//{ "Sweets_Strawberry",  SRC::SWEETS_BERRY },
		//{ "Coffee_Machine",   SRC::COFFEE_MACHINE },
		//{ "Hot_Cup",		SRC::HOTCUP },
		//{ "Cup_Hot_Rack",   SRC::HOTCUP_RACK },
		//{ "Hot_Coffee",		SRC::HOTCOFFEE },
		//{ "Ice_Dispenser",			SRC::ICEDISPENSER },
		//{ "Ice",			SRC::ICE },
		//{ "Ice_Cup",		SRC::ICECUP },
		//{ "Cup_Ice_Rack",	SRC::ICECUP_RACK },
		//{ "Ice_Coffee",		SRC::ICECOFFEE },
		//{ "Cup_Lid_Rack",	SRC::CUPLID_RACK },
		//{ "Hot_Cup_Lid",		SRC::HOTCUP_LID },
		//{ "Ice_Cup_Lid",		SRC::ICECUP_LID },
		//{ "Dust_Box",		SRC::DUSTBOX },
		// 新しい要素はここに追加
	};

	auto it = map.find(name);
	if (it != map.end()) {
		return it->second;
	}

	return SRC::NONE; // 不正な名前が来たときのデフォルト対応
}

void ResourceManager::InitTitle(void)
{
	//推奨しませんが、どうしても使いたい方は
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::unique_ptr<Resource> res;

	//PushSpace画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "PleaseKey.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//タイトルロゴ
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Title.png");
	resourcesMap_.emplace(SRC::TITLE_LOGO, std::move(res));

	//プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Player/Player.mv1");
	resourcesMap_.emplace(SRC::PLAYER, std::move(res));

	//スカイドーム
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "SkyDome/SkyDome.mv1");
	resourcesMap_.emplace(SRC::SKY_DOME, std::move(res));

	//家具：床
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/floor.mv1");
	resourcesMap_.emplace(SRC::FLOOR, std::move(res));

	//地面テクスチャ
	res = std::make_unique<RES>(RES_T::IMG, PATH_MDL + "Stage/Tex/ground.png");
	resourcesMap_.emplace(SRC::GROUND, std::move(res));

	//音------------------------------------------------------------------------

	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Title.mp3");
	resourcesMap_.emplace(SRC::TITLE_BGM, std::move(res));

	//SE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/press_key.mp3");
	resourcesMap_.emplace(SRC::PUSH_SPACE_SE, std::move(res));

	//--------------------------------------------------------------------------
}

void ResourceManager::InitMovie(void)
{
}

void ResourceManager::InitSelect(void)
{
}

void ResourceManager::InitTutorial(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::unique_ptr<Resource> res;

}

void ResourceManager::InitGame(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::unique_ptr<Resource> res;

	//ステージ
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Stage_.mv1");
	resourcesMap_.emplace(SRC::STAGE, std::move(res));

	//ステージ
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/long.mv1");
	resourcesMap_.emplace(SRC::THEATER, std::move(res));

	//敵
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Enemy/Magician/Magician_.mv1");
	resourcesMap_.emplace(SRC::ENEMY, std::move(res));

	//敵の弾
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Enemy/Arrow.mv1");
	resourcesMap_.emplace(SRC::ENEMY_BULLET, std::move(res));

	//プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Player/Player.mv1");
	resourcesMap_.emplace(SRC::PLAYER, std::move(res));

	//ー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Sphere.mv1");
	resourcesMap_.emplace(SRC::SPHERE, std::move(res));

	//プレイヤー影
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Shadow.png");
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, std::move(res));

	//床
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Floor/floor.mv1");
	resourcesMap_.emplace(SRC::FLOOR, std::move(res));

	//コイン
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Coin.mv1");
	resourcesMap_.emplace(SRC::COIN, std::move(res));

	//足煙
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "Smoke/Smoke.efkefc");
	resourcesMap_.emplace(SRC::FOOT_SMOKE, std::move(res));

	//音------------------------------------------------------------------------
	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Battle.mp3");
	resourcesMap_.emplace(SRC::GAME_BGM, std::move(res));
	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Explore.mp3");
	resourcesMap_.emplace(SRC::EXPLORE_BGM, std::move(res));
	
	//SE
	
	//ライトアップ
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Light Up.mp3");
	resourcesMap_.emplace(SRC::LIGHT_UP_SE, std::move(res));
	
	//パリィ
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Parry.mp3");
	resourcesMap_.emplace(SRC::PARRY_SE, std::move(res));

}

void ResourceManager::InitPause(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::unique_ptr<Resource> res;
}

void ResourceManager::InitResult(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	std::unique_ptr<Resource> res;

	//PushSpace画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "PleaseKey.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//音------------------------------------------------------------------------
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{

	//ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return lPair->second;
	}

	//リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		//登録されていない
		return dummy_;
	}

	//ロード処理
	rPair->second->Load();

	//念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;

}
