#pragma once
#include <vector>
#include <memory>
#include "../Object/Common/Transform.h"
#include "SceneBase.h"

class PixelMaterial;
class PixelRenderer;
class Player;
class Enemy;
class Stage;
class EncountScene;
class PointLight;
class SpotLight;

class GameScene : public SceneBase
{
public:
	
	//状態
	enum class STATE
	{
		NONE,		//初期化用
		LOADING,	//読み込み
		WAKE_UP,	//ゲーム開始
		EXPLORE,	//探索
		ENCOUNT,	//エンカウント演出
		BATTLE,		//戦闘
		ENEMY_SUMMON,//雑魚敵召喚
		BATTLE_SECOND,//戦闘第2フェーズ
	};

	//コンストラクタ
	GameScene(void);

	//デストラクタ
	~GameScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

	VECTOR GetPointLightPos();
	VECTOR GetSpotLightPos();

private:
	// ポストエフェクト用スクリーン
	int postEffectScreen_;

	// ポストエフェクト用(ブラー)
	std::unique_ptr<PixelMaterial> blurMaterial_;
	std::unique_ptr<PixelRenderer> blurRenderer_;

	//状態管理
	STATE state_;		//現在の状態

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理
	std::function<void(void)> stateUpdate_;		//更新ステップ
	std::function<void(void)> stateDraw_;		//描画ステップ

	//フェーズ管理
	bool isFaseChange_;

	//プレイヤー
	std::shared_ptr<Player> player_;
	//敵
	std::shared_ptr<Enemy> enemy_;
	//ステージ
	std::shared_ptr<Stage> stage_;
	//エンカウント演出
	std::unique_ptr<EncountScene> encountScene_;

	//演出スキップ用タイマー
	float skipTimer_;
	//スキップ中かどうか
	bool isSkip_;	

	//
	std::vector<std::unique_ptr<PointLight>> pointLight_;
	std::vector<std::unique_ptr<SpotLight>> spotLight_;

	float loadingTime_;

	void Backstab(void);

	void InitStateExplore(void);

	void InitStateBattle(void);

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(STATE state);

	/// <summary>
	/// 状態遷移：LOADING
	/// </summary>
	void ChangeStateLoading(void);

	/// <summary>
	/// 状態遷移：WAKE_UP
	/// </summary>
	void ChangeStateWakeUp(void);

	/// <summary>
	/// 状態遷移：EXPLORE
	/// </summary>
	void ChangeStateExplore(void);

	/// <summary>
	/// 状態遷移：ENCOUNT
	/// </summary>
	void ChangeStateEncount(void);

	/// <summary>
	/// 状態遷移：BATTLE
	/// </summary>
	void ChangeStateBattle(void);

	/// <summary>
	/// 状態遷移：ENEMY_SUMMON
	/// </summary>
	void ChangeStateEnemySummon(void);

	/// <summary>
	/// 状態遷移：BATTLE_SECOND
	/// </summary>
	void ChangeStateBattleSecond(void);

	//状態ごとの更新と描画--------------------------------------------------------
	
	void LoadingUpdate(void);
	void LoadingDraw(void);

	//ゲーム開始

	/// <summary>
	/// ゲーム開始の更新処理
	/// </summary>
	void UpdateWakeUp(void);

	/// <summary>
	/// ゲーム開始の描画処理
	/// </summary>
	void DrawWakeUp(void);

	//探索

	/// <summary>
	/// 探索フェーズの更新処理
	/// </summary>
	void UpdateExplore(void);

	/// <summary>
	/// 探索フェーズの描画処理
	/// </summary>
	void DrawExplore(void);

	//エンカウント演出

	/// <summary>
	/// エンカウント中の更新処理
	/// </summary>
	void UpdateEncount(void);

	/// <summary>
	/// エンカウント中の描画処理
	/// </summary>
	void DrawEncount(void);

	//戦闘

	/// <summary>
	/// ゲーム中の更新処理
	/// </summary>
	void UpdateBattle(void);

	/// <summary>
	/// ゲーム中の描画
	/// </summary>
	void DrawBattle(void);

	//雑魚敵召喚

	/// <summary>
	/// 雑魚敵召喚の更新処理
	/// </summary>
	void UpdateEnemySummon(void);

	/// <summary>
	/// 雑魚敵召喚の描画
	/// </summary>
	void DrawEnemySummon(void);

	//戦闘第2フェーズ

	/// <summary>
	/// 戦闘第2フェーズの更新処理
	/// </summary>
	void UpdateBattleSecond(void);

	/// <summary>
	/// 戦闘第2フェーズの描画
	/// </summary>
	void DrawBattleSecond(void);

	void SkipBarDraw(void);
	
	/// <summary>
	/// ゲーム中のメッセージ描画処理
	/// </summary>
	/// <param name="wStr">描画する文字列</param>
	void DrawMessage(const std::wstring& wStr);

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);
};
