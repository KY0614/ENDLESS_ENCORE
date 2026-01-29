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
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Push_Space.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//タイトルロゴ
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Title.png");
	resourcesMap_.emplace(SRC::TITLE_LOGO, std::move(res));

	//ステージ
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Theater/large_theater.mv1");
	resourcesMap_.emplace(SRC::THEATER, std::move(res));

	//ノイズ用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "FilmNoise.png");
	resourcesMap_.emplace(SRC::FILM_NOISE, std::move(res));

	//音------------------------------------------------------------------------

	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Title.mp3");
	resourcesMap_.emplace(SRC::TITLE_BGM, std::move(res));

	//上映開始のSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Push_Space.mp3");
	resourcesMap_.emplace(SRC::PUSH_SPACE_SE, std::move(res));

	//フィルムがまわるSE
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/FilmScroll.mp3");
	resourcesMap_.emplace(SRC::FILM_SCROLL_SE, std::move(res));

	//--------------------------------------------------------------------------
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
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Theater/large_theater.mv1");
	resourcesMap_.emplace(SRC::THEATER, std::move(res));

	//霧の壁
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Mist/MistWall.mv1");
	resourcesMap_.emplace(SRC::MIST_WALL, std::move(res));

	//ノイズ用画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_MDL + "Stage/Mist/Noise.png");
	resourcesMap_.emplace(SRC::NOISE_TEXTURE, std::move(res));

	//敵
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Enemy/Magician/Magician_.mv1");
	resourcesMap_.emplace(SRC::ENEMY, std::move(res));

	//敵の弾
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Enemy/Arrow.mv1");
	resourcesMap_.emplace(SRC::ENEMY_BULLET, std::move(res));

	//プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Player/Player.mv1");
	resourcesMap_.emplace(SRC::PLAYER, std::move(res));

	//プレイヤー影
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Shadow.png");
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, std::move(res));

	//プレイヤーHPバー
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/HPBar.png");
	resourcesMap_.emplace(SRC::PLAYER_HP_BAR, std::move(res));

	//プレイヤーHPバー背景
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/HPBarBack.png");
	resourcesMap_.emplace(SRC::PLAYER_HP_BACK_BAR, std::move(res));

	//プレイヤーHPバーの額縁
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/HPBarFrame.png");
	resourcesMap_.emplace(SRC::PLAYER_HP_BAR_FRAME, std::move(res));

	//プレイヤーパリィバー
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/ParryBar.png");
	resourcesMap_.emplace(SRC::PLAYER_PARYY_BAR, std::move(res));

	//プレイヤーパリィバー
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/ParryCDBar.png");
	resourcesMap_.emplace(SRC::PLAYER_PARYY_CD_BAR, std::move(res));

	//プレイヤーパリィバー
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "UI/EnemyHPBar.png");
	resourcesMap_.emplace(SRC::ENEMY_HP_BAR, std::move(res));

	//勝利
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "VICTORY.png");
	resourcesMap_.emplace(SRC::VICTORY, std::move(res));

	//死亡
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "YOU DIED.png");
	resourcesMap_.emplace(SRC::YOU_DIED, std::move(res));

	//足煙
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "Smoke/Smoke.efkefc");
	resourcesMap_.emplace(SRC::FOOT_SMOKE, std::move(res));

	//音------------------------------------------------------------------------
	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Battle.mp3");
	resourcesMap_.emplace(SRC::BATTLE_BGM, std::move(res));
	//BGM
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "BGM/Explore.mp3");
	resourcesMap_.emplace(SRC::EXPLORE_BGM, std::move(res));
	
	//SE
	
	//ライトアップ
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Light_Up.mp3");
	resourcesMap_.emplace(SRC::LIGHT_UP_SE, std::move(res));
	
	//パリィ
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Barrior.mp3");
	resourcesMap_.emplace(SRC::PARRY_SE, std::move(res));
	
	//パリィ
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/damage.mp3");
	resourcesMap_.emplace(SRC::DAMAGE_SE, std::move(res));
	
	//起き上がるときの音
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/Wake_Up.mp3");
	resourcesMap_.emplace(SRC::WAKE_UP_SE, std::move(res));
	
	//炎の音
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/fire_.mp3");
	resourcesMap_.emplace(SRC::FIRE_SE, std::move(res));
		
	//爆発
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/flame.mp3");
	resourcesMap_.emplace(SRC::EXPLOSION_SE, std::move(res));
	
	//炎の音
	res = std::make_unique<RES>(RES_T::SOUND, PATH_SND + "SE/backstab.mp3");
	resourcesMap_.emplace(SRC::BACKSTAB_SE, std::move(res));

	//エフェクト

	//パリィ
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "Barrior.efkefc");
	resourcesMap_.emplace(SRC::PARRY_EFKT, std::move(res));

	//敵の弾
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "fire_test.efkefc");
	resourcesMap_.emplace(SRC::FIRE_EFFECT, std::move(res));

	//敵のチャージ
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "charge.efkefc");
	resourcesMap_.emplace(SRC::CHARGE_EFFECT, std::move(res));

	//敵のチャージ
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "CosmicMist.efkefc");
	resourcesMap_.emplace(SRC::COSMIC_EFFECT, std::move(res));

	//敵のチャージ
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "charge_atk.efkefc");
	resourcesMap_.emplace(SRC::EXPLOSIVE_EFFECT, std::move(res));

	//敵のチャージ
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + "LossOfBlood.efkefc");
	resourcesMap_.emplace(SRC::BLOOD_EFFECT, std::move(res));
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
