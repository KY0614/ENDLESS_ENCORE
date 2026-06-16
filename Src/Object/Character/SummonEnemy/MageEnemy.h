#pragma once
#include "../Player.h"
#include "SummonEnemyBase.h"

class ModelRenderer;
class ModelMaterial;
class AnimationController;
class Capsule;
class Sphere;
class EnemyBullet;

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

	float bulletInterval_;	//弾の発射間隔

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

