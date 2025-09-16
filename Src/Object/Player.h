#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <DxLib.h>
#include "ActorBase.h"

class AnimationController;
class Collider;
class Capsule;
class Sphere;

class Player : public ActorBase
{

public:

	//スピード
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 7.0f;

	//回転完了までの時間
	static constexpr float TIME_ROT = 0.5f;

	//煙エフェクト発生間隔
	static constexpr float TERM_FOOT_SMOKE = 0.3f;

	//状態
	enum class STATE
	{
		NONE,	//初期化前
		PLAY,	//操作可能
		DEAD,	//死亡
	};

	//アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,	//通常
		WALK,	//歩き
		RUN,	//走り
		JUMP,	//ジャンプ
		PARRY,	//パリィ
		DODGE,	//回避
		USE_ITEM, //アイテム使用
	};

	//コンストラクタ
	Player(void);

	//デストラクタ
	~Player(void);

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
	/// 衝突判定に用いられるコライダーを追加する
	/// </summary>
	/// <param name="collider"></param>
	void AddCollider(std::weak_ptr<Collider> collider);

	/// <summary>
	/// コライダーの削除
	/// </summary>
	/// <param name=""></param>
	void ClearCollider(void);

	/// <summary>
	/// 衝突用カプセルの取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	const Capsule& GetCapsule(void) const;

	const Sphere& GetSphere(void) const { return *sphere_; }

	bool IsPlay(void);

	void ChangeState(STATE state);

private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//移動スピード
	float speed_;
	
	//移動方向
	VECTOR moveDir_;
	
	//移動量
	VECTOR movePow_;
	
	//移動後の座標
	VECTOR movedPos_;

	//回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;
	
	//衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	//衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;
	
	//丸影
	int imgShadow_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	//球体
	std::unique_ptr<Sphere> sphere_;

	//足煙エフェクト
	int effectSmokeResId_;
	int effectSmokePlayId_;
	float stepFootSmoke_;	

	//フレームごとの移動値
	VECTOR moveDiff_;

	//ジャンプ量
	VECTOR jumpPow_;

	//ジャンプ判定
	bool isJump_;

	//ジャンプの入力受付時間
	float stepJump_;

	int chestFrmNo_;
	VECTOR chestPos_;

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	//状態遷移
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateStop(void);

	//更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateStop(void);

	//描画系
	void DrawShadow(void);

	//操作 
	void ProcessMove(void);

	void ProcessJump(void);

	//回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	//衝突判定
	void Collision(void);
	void CollisionCapsule(void);
	void CollisionGravity(void);

	// 移動量の計算
	void CalcGravityPow(void);

	//着地モーション終了
	bool IsEndLanding(void);

	//足煙エフェクト
	void EffectFootSmoke(void);
};
