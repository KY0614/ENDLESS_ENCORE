#pragma once
#include <map>
#include <functional>
#include "../../Common/ActorBase.h"

class Player;

class SummonEnemyBase : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,		
		SUMMON,	//召喚
		MOVE,	//移動
		ATTACK,	//攻撃
		DAMAGE,	//ダメージ	
		DEAD,	//死亡
	};
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="player">プレイヤーの参照</param>
	SummonEnemyBase(Player& player);
	//デストラクタ
	virtual ~SummonEnemyBase(void) override;

	/// <summary>
	///	初期化
	/// </summary>
	virtual void Init(void) override = 0;

	/// <summary>
	///	更新処理
	/// </summary>
	virtual void Update(void) override = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw(void) override = 0;

	/// <summary>
	/// 召喚する
	/// </summary>
	void Summon(void) { ChangeState(STATE::SUMMON); }

	/// <summary>
	/// 召喚する位置を設定
	/// </summary>
	/// <param name="pos">位置</param>
	void SetSummonPos(const VECTOR& pos) { transform_.pos = pos; }

	/// <summary>
	/// 召喚が完了したかどうか
	/// </summary>
	/// <returns>ture:完了　false:未完了</returns>
	const bool GetIsSummoned(void) const { return isSummoned_; }

	void Dead(void) { hp_ = 0.0f; }

protected:

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//現在の状態
	STATE state_;

	//プレイヤーの参照
	Player& player_;

	//回転
	Quaternion enemyRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過

	//召喚完了したかどうか
	bool isSummoned_;

	//追従速度
	float followSpeed_;		

	//体力
	float hp_;		//現在体力
	float maxHp_;	//最大体力

	bool isAlive_;	//生存しているかどうか

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	virtual void Init3DModel(void) = 0;

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	virtual void InitAnimation(void) = 0;

	/// <summary>
	/// マテリアルの初期化
	/// </summary>
	virtual void InitMaterial(void) = 0;

	/// <summary>
	/// 当たり判定の初期化
	/// </summary>
	virtual void InitCollider(void) = 0;
	
	/// <summary>
	/// UI初期化
	/// </summary>
	virtual void InitUI(void) = 0;

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state"></param>
	void ChangeState(const STATE& state);

	/// <summary>
	/// 現在体力を設定する　
	/// </summary>
	/// <param name="hp">現在体力</param>
	void SetHP(const float hp) { hp_ = hp; }

	/// <summary>
	/// 最大体力を設定する　
	/// </summary>
	/// <param name="maxHp">最大体力</param>
	void SetMaxHP(const float maxHp) { maxHp_ = maxHp; }

	/// <summary>
	/// 召喚完了
	/// </summary>
	void IsSummoned(void) { isSummoned_ = true; }

	/// <summary>
	/// 追従速度設定
	/// </summary>
	/// <param name="speed">追従速度</param>
	void SetFollowSpeed(const float speed) { followSpeed_ = speed; }

	//回転---------------------------------------------------------

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
	void Rotate2Player(void);
};

