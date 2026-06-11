#pragma once
#include "../Player.h"
#include "SummonEnemyBase.h"

class ModelRenderer;
class ModelMaterial;
class AnimationController;
class Capsule;
class Sphere;

class FighterEnemy : public SummonEnemyBase
{
public:
	//攻撃の種類
	enum class ATTACK
	{
		SLASH,	//斬撃
		MAX
	};

	enum class ANIM_TYPE
	{
		IDLE,		//待機
		MOVE,		//移動
		ATTACK,		//攻撃
	};

	//コンストラクタ
	FighterEnemy(Player& player);
	//デストラクタ
	~FighterEnemy(void) override;

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
	/// 攻撃中かどうか
	/// </summary>
	/// <returns>true:攻撃中　false:攻撃中じゃない</returns>
	const bool GetIsAttack(void) const { return isAttack_; }

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

	//状態遷移のタイマー
	float stateTimer_;		

	//攻撃中かどうか
	bool isAttack_;

	//攻撃が当たったかどうか
	bool isHitAttack_;

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void)override;

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	/// <summary>
	/// マテリアルの初期化
	/// </summary>
	void InitMaterial(void);

	/// <summary>
	/// 当たり判定の初期化
	/// </summary>
	void InitCollider(void);

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

	//移動---------------------------------------------------------

	/// <summary>
	/// 追従処理（追いかける)
	/// </summary>
	void FollowMove(void);


	bool IsEndAttack(void);
};

