#pragma once
#include "../../Common/ActorBase.h"

class ClearPlayer : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,		//初期化前
	};

	//アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,		//通常
		WALK_SLOW,	//ゆっくり歩く
		LOOK_AROUND,//周りを見渡す
	};

	//コンストラクタ
	ClearPlayer(void);
	//デストラクタ
	~ClearPlayer(void)override;

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void) override;

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;
};

#pragma region 流れ？

//１朽ちていく敵
//２プレイヤーが見つめている
//３後ろから拍手喝采
//4プレイヤーが振り向く
//５カメラが引いていくと同時にカーテン（幕）が閉じる
//フェードアウト
//void Init(void);
//void Update(void);
//void Draw(void);
//void Release(void);
#pragma endregion

