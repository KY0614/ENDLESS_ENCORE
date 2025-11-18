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
	// 演出のサブステートを定義
	enum class STATE
	{
		NONE,                   //初期状態
		FADE,					//演出初めの暗転明転
		PLAYER_WALK,			//プレイヤー前進
		PLAYER_ATTENTION,		//プレイヤー前進
		BLACK_OUT,				//照明消灯
		LOOK_AROUND,			//周りを見渡す
		ENEMY_SPOTLIGHT,		//敵をスポットライトで照らす
		ENEMY_ATTENTION,        //敵が振り向く
		FINISHED                //終了
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

	bool isBlackOutNotice_;

	/// <summary>
	/// フェードアウトが終了したかどうか
	/// </summary>
	/// <returns>true: 終了 false : 終了していない</returns>
	bool IsFadeOutEnd(void);

	/// <summary>
	/// フェードインが終了したかどうか
	/// </summary>
	/// <returns>true: 終了 false : 終了していない</returns>
	bool IsFadeInEnd(void);

	void ChangeState(STATE state);

	void ChangeStateNone(void);
	void ChangeStateFade(void);
	void ChangeStatePlayerWalk(void);
	void ChangeStatePlayerAttention(void);
	void ChangeStateBlackOut(void);
	void ChangeStateLookAround(void);
	void ChangeStateEnemySpotlight(void);
	void ChangeStateEnemyAttention(void);

	void UpdateNone(void);
	void UpdateFade(void);
	void UpdatePlayerWalk(void);
	void UpdatePlayerAttention(void);
	void UpdateBlackOut(void);
	void UpdateLookAround(void);
	void UpdateEnemySpotlight(void);
	void UpdateEnemyAttention(void);

	void DebugDraw(void);
};

