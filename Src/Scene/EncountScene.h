#pragma once
#include <map>
#include <functional>
#include "SceneBase.h"

class Fader;
class Player;
class Enemy;
class Stage;

class EncountScene :  public SceneBase
{
public:
	//演出ごとの状態
	enum class STATE
	{
		NONE,                   //初期状態
		FADE,					//演出初めの暗転明転
		PLAYER_WALK,			//プレイヤー前進
		PLAYER_ATTENTION,		//プレイヤー注目
		BLACK_OUT,				//照明消灯
		LOOK_AROUND,			//周りを見渡す
		ENEMY_SPOTLIGHT,		//敵をスポットライトで照らす
		ENEMY_ATTENTION,        //敵が出現
		ENEMY_TURN,				//敵が振り向く
		FADE2FINISH,			//敵が振り向く
		FINISH					//終了
	};

	//コンストラクタ
	EncountScene(Player& player,Enemy& enemy);

	//デストラクタ
	~EncountScene(void);

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

	void Start(void);

	const bool& IsFinished(void) const { return isFinish_; }

private:
	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// フェード
	std::unique_ptr<Fader> fader_;

	Player& player_;			//プレイヤー参照
	Enemy& enemy_;				//敵参照

	float intervalTimer_;		//インターバルタイマー
	bool isStateActioned_;

	bool isFinish_;				//シーン終了フラグ

	/// <summary>
	/// 状態遷移
	/// </summary>
	/// <param name="state">遷移させる状態</param>
	void ChangeState(STATE state);

	//状態遷移処理----------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：FADE
	/// </summary>
	void ChangeStateFade(void);
	/// <summary>
	/// 状態遷移：PLAYER_WALK
	/// </summary>
	void ChangeStatePlayerWalk(void);
	/// <summary>
	/// 状態遷移：PLAYER_ATTENTION
	/// </summary>
	void ChangeStatePlayerAttention(void);
	/// <summary>
	/// 状態遷移：BLACK_OUT
	/// </summary>
	void ChangeStateBlackOut(void);
	/// <summary>
	/// 状態遷移：LOOK_AROUND
	///	</summary>
	void ChangeStateLookAround(void);
	/// <summary>
	/// 状態遷移：ENEMY_SPOTLIGHT
	/// </summary>
	void ChangeStateEnemySpotlight(void);
	/// <summary>
	/// 状態遷移：ENEMY_ATTENTION
	/// </summary>
	void ChangeStateEnemyAttention(void);
	/// <summary>
	/// 状態遷移：FINISH
	/// </summary>
	void ChangeStateFinish(void);

	//状態更新処理----------------------------------------------------------

	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void);
	/// <summary>
	/// 更新：FADE
	/// </summary>
	void UpdateFade(void);
	/// <summary>
	/// 更新：PLAYER_WALK
	/// </summary>
	void UpdatePlayerWalk(void);
	/// <summary>
	/// 更新：PLAYER_ATTENTION
	/// </summary>
	void UpdatePlayerAttention(void);
	/// <summary>
	/// 更新：BLACK_OUT
	/// </summary>
	void UpdateBlackOut(void);
	/// <summary>
	/// 更新：LOOK_AROUND
	/// </summary>
	void UpdateLookAround(void);
	/// <summary>
	/// 更新：ENEMY_SPOTLIGHT
	/// </summary>
	void UpdateEnemySpotlight(void);
	/// <summary>
	/// 更新：ENEMY_ATTENTION
	/// </summary>
	void UpdateEnemyAttention(void);
	/// <summary>
	/// 更新：FINISH
	/// </summary>
	void UpdateFinish(void);

	void DebugDraw(void);
};

