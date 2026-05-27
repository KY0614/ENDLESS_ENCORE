#pragma once
#include <map>
#include <functional>
#include "../Common/ActorBase.h"
#include "Player.h"

class SummonEnemy : public ActorBase
{
public:
	//クラスで分けたほうがいいかも
	//召喚する敵の種類
	enum class TYPE
	{
		NONE,
		ATTACK_NEAR,//近距離攻撃タイプ
		ATTACK_FAR,	//遠距離攻撃タイプ
	};

	enum class STATE
	{
		NONE,
		SUMMON,	//召喚中
		MOVE,	//移動中
		LOOK,	//プレイヤーの方を向く
	};;

	//コンストラクタ
	SummonEnemy(Player& player);
	//デストラクタ
	~SummonEnemy(void) override;

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

	void Summon(void);

	/// <summary>
	/// 召喚する敵の種類を設定
	/// </summary>
	/// <param name="type">召喚する種類</param>
	void SetSummonType(const TYPE& type) { type_ = type; }

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

private:

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	STATE state_;		//現在の状態

	//召喚する敵の種類
	TYPE type_;
	
	//プレイヤーの参照
	Player& player_;

	//回転
	Quaternion enemyRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過

	bool isSummoned_;	//召喚完了フラグ

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state"></param>
	void ChangeState(const STATE& state);

	void ChangeStateNone(void);
	void ChangeStateSummon(void);
	void ChangeStateMove(void);
	void ChangeStateLook(void);

	void UpdateNone(void);
	void UpdateSummon(void);
	void UpdateMove(void);
	void UpdateLook(void);

	/// <summary>
	/// 召喚完了
	/// </summary>
	void IsSummoned(void) { isSummoned_ = true; }

	void SetGoalRotate(double rotRad);

	void Rotate(void);

	void Rotate2Player(void);
};

