#pragma once
#include <functional>
#include <map>
#include "../Libs/nlohmann/json.hpp"
#include "ActorBase.h"

class BarUI;
class AnimationController;
class Sphere;
class Player;
class EnemyBullet;

class Enemy : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,
		ENCOUNT,		//エンカウント(登場）
		TURN,			//振り向く
		ENCOUNT_FINISH,	//エンカウント演出終了
		FOLLOW,			//追跡
		WAIT,			//待機
		MOVE,			//移動(左右に)
		ATTACK_NEAR,	//近接攻撃
		SHOT_ONE,		//遠距離攻撃(１つずつ発射）
		SHOT_ALL,		//遠距離攻撃(全弾同時発射)
		CHARGE,			//チャージ
		ATTACK_CHARGE,	//ため攻撃
		BACKSTAB,		//バックスタブ(致命攻撃)され中
		DOWN,			//ダウン中
		DEAD,			//死
	};

	//アニメーションタイプ
	enum class ANIM_TYPE
	{
		IDLE,			//待機
		TURN,			//振り向き
		WALK,			//歩行
		WALK_RIGHT,		//右歩行
		WALK_LEFT,		//左歩行
		RUN,			//走行
		ATTACK_NEAR,	//近接攻撃
		MAGIC_ILDE,		//魔法待機
		CAST_SPELL,		//魔法詠唱
		ATTACK_FAR_ONE,	//遠距離攻撃(１つずつ発射）
		ATTACK_FAR_ALL,	//遠距離攻撃(全弾同時発射)
		ATTACK_CHARGE,	//ため攻撃
		DAMAGE,			//ダメージ
		BACKSTAB,		//バックスタブ(致命攻撃される)
		STAND_UP,		//起き上がり
		DOWN,			//ダウン
		DEATH,			//死
	};

	//コンストラクタ
	Enemy(Player& player);
	//デストラクタ
	~Enemy(void);

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
	/// HPバーの描画
	/// </summary>
	void DrawHPBar(void);

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(const STATE& state);

	/// <summary>
	/// 現在の状態を取得
	/// </summary>
	/// <returns>現在の状態</returns>
	const STATE& GetState(void)const { return state_; }

	/// <summary>
	/// ダウンしているかどうかを取得
	/// </summary>
	/// <returns>true:ダウン中　false:ダウンではない</returns>
	const bool GetIsDown(void)const { return state_ == STATE::DOWN; }

	/// <summary>
	/// 死亡しているかどうかを取得
	/// </summary>
	/// <returns>true:死亡　false:生存</returns>
	const bool GetIsDead(void)const;

	/// <summary>
	/// バックスタブ可能範囲を判断
	/// </summary>
	/// <returns>true:可能　false:不可能</returns>
	bool CheckBackstab(void);

	/// <summary>
	/// エンカウントしたかどうかを設定
	/// </summary>
	/// <param name="isEncount">ture:エンカウント済み、false：エンカウントしていない</param>
	const void SetIsEncount(const bool isEncount) { isEncount_ = isEncount; }

private:
	std::unique_ptr<BarUI> hpBar_;	//HPバー
	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;		//現在の状態
	STATE prevState_;	//遷移する前の状態

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	std::vector<std::unique_ptr<EnemyBullet>> bullets_;

	//近接攻撃用の当たり判定球
	std::unique_ptr<Sphere> sphereNear_;

	//当たり判定用カプセル
	Player& player_;

	//衝突チェック	
	VECTOR gravHitPosDown_;	//重力方向の当たり判定位置
	VECTOR gravHitPosUp_;	//重力と逆方向の当たり判定位置
	VECTOR movedPos_;		//移動後の位置
	VECTOR movePow_;		//移動量

	//移動方向
	VECTOR moveDir_;

	//体力
	float hp_;
	//最大体力
	float maxHp_;

	//回転
	Quaternion enemyRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過

	//ダウン
	bool isDown_;			//ダウン中かどうか	
	int hitCount_;			//ヒット回数(ダウンまでのカウント)
	float stepDownTime_;	//ダウン中の時間経過

	//状態毎のアクション完了フラグ
	bool isStepActioned_;

	//バックスタブ中かどうか
	bool isBackstab_;

	//チャージ攻撃済みかどうか
	bool isChargeAtk_;
	//チャージ量
	float chargeRadius_;

	//エンカウント済みかどうか
	bool isEncount_;

	//チャージ中のエフェクト
	int effectChargeResId_;		//エフェクトリソースID
	int effectChargePlayId_;	//エフェクト再生ID
	//チャージ攻撃時のエフェクト
	int effectChargeAtkResId_;	//エフェクトリソースID
	int effectChargeAtkPlayId_;	//エフェクト再生ID


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
	/// HPを設定
	/// </summary>
	/// <param name="hp">HP</param>
	void SetHP(const float hp) { hp_ = hp; }

	/// <summary>
	/// 最大HPを設定
	/// </summary>
	/// <param name="maxHp">最大HP</param>
	void SetMaxHP(const float maxHp) { maxHp_ = maxHp; }

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：ENCOUNT
	/// </summary>
	void ChangeStateEncount(void);
	/// <summary>
	/// 状態遷移：TURN
	/// </summary>
	void ChangeStateTurn(void);
	/// <summary>
	/// 状態遷移：ENCOUNT_FINISH
	/// </summary>
	void ChangeStateEncountFinish(void);
	/// <summary>
	/// 状態遷移：WAIT
	/// </summary>
	void ChangeStateWait(void);
	/// <summary>
	/// 状態遷移：FOLLOW
	/// </summary>
	void ChangeStateFollow(void);
	/// <summary>
	/// 状態遷移：MOVE
	/// </summary>
	void ChangeStateMove(void);
	/// <summary>
	/// 状態遷移：ATTACK
	/// </summary>
	void ChangeStateAttackNear(void);
	/// <summary>
	/// 状態遷移：SHOT_ONE
	/// </summary>
	void ChangeStateShotOne(void);
	/// <summary>
	/// 状態遷移：SHOT_ALL
	/// </summary>
	void ChangeStateShotAll(void);
	/// <summary>
	/// 状態遷移：CHARGE
	/// </summary>
	void ChangeStateCharge(void);
	/// <summary>
	/// 状態遷移：ATTACK_CHARGE
	/// </summary>
	void ChangeStateAttackCharge(void);
	/// <summary>
	/// 状態遷移：BACKSTAB
	/// </summary>
	void ChangeStateBackstab(void);
	/// <summary>
	/// 状態遷移：DOWN
	/// </summary>
	void ChangeStateDown(void);
	/// <summary>
	/// 状態遷移：DEAD
	/// </summary>
	void ChangeStateDead(void);

	//更新ステップ--------------------------------------------------------
	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void);
	/// <summary>
	/// 更新：ENCOUNT
	/// </summary>
	void UpdateEncount(void);
	/// <summary>
	/// 更新：TURN
	/// </summary>
	void UpdateTurn(void);
	/// <summary>
	/// 更新：WAIT
	/// </summary>
	void UpdateEncountFinish(void);
	/// <summary>
	/// 更新：WAIT
	/// </summary>
	void UpdateWait(void);
	/// <summary>
	/// 更新：FOLLOW
	/// </summary>
	void UpdateFollow(void);
	/// <summary>
	/// 更新：MOVE
	/// </summary>
	void UpdateMove(void);
	/// <summary>
	/// 更新：ATTACK_NEAR
	/// </summary>
	void UpdateAttackNear(void);
	/// <summary>
	/// 更新：SHOT_ONE
	/// </summary>
	void UpdateShotOne(void);
	/// <summary>
	/// 更新：SHOT_ALL
	/// </summary>
	void UpdateShotAll(void);
	/// <summary>
	/// 更新：CHARGE
	/// </summary>
	void UpdateCharge(void);
	/// <summary>
	/// 更新：CHARGE_ATTACK
	/// </summary>
	void UpdateChargeAttack(void);
	/// <summary>
	/// 更新：BACKSTAB
	/// </summary>
	void UpdateBackstab(void);
	/// <summary>
	/// 更新：DOWN
	/// </summary>
	void UpdateDown(void);
	/// <summary>
	/// 更新：DEAD
	/// </summary>
	void UpdateDead(void);


	/// <summary>
	/// ダメージを与える
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	void Damage(const float damage);

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(void);

	/// <summary>
	/// プレイヤーとの距離をチェックする
	/// </summary>
	/// <param name=""></param>
	/// <returns>プレイヤーと敵の距離</returns>
	float CheckPlayerDistance(void);

	bool IsCastSpell(void);

	/// <summary>
	/// プレイヤーを追従する処理
	/// </summary>
	/// <param name=""></param>
	void FollowPlayer(VECTOR& pos);

	//衝突判定--------------------------------------------------------
	
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

	//回転--------------------------------------------------------

	/// <summary>
	/// 目標回転角度の設定
	/// </summary>
	/// <param name="rotRad">目標回転角度</param>
	void SetGoalRotate(double rotRad);

	/// <summary>
	/// 回転処理
	/// </summary>
	void Rotate(void);

	/// <summary>
	/// プレイヤーの方向へ回転する処理
	/// </summary>
	/// <param name=""></param>
	void RotateToPlayer(void);

	//弾--------------------------------------------------------
	
	/// <summary>
	/// 弾の生成
	/// </summary>
	/// <param name="createNum">生成する数</param>
	void CreateBullet(const int createNum);

	/// <summary>
	/// 生成した弾全てが準備状態かチェックする
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:全て準備状態　false:未準備</returns>
	bool CheckBulletReady(void);

	/// <summary>
	/// 生成した弾全てが破棄状態かチェックする
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:全て破棄状態　false:未破棄</returns>
	bool CheckBulletDestroy(void);

	/// <summary>
	/// チャージエフェクトの再生
	/// </summary>
	void EffectCharge(void);

	/// <summary>
	/// チャージ攻撃エフェクトの再生
	/// </summary>
	/// <param name=""></param>
	void EffectChargeAtk(void);

	/// <summary>
	/// Jsonデータ取得
	/// </summary>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json GetJsonData(void)const;

	void DebugImGui(void);

	//状態を遷移させる用の時間管理変数
	float stateStep_;
	//方向転換用の時間管理変数
	float changeDirStep_;
};

