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
		DEATH,		//死亡
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

	void DrawDead(void);
	void DrawVictory(void);

	void DrawResultString(std::wstring str);

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
	const Sphere& GetSphere(void) const;

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

	void Damage(float subHp) { hp_ -= subHp; }

	/// <summary>
	/// 回避中かどうかを取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:回避中　false:回避してない</returns>
	const bool& GetIsDodge(void)const  { return isDodge_; }
	const bool& GetIsParry(void)const  { return isParry_	; }

private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//HP
	float hp_;		//現在HP
	float maxHp_;	//最大HP

	//移動スピード
	float speed_;
	
	//移動方向
	VECTOR moveDir_;
	
	//移動量
	VECTOR movePow_;
	
	//移動後の座標
	VECTOR movedPos_;

	float stepWalk_;	//歩きモーション完了までの時間経過

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
	std::unique_ptr<Sphere> sphere_;

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
	
	//回避判定
	bool isDodge_;
	float stepDodge_;
	bool isDecelerate_;	//減速中かどうか

	//パリィ判定
	bool isParry_;
	float stepParry_;

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

	void SetHP(const float hp) { hp_ = hp; }
	void SetMaxHP(const float maxHp) { maxHp_ = maxHp; }

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

	//移動------------------------------------------------------------

	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="">WASDで移動する処理</param>
	void ProcessMove(void);

	void SetMoveSpeed(const float speed) { speed_ = speed; }

	//ジャンプ--------------------------------------------------------

	/// <summary>
	/// ジャンプ処理
	/// </summary>
	void ProcessJump(void);

	//回避------------------------------------------------------------

	/// <summary>
	/// 回避処理
	/// </summary>
	/// <param name=""></param>
	void ProcessDodge(void);

	//パリィ------------------------------------------------------------

	/// <summary>
	/// パリィ処理
	/// </summary>
	/// <param name=""></param>
	void ProcessParry(void);

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
	/// <returns>true: 着地モーションが終了した場合　false:それ以外</returns>
	bool IsEndLanding(void) const;

	/// <summary>
	/// 回避モーションが終了したかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:回避モーションが終了した場合　false:それ以外</returns>
	bool IsEndDodge(void) const;

	/// <summary>
	/// 足煙エフェクトの発生処理
	/// </summary>
	void EffectFootSmoke(void);

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);

	/// <summary>
	/// デバッグ用の描画処理
	/// </summary>
	void DebugDraw(void);

	int col_;
};
