#pragma once
#include "SceneBase.h"

class ClearScene : public SceneBase
{
public:
	//演出ごとの状態
	enum class STATE
	{
		NONE,                   //初期状態
		ENEMY_DISSOVLE,			//敵が消える
		PLAYER_LOOK_ENEMY,		//プレイヤーが敵を見つめる
		CLAP,					//拍手
		PLAYER_TURN,			//プレイヤーが振り向く
		CURTAIN_CLOSE,			//カーテンが閉まる
		FINISH					//終了
	};

	//コンストラクタ
	ClearScene(void);

	//デストラクタ
	~ClearScene(void);

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
	/// 演出開始
	/// </summary>
	void Start(void);

private:
};

