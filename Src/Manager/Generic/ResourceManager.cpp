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
	static const std::unordered_map<std::string, SRC> map = {
		{ "Counter", SRC::COUNTER },
		{ "Table",   SRC::TABLE },
		{ "Sweets_Choco_Rack",  SRC::CHOCO_RACK },
		{ "Sweets_Choco",		SRC::SWEETS_CHOCO },
		{ "Sweets_Strawberry_Rack",   SRC::BERRY_RACK },
		{ "Sweets_Strawberry",  SRC::SWEETS_BERRY },
		{ "Coffee_Machine",   SRC::COFFEE_MACHINE },
		{ "Hot_Cup",		SRC::HOTCUP },
		{ "Cup_Hot_Rack",   SRC::HOTCUP_RACK },
		{ "Hot_Coffee",		SRC::HOTCOFFEE },
		{ "Ice_Dispenser",			SRC::ICEDISPENSER },
		{ "Ice",			SRC::ICE },
		{ "Ice_Cup",		SRC::ICECUP },
		{ "Cup_Ice_Rack",	SRC::ICECUP_RACK },
		{ "Ice_Coffee",		SRC::ICECOFFEE },
		{ "Cup_Lid_Rack",	SRC::CUPLID_RACK },
		{ "Hot_Cup_Lid",		SRC::HOTCUP_LID },
		{ "Ice_Cup_Lid",		SRC::ICECUP_LID },
		{ "Dust_Box",		SRC::DUSTBOX },
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

	//お店
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/cafe.mv1");
	resourcesMap_.emplace(SRC::CAFE, std::move(res));

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

	//説明用画像
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "tutorial.png",
		3, 1, 1080, 1080);
	resourcesMap_.emplace(SRC::TUTORIAL, std::move(res));

	//背景用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "tutorial_back.png");
	resourcesMap_.emplace(SRC::TUTORIAL_BACK, std::move(res));

	//装飾用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Hangin_Banner.png");
	resourcesMap_.emplace(SRC::PINK_BANNER, std::move(res));

	//カーソル用画像
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "cursor_R.png",
		2,1,300,300);
	resourcesMap_.emplace(SRC::CURSOR_R, std::move(res));

	//PushSpace画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "PleaseKey.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//音----------------------------------------------------------------------------
	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Tutorial.mp3");
	resourcesMap_.emplace(SRC::TUTORIAL_BGM, std::move(res));

	//ページを進ませるときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/next_page.mp3");
	resourcesMap_.emplace(SRC::NEXT_PAGE, std::move(res));

	//ページを戻すときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/return_page.mp3");
	resourcesMap_.emplace(SRC::RETURN_PAGE, std::move(res));

	//ページが進まないときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/not_page.mp3");
	resourcesMap_.emplace(SRC::NOT_PAGE, std::move(res));

	//開店カランコロン
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/open_door.mp3");
	resourcesMap_.emplace(SRC::OPEN_DOOR, std::move(res));
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

	//プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Player/Model.mv1");
	resourcesMap_.emplace(SRC::PLAYER, std::move(res));

	//プレイヤー影
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Shadow.png");
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, std::move(res));

	//家具：床
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Floor/floor.mv1");
	resourcesMap_.emplace(SRC::FLOOR, std::move(res));

	//足煙
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "Smoke/Smoke.efkefc");
	resourcesMap_.emplace(SRC::FOOT_SMOKE, std::move(res));

	//タイマー用-----------------------------------------------------------
	//数字
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "Timer_Numbers.png",
		NUMBER_NUM_X, NUMBER_NUM_Y, NUMBER_SIZE_X, NUMBER_SIZE_Y);
	resourcesMap_.emplace(SRC::TIMER_NUMBER, std::move(res));

	//コロン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "colon.png");
	resourcesMap_.emplace(SRC::TIMER_COLON, std::move(res));
	//---------------------------------------------------------------------

	//音------------------------------------------------------------------------
	
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

	//背景用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "tutorial_back.png");
	resourcesMap_.emplace(SRC::TUTORIAL_BACK, std::move(res));

	//メニューリスト
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "MenuList.png",1,5,1080,216);
	resourcesMap_.emplace(SRC::MENU_LIST, std::move(res));
	
	//メニューカーソル
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Menu_Cursor.png");
	resourcesMap_.emplace(SRC::MENU_CURSOR, std::move(res));

	//説明用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Explain.png");
	resourcesMap_.emplace(SRC::TUTORIAL, std::move(res));

	//メニューへ戻る画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Menu_Back.png");
	resourcesMap_.emplace(SRC::MENU_BACK_LOGO, std::move(res));
	
	//操作方法
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "keyconfig.png");
	resourcesMap_.emplace(SRC::MENU_KEY_CONFIG, std::move(res));

	//ページを進ませるときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/next_page.mp3");
	resourcesMap_.emplace(SRC::NEXT_PAGE, std::move(res));

	//ページを戻すときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/return_page.mp3");
	resourcesMap_.emplace(SRC::RETURN_PAGE, std::move(res));

	//ページを戻すときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/menu_back.mp3");
	resourcesMap_.emplace(SRC::MENU_BACK, std::move(res));

	//メニューを閉じるときのSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/menu_cancel.mp3");
	resourcesMap_.emplace(SRC::MENU_CLOSE, std::move(res));
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

	//円（C)
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Circle_C.png");
	resourcesMap_.emplace(SRC::RANK_C, std::move(res));

	//円（B)
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Circle_B.png");
	resourcesMap_.emplace(SRC::RANK_B, std::move(res));

	//円（A)
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Circle_A.png");
	resourcesMap_.emplace(SRC::RANK_A, std::move(res));

	//円（S)
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Circle_S.png");
	resourcesMap_.emplace(SRC::RANK_S, std::move(res));

	//ゲージ影UI
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "CircleGage.png");
	resourcesMap_.emplace(SRC::UI_CIRCLESHADOW, std::move(res));

	//数字
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "Numbers.png",
		NUMBER_NUM_X, NUMBER_NUM_Y, 128, 128);
	resourcesMap_.emplace(SRC::SCORE_NUMBER, std::move(res));
	
	//現在スコアラベル
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "current_score.png");
	resourcesMap_.emplace(SRC::CURRENT_SCORE, std::move(res));

	//ランク（C,B,A,S)
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "Ranks.png",
		4, 1, 300, 300);
	resourcesMap_.emplace(SRC::RANKS, std::move(res));

	//ランキングラベル
	res = std::make_unique<RES>(RES_T::IMGS, PATH_IMG + "ranking.png",
		1, 5, 300, 100);
	resourcesMap_.emplace(SRC::RANKING_LABEL, std::move(res));

	//ランキング背景
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "ranking_back.png");
	resourcesMap_.emplace(SRC::RANKING_BACK, std::move(res));

	//装飾用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Hangin_Banner.png");
	resourcesMap_.emplace(SRC::PINK_BANNER, std::move(res));

	//PushSpace画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "PleaseKey.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//音------------------------------------------------------------------------

	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Result.mp3");
	resourcesMap_.emplace(SRC::RESULT_BGM, std::move(res));

	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/normal.mp3");
	resourcesMap_.emplace(SRC::SCORE_NORMAL, std::move(res));

	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/good.mp3");
	resourcesMap_.emplace(SRC::SCORE_GOOD, std::move(res));

	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/greate.mp3");
	resourcesMap_.emplace(SRC::SCORE_GREATE, std::move(res));

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
