//#pragma once
#include <functional>
#include <map>
#include "ActorBase.h"

class AnimationController;
class ModelRenderer;
class ModelMaterial;

class EncountEnemy : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,			//何もしない
		ENCOUNT,		//エンカウント(登場）
		TURN,			//振り向く
		CAST_SPELL,		//魔法詠唱
		ATTACK_PLAYER,	//プレイヤーへ攻撃
		ENCOUNT_FINISH,	//エンカウント演出終了
	};

	//アニメーションタイプ
	enum class ANIM_TYPE
	{
		IDLE,			//待機
		TURN,			//振り向き
		WALK,			//歩行
	};

	//コンストラクタ
	EncountEnemy(void);
	//デストラクタ
	~EncountEnemy(void);

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
	/// エンカウント演出開始
	/// </summary>
	void EncountStart(void);

	/// <summary>
	/// 振り向く
	/// </summary>
	void Turn(void);

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

private:

	//マテリアルとレンダー
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;		//現在の状態

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//エンカウントしているかどうか
	bool isEncount_;		

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	/// <summary>
	/// マテリアルの初期化
	/// </summary>
	void InitMaterial(void);

	/// <summary>
	/// Jsonデータ取得
	/// </summary>
	/// <returns>Jsonデータ</returns>
	const nlohmann::json GetJsonData(void)const;

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
	/// 状態遷移：CAST_SPELL
	/// </summary>
	void ChangeStateCastSpell(void);
	/// <summary>
	/// 状態遷移：ATTACK_PLAYER
	/// </summary>
	void ChangeStateAttackPlayer(void);
	/// <summary>
	/// 状態遷移：ENCOUNT_FINISH
	/// </summary>
	void ChangeStateEncountFinish(void);

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
	/// 更新：ENCOUNT_FINISH
	/// </summary>
	void UpdateEncountFinish(void);
};

