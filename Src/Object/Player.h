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
	static constexpr float SPEED_MOVE = 4.0f;
	static constexpr float SPEED_RUN = 8.0f;

	static constexpr float ACCEL_MOVE = 3.0f;	//加速度
	static constexpr float DECEL_MOVE = 0.1f;	//減速度

	//回転完了までの時間
	static constexpr float TIME_ROT = 0.3f;

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
		IDLE,		//通常
		WALK,		//歩き
		RUN,		//走り
		JUMP,		//ジャンプ
		PARRY,		//パリィ
		DODGE,		//回避
		USE_ITEM,	//アイテム使用
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

	/// <summary>
	/// 状態がPLAYかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:状態がPLAYの場合　false:それ以外</returns>
	bool IsPlay(void) const;

	/// <summary>
	/// 状態を変更する
	/// </summary>
	/// <param name="state">変更する状態</param>
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

	////加速度、減速度
	//VECTOR velocity_;	//現在の速度
	//VECTOR accelation_;	//加速度

	//回転
	Quaternion playerRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過
	
	//衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	//衝突チェック	
	VECTOR gravHitPosDown_;	//重力方向の当たり判定位置
	VECTOR gravHitPosUp_;	//重力と逆方向の当たり判定位置
	
	//丸影
	int imgShadow_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	//足煙エフェクト
	int effectSmokeResId_;	//エフェクトリソースID
	int effectSmokePlayId_;	//エフェクト再生ID
	float stepFootSmoke_;	//足煙エフェクト発生までの時間経過

	//フレームごとの移動値
	VECTOR moveDiff_;

	//ジャンプ量
	VECTOR jumpPow_;

	//ジャンプ判定
	bool isJump_;

	//無限ジャンプ
	bool isJumpUnlimited_;

	//ジャンプ用
	VECTOR jumpVelocity_;	//現在の速度

	//ジャンプの入力受付時間
	float stepJump_;

	int chestFrmNo_;
	VECTOR chestPos_;

	//回避判定
	bool isDodge_;

	float stepDodge_;

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	/// <param name=""></param>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：PLAY
	/// </summary>
	/// <param name=""></param>
	void ChangeStatePlay(void);
	/// <summary>
	/// 状態遷移：DEAD
	/// </summary>
	/// <param name=""></param>
	void ChangeStateDead(void);

	//更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateDead(void);

	/// <summary>
	/// 影の描画処理
	/// </summary>
	void DrawShadow(void);

	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="">WASDで移動する処理</param>
	void ProcessMove(void);

	//ジャンプ--------------------------------------------------------

	/// <summary>
	/// ジャンプ処理（真上にジャンプ）
	/// </summary>
	void ProcessJump(void);

	/// <summary>
	/// ジャンプ処理
	/// </summary>
	/// <param name="">スペースキー押下でジャンプする処理</param>
	void ProcessJumpTest(void);

	//回避------------------------------------------------------------

	/// <summary>
	/// 回避処理
	/// </summary>
	/// <param name=""></param>
	void ProcessDodge(void);

	//モデルの回転----------------------------------------------------

	/// <summary>
	/// 目標回転角度の設定
	/// </summary>
	/// <param name="rotRad"></param>
	void SetGoalRotate(double rotRad);

	/// <summary>
	/// 回転処理
	/// </summary>
	void Rotate(void);

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

	/// <summary>
	/// 移動量の計算
	/// </summary>
	void CalcGravityPow(void);

	/// <summary>
	/// 着地モーションが終了したかどうか
	/// </summary>
	/// <returns>true: 着地モーションが終了した場合</returns>
	bool IsEndLanding(void) const;

	bool IsEndDodge(void) const;

	/// <summary>
	/// 足煙エフェクトの発生処理
	/// </summary>
	void EffectFootSmoke(void);

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);
};
