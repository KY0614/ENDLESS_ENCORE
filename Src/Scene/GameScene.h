#pragma once
#include <vector>
#include <memory>
#include "../Object/Common/Transform.h"
#include "SceneBase.h"

class Player;
class Enemy;
class EncountPlayer;
class EncountEnemy;
class Stage;
class Tutorial;
class EncountScene;
class SkipBar;

class GameScene : public SceneBase
{
public:
	
	//状態
	enum class STATE
	{
		NONE,			//初期化用
		WAKE_UP,		//ゲーム開始
		EXPLORE,		//探索
		ENCOUNT,		//エンカウント演出
		BATTLE,			//戦闘
	};

	//コンストラクタ
	GameScene(void);

	//デストラクタ
	~GameScene(void);

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

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name=""></param>
	void UpdateImGui(void) override;
private:

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
	//エンカウント演出用のプレイヤー
	std::shared_ptr<EncountPlayer> encountPlayer_;
	//敵
	std::shared_ptr<Enemy> enemy_;
	//エンカウント演出用の敵
	std::shared_ptr<EncountEnemy> encountEnemy_;
	//ステージ
	std::shared_ptr<Stage> stage_;
	//チュートリアル
	std::shared_ptr<Tutorial> tutorial_;
	//エンカウント演出
	std::unique_ptr<EncountScene> encountScene_;
	//スキップバーUI
	std::unique_ptr<SkipBar> skipBarUI_;

	//演出スキップ用タイマー
	float skipTimer_;
	//スキップ中かどうか
	bool isSkip_;	

	//スローモーション用フレームカウント
	float slowMotionFrameCount_;
	float slowMotionFrame_;

	///フォントハンドル
	int fontHandle_;

	/// <summary>
	/// チュートリアル初期化処理
	/// </summary>
	void InitTutorial(void);

	/// <summary>
	/// サウンド初期化処理
	/// </summary>
	void InitSound(void);

	/// <summary>
	/// 探索状態初期化処理
	/// </summary>
	void InitStateExplore(void);

	void InitStaeEncount(void);

	/// <summary>
	/// 戦闘状態初期化処理
	/// </summary>
	void InitStateBattle(void);

	/// <summary>
	/// バックスタブ処理
	/// </summary>
	void Backstab(void);

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(STATE state);

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

	//状態ごとの更新と描画
	//ゲーム開始-----------------------------------------------------

	/// <summary>
	/// ゲーム開始の更新処理
	/// </summary>
	void UpdateWakeUp(void);

	/// <summary>
	/// ゲーム開始の描画処理
	/// </summary>
	void DrawWakeUp(void);

	//探索-----------------------------------------------------

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

	//戦闘-----------------------------------------------------

	/// <summary>
	/// ゲーム中の更新処理
	/// </summary>
	void UpdateBattle(void);

	/// <summary>
	/// ゲーム中の描画
	/// </summary>
	void DrawBattle(void);

	//---------------------------------------------------------

	/// <summary>
	/// スキップバーの描画処理
	/// </summary>
	void SkipBarDraw(void);

	/// <summary>
	/// オブジェクトのImGui更新処理
	/// </summary>
	void ObjectUpdateImGui(void);
};
