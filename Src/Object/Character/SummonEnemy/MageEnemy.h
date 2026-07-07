#pragma once
#include "../Player.h"
#include "SummonEnemyBase.h"

class ModelRenderer;
class ModelMaterial;
class AnimationController;
class Capsule;
class Sphere;
class EnemyBullet;
class HPBar;

class MageEnemy : public SummonEnemyBase
{
public:
	//攻撃の種類
	enum class ATTACK
	{
		SHOT,	//射撃
		MAX
	};

	//アニメーションの種類
	enum class ANIM_TYPE
	{
		IDLE,		//待機
		ATTACK,		//攻撃
		DAMAGE,		//ダメージを受ける
	};

	//コンストラクタ
	MageEnemy(Player& player);
	//デストラクタ
	~MageEnemy(void)override;

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
	/// UI描画
	/// </summary>
	void DrawUI(void);

private:
	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//マテリアルとレンダー
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;
	//球体
	std::unique_ptr<Sphere> sphere_;

	//弾
	std::unique_ptr<EnemyBullet> bullet_;

	//HPバーUI
	std::unique_ptr<HPBar> hpBar_;

	//弾の発射間隔
	float bulletInterval_;	
	//方向転換用の時間管理変数
	float changeDirStep_;

	//移動関係
	VECTOR movedPos_;		//移動後の位置
	VECTOR movePow_;		//移動量
	VECTOR moveDir_;		//移動方向

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void)override;

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void)override;

	/// <summary>
	/// マテリアルの初期化
	/// </summary>
	void InitMaterial(void)override;

	/// <summary>
	/// 当たり判定の初期化
	/// </summary>
	void InitCollider(void)override;

	/// <summary>
	/// UIの初期化
	/// </summary>
	void InitUI(void)override;
	void InitSound(void);
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
	/// <summary>
	/// 状態遷移：DAMAGE
	/// </summary>
	void ChangeStateDamage(void);
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
	/// <summary>
	/// 更新：DAMAGE
	/// </summary>
	void UpdateDamage(void);
	/// <summary>
	/// 更新：DEAD
	/// </summary>
	void UpdateDead(void);

	/// <summary>
	/// 移動
	/// </summary>
	void Move(void);

	//攻撃---------------------------------------------------------

	/// <summary>
	/// 射撃処理
	/// </summary>
	void Shoot(void);
};

