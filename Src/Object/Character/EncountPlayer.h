#pragma once
#include <functional>
#include <map>
#include "ActorBase.h"

class Capsule;
class AnimationController;

class EncountPlayer : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,		//初期化前
		STAGE_WALK,	//ステージ上を歩く
		STAGE_WAIT,	//待機
		LOOK_AROUND,//周りを見渡す
		ATTACKED_ENEMY,	//敵に攻撃される
	};

	//アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,		//通常
		WALK_SLOW,	//ゆっくり歩く
		LOOK_AROUND,//周りを見渡す
		ATTACKED,	//攻撃をされる
	};

	//コンストラクタ
	EncountPlayer(void);

	//デストラクタ
	~EncountPlayer(void);

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

	/// <summary>
	/// エンカウント演出開始
	/// </summary>
	void EncountStart(void);

	/// <summary>
	/// 周りを見渡す状態へ
	/// </summary>
	void LookAround(void);

	/// <summary>
	/// 攻撃される状態へ	
	/// </summary>
	void AttackedEnemy(void);

	/// <summary>
	/// 現在の状態を取得
	/// </summary>
	/// <returns>現在の状態</returns>
	const STATE& GetState(void)const { return state_; }

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name=""></param>
	void UpdateImGui(void);
private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;		//現在の状態

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	//移動量
	VECTOR movePow_;

	//移動後の座標
	VECTOR movedPos_;

	//ジャンプ量
	VECTOR jumpPow_;

	//行動終了判定
	bool isActionEnd_;

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// 当たり判定の初期化
	/// </summary>
	void InitCollider(void);

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	/// <summary>
	/// エンカウント演出：ステージ上を歩く準備
	/// </summary>
	/// <param name=""></param>
	void StageWalkReady(void);

	//状態遷移処理--------------------------------------------------------

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(const STATE& state);

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：STAGE_WALK
	/// </summary>
	void ChangeStateStageWalk(void);
	/// <summary>
	/// 状態遷移：STAGE_WAIT
	/// </summary>
	void ChangeStateStageWait(void);
	/// <summary>
	/// 状態遷移：LOOK_AROUND
	/// </summary>
	void ChangeStateLookAround(void);
	/// <summary>
	/// 状態遷移：ATTACKED_ENEMY
	/// </summary>
	void ChangeStateAttackedEnemy(void);

	//状態遷移処理--------------------------------------------------------

	/// <summary>
	/// 状態更新：NONE
	/// </summary>
	void UpdateNone(void);
	/// <summary>
	/// 状態更新：STAGE_WALK
	/// </summary>
	void UpdateStageWalk(void);
	/// <summary>
	/// 状態更新：STAGE_WAIT
	/// </summary>
	void UpdateStageWait(void);
	/// <summary>
	/// 状態更新：LOOK_AROUND
	/// </summary>
	void UpdateLookAround(void);
	/// <summary>
	/// 状態更新：ATTACKED_ENEMY
	/// </summary>
	void UpdateAttackedEnemy(void);

	//衝突判定---------------------------------------

	/// <summary>
	/// 衝突判定処理
	/// </summary>
	void Collision(void);

	/// <summary>
	/// カプセルの衝突判定処理
	/// </summary>
	void CollisionCapsule(void);

	/// <summary>
	/// 重力方向の衝突判定処理
	/// </summary>
	void CollisionGravity(void);

	//------------------------------------------------
};

