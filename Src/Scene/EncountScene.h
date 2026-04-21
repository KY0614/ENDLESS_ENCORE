#pragma once
#include <map>
#include <functional>
#include "SceneBase.h"

class Fader;
class Player;
class Enemy;
class EncountPlayer;
class EncountEnemy;
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

		ENEMY_CAST_SPELL,		//敵が魔法を唱える
		ENEMY_ATTACK,			//敵が攻撃する
		LETS_PARRY,				//パリィの合図
		PLAYER_PARRY,			//パリィ

		FINISH					//終了
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="encountEnemy">エンカウント演出用の敵参照</param>
	/// <param name="encountPlayer">エンカウント演出用のプレイヤー参照</param>
	EncountScene(EncountEnemy& encountEnemy, EncountPlayer& encountPlayer);

	//デストラクタ
	~EncountScene(void);

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
	/// エンカウント演出開始
	/// </summary>
	void Start(void);

	/// <summary>
	/// スローモーション開始
	/// </summary>
	void StartSlowMotion(void) { isSlowMotion_ = true; }

	/// <summary>
	/// 停止
	/// </summary>
	void Stop(void) { isStop_ = true; }

	/// <summary>
	/// スローモーションするかどうか取得
	/// </summary>
	/// <returns>true:スローモーション中、false:スローモーションしない</returns>
	const bool& IsSlowMotion(void)const { return isSlowMotion_; }

	/// <summary>
	/// 停止中かどうか取得
	/// </summary>
	/// <returns>true:停止中、false:停止していない</returns>
	const bool& IsStop(void)const { return isStop_; }

	/// <summary>
	/// エンカウント演出が終了したかどうか取得
	/// </summary>
	/// <returns>true:終了　false：まだ終了していない</returns>
	const bool& IsFinished(void) const { return isFinish_; }

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui(void) override;

private:
	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//参照
	EncountEnemy& encountEnemy_;		//エンカウント演出用の敵参照
	EncountPlayer& encountPlayer_;		//エンカウント演出用のプレイヤー参照

	//インターバルタイマー
	float intervalTimer_;	

	//ライトアップ用フラグ true:ライトアップ済み false:ライトアップ待ち
	bool isLightUp_;

	//スローモーションフラグ true:スローモーション中 false:通常速度
	bool isSlowMotion_;	
	//一時停止フラグ true:停止中 false:通常進行
	bool isStop_;

	//シーン終了フラグ
	bool isFinish_;				

	/// <summary>
	/// サウンド初期化処理
	/// </summary>
	void InitSound(void);

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
	/// 状態遷移：ENEMY_CAST_SPELL
	/// </summary>
	void ChangeStateEnemyCastSpell(void);
	/// <summary>
	/// 状態遷移：ENEMY_ATTACK
	/// </summary>
	void ChangeStateEnemyAttack(void);
	/// <summary>
	/// 状態遷移：LETS_PARRY
	/// </summary>
	void ChangeStateLetsParry(void);
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
	/// 更新：ENEMY_CAST_SPELL
	/// </summary>
	void UpdateEnemyCastSpell(void);
	/// <summary>
	/// 更新：ENEMY_ATTACK
	/// </summary>
	void UpdateEnemyAttack(void);
	/// <summary>
	/// 更新：LETS_PARRY
	/// </summary>
	void UpdateLetsParry(void);
	/// <summary>
	/// 更新：FINISH
	/// </summary>
	void UpdateFinish(void);
};

