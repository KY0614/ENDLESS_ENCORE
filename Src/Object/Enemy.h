#pragma once
#include <functional>
#include <map>
#include<medparam.h>
#include "ActorBase.h"

class AnimationController;
class Sphere;
class Player;

class Enemy : public ActorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,
		FOLLOW,
		MOVE,			//
		ATTACK_NEAR,	//
		ATTACK_FAR,		//
		DOWN,			//
		DEAD,			//
	};

	//アニメーションタイプ
	enum class ANIM_TYPE
	{
		IDLE,
		MOVE,
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
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(const STATE state);

private:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	//近接攻撃用の当たり判定球
	std::unique_ptr<Sphere> sphereNear_;
	//遠距離攻撃用の当たり判定球
	std::vector<std::unique_ptr<Sphere>> spheresFar_; 

	//当たり判定用カプセル
	Player& player_;

	float hp_;

	bool isAttackedNear_;

	//回転
	Quaternion enemyRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過

	float currentAngle_;        // プレイヤーを中心とした現在の角度 (ラジアン)
	float circlingSpeedRad_;    // 円周移動の角速度 (ラジアン/秒)

	bool isDown_;
	float stepDownTime_;

	float modelCol_[3];

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name=""></param>
	void Move(void);

	/// <summary>
	/// プレイヤーとの距離をチェックする
	/// </summary>
	/// <param name=""></param>
	/// <returns>プレイヤーと敵の距離</returns>
	float CheckPlayerDistance(void);

	/// <summary>
	/// プレイヤーを追従する処理
	/// </summary>
	/// <param name=""></param>
	void FollowPlayer(VECTOR& pos);

	//回転--------------------------------------------------------

	/// <summary>
	/// 目標回転角度の設定
	/// </summary>
	/// <param name="rotRad"></param>
	void SetGoalRotate(double rotRad);

	/// <summary>
	/// 回転処理
	/// </summary>
	void Rotate(void);

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	/// <param name=""></param>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：FOLLOW
	/// </summary>
	/// <param name=""></param>
	void ChangeStateFollow(void);
	/// <summary>
	/// 状態遷移：MOVE
	/// </summary>
	/// <param name=""></param>
	void ChangeStateMove(void);
	/// <summary>
	/// 状態遷移：ATTACK
	/// </summary>
	/// <param name=""></param>
	void ChangeStateAttackNear(void);
	/// <summary>
	/// 状態遷移：ATTACK
	/// </summary>
	/// <param name=""></param>
	void ChangeStateAttackFar(void);
	/// <summary>
	/// 状態遷移：DOWN
	/// </summary>
	/// <param name=""></param>
	void ChangeStateDown(void);
	/// <summary>
	/// 状態遷移：DEAD
	/// </summary>
	/// <param name=""></param>
	void ChangeStateDead(void);

	//更新ステップ
	void UpdateNone(void);
	void UpdateFollow(void);
	void UpdateMove(void);
	void UpdateAttackNear(void);
	void UpdateAttackFar(void);
	void UpdateDown(void);
	void UpdateDead(void);

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);

#ifdef _DEBUG

	float stateStep_;

	int col_;

#endif // _DEBUG

};

