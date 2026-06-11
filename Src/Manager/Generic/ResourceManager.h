#pragma once
#include <memory>
#include <map>
#include <string>
#include "Resource.h"

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		NONE,

		//タイトル関連
		TITLE_LOGO,	//タイトルロゴ画像
		PUSH_SPACE,	//プッシュスペース画像
		FILM_NOISE,	//フィルムノイズ

		//キャラクター関連
		CHARACTOR_SHADOW,	//キャラクターの影

		//プレイヤー関連
		PLAYER,				//プレイヤーの3Dモデル
		PLAYER_HP_BAR,		//プレイヤーのHPバー
		BAR_BACK,			//HPバーの背景
		BAR_FRAME,			//HPバーの額縁
		PLAYER_PARYY_BAR,	//パリィバー
		PLAYER_PARYY_CD_BAR,//パリィクールダウンバー
		FOOT_SMOKE,			//足元の煙エフェクト
		PARRY_EFKT,			//パリィエフェクト

		//ボス敵関連
		FIRE_EFFECT,	//火炎エフェクト
		CHARGE_EFFECT,	//チャージエフェクト
		EXPLOSIVE_EFFECT,//爆発エフェクト
		COSMIC_EFFECT,	//コズミックエフェクト
		BLOOD_EFFECT,	//血しぶき
		ENEMY,			//敵
		ENEMY_BULLET,	//敵の弾
		ENEMY_HP_BAR,	//敵のHPバー

		//雑魚敵関連
		FIGHTER_GHOST,	//近接型の幽霊
		MAGE_GHOST,		//遠距離型の幽霊

		//演出関連
		VICTORY,
		YOU_DIED,

		//ステージオブジェクト
		THEATER,
		MIST_WALL,
		NOISE_TEXTURE,
		ROCK,

		//音
		TITLE_BGM,		//タイトルBGM
		PUSH_SPACE_SE,	//プッシュスペースSE
		FILM_SCROLL_SE,	//フィルムスクロールSE
		EXPLORE_BGM,	//探索BGM
		LIGHT_UP_SE,	//ライトアップSE
		BATTLE_BGM,		//バトルBGM
		PARRY_SE,		//パリィSE
		DAMAGE_SE,		//
		WAKE_UP_SE,		//起き上がるSE(布が擦れる音)
		FIRE_SE,		//火炎SE（火を噴く音）		
		EXPLOSION_SE,	//爆発SE（ため攻撃用の音）
		BACKSTAB_SE,	//致命攻撃SE(ぐさって音)

		//フォント
		TUTORIAL_FONT,
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

	//シーンごとにデータを読み込むことにする
	void InitTitle(void);
	void InitGame(void);
	void InitPause(void);

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
