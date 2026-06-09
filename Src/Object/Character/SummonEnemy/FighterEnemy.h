#pragma once
#include "../Player.h"
#include "SummonEnemyBase.h"

class ModelRenderer;
class ModelMaterial;
class AnimationController;

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

private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//マテリアルとレンダー
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;

	float stateTimer_;		//状態遷移のタイマー

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void)override;

	void InitAnimation(void);

	void InitMaterial(void);

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

