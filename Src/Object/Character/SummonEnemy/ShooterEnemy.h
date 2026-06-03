#pragma once
#include "../Player.h"
#include "SummonEnemyBase.h"

class EnemyBullet;

class ShooterEnemy : public SummonEnemyBase
{
public:
	//攻撃の種類
	enum class ATTACK
	{
		SHOT,	//射撃
		MAX
	};

	//コンストラクタ
	ShooterEnemy(Player& player);
	//デストラクタ
	~ShooterEnemy(void)override;

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

	//弾
	std::unique_ptr<EnemyBullet> bullet_;

	float bulletInterval_;	//弾の発射間隔

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void)override;

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：SUMMON
	/// </summary>
	void ChangeStateSummon(void);
	/// <summary>
	/// 状態遷移：MOVE
	/// </summary>
	void ChangeStateMove(void);
	/// <summary>
	/// 状態遷移：ATTACK
	/// </summary>
	void ChangeStateAttack(void);

	//更新ステップ--------------------------------------------------------
	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void);
	/// <summary>
	/// 更新：SUMMON
	/// </summary>
	void UpdateSummon(void);
	/// <summary>
	/// 更新：MOVE
	/// </summary>
	void UpdateMove(void);
	/// <summary>
	/// 更新：ATTACK
	/// </summary>
	void UpdateAttack(void);

	//攻撃---------------------------------------------------------

	/// <summary>
	/// 射撃処理
	/// </summary>
	void Shoot(void);
};

