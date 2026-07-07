#pragma once
#include <functional>
#include "../../Common/ActorBase.h"

class AnimationController;

class ClearPlayer : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,		//初期化前
		LOOK_ENEMY,	//敵を見つめる
		TURN,		//振り向く
		WAIT,		//待機
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

private:
	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;		//現在の状態

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	VECTOR movePow_;

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	//状態遷移

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">変更したい状態</param>
	void ChangeState(const STATE& state);
 
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

