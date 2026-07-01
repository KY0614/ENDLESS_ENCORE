#pragma once
#include <functional>
#include <map>
#include "../Common/ActorBase.h"

class Sphere;

class BulletBase : public ActorBase
{
public:

	//弾の速度
	static constexpr float BULLET_SPEED = 20.0f;
	//弾の生存時間
	static constexpr float LIFE_TIME = 8.0f;
	//SEの音量
	static constexpr int FIRE_SE_VOLUME = 70;
	//弾の当たり判定用の球体の半径
	static constexpr float BULLET_RADIUS = 20.0f;

	//状態
	enum class STATE
	{
		NONE,	//初期状態
		READY,	//準備
		SHOT,	//発射
		REVERSE,//反射
		DESTROY,//破棄
	};

	//コンストラクタ
	BulletBase(Transform& parent);
	//デストラクタ
	virtual ~BulletBase(void)override;

	/// <summary>
	///	初期化
	/// </summary>
	virtual void Init(void) override;

	/// <summary>
	///	更新処理
	/// </summary>
	virtual void Update(void) override;

	/// <summary>
	/// 状態を取得
	/// </summary>
	/// <returns>現在の状態</returns>
	const STATE& GetState(void)const { return state_; }

	/// <summary>
	/// 当たり判定用の球体を取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	const Sphere& GetSphere(void)const { return *sphere_; }

	/// <summary>
	/// 破棄状態へ遷移
	/// </summary>
	/// <param name=""></param>
	void SetStateDestroy(void) { ChangeState(STATE::DESTROY); }

	/// <summary>
	/// 準備状態へ遷移
	/// </summary>
	void SetStateReady(void) { ChangeState(STATE::READY); }

	/// <summary>
	/// 発射する
	/// </summary>
	void SetStateShot(void) { ChangeState(STATE::SHOT); }

	/// <summary>
	/// 反射状態へ遷移
	/// </summary>
	void SetStateReverse(void) { ChangeState(STATE::REVERSE); }

	/// <summary>
	/// 速度を設定
	/// </summary>
	/// <param name="speed">速度</param>
	void SetSpeed(const float speed) { speed_ = speed; }

	/// <summary>
	/// 生存状態を設定
	/// </summary>
	/// <param name="isAlive">true:生存中　false:生存してない</param>
	void SetIsAlive(const bool isAlive) { isAlive_ = isAlive; }

	/// <summary>
	/// オフセット座標を設定
	/// </summary>
	/// <param name="pos">指定するローカル座標</param>
	void SetOffsetPos(const VECTOR& offset) { offsetPos_ = offset; }

	/// <summary>
	/// ローカル座標を設定
	/// </summary>
	/// <param name="pos">指定するローカル座標</param>
	void SetLocalPos(const VECTOR& local) { localPos_ = local; }
	/// <summary>
	/// 座標を設定
	/// </summary>
	/// <param name="pos">座標</param>
	void SetPos(const VECTOR& pos) { transform_.pos = pos; }

	/// <summary>
	/// リセットする
	/// </summary>
	void Reset(const Transform& transform);

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(const STATE& state);

protected:
	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//球体
	std::unique_ptr<Sphere> sphere_;

	//親のモデル情報
	Transform& parentTran_;

	//速度
	float speed_;

	//生存時間
	float lifeTime_;

	//生存状態
	bool isAlive_;

	//座標情報	
	VECTOR localPos_;	//ローカル座標
	VECTOR offsetPos_;	//オフセット座標

	//炎エフェクト
	int effectFireResId_;	//エフェクトリソースID
	int effectFirePlayId_;	//エフェクト再生ID

	/// <summary>
	/// モデルの基本情報を初期化
	/// </summary>
	/// <param name=""></param>
	virtual void InitTransform(void) = 0;

	/// <summary>
	/// 当たり判定用のコライダーを初期化
	/// </summary>
	virtual void InitCollider(void) = 0;

	/// <summary>
	/// サウンド初期化
	/// </summary>
	void InitSound(void);

	/// <summary>
	/// 生存時間を設定
	/// </summary>
	/// <param name="time">生存時間</param>
	void SetLifeTime(const float time) { lifeTime_ = time; }

	//状態遷移処理--------------------------------------------------------

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	virtual void ChangeStateNone(void) = 0;
	/// <summary>
	/// 状態遷移：READY
	/// </summary>
	virtual void ChangeStateReady(void) = 0;
	/// <summary>
	/// 状態遷移：SHOT
	/// </summary>
	virtual void ChangeStateShot(void) = 0;
	/// <summary>
	/// 状態遷移：REVERSE
	/// </summary>
	virtual void ChangeStateReverse(void) = 0;
	/// <summary>
	/// 状態遷移：DESTROY
	/// </summary>
	virtual void ChangeStateDestroy(void) = 0;

	//状態更新処理------------------------------------------------------

	/// <summary>
	/// 更新：NONE
	/// </summary>
	virtual void UpdateNone(void) = 0;
	/// <summary>
	/// 更新：READY
	/// </summary>
	virtual void UpdateReady(void) = 0;
	/// <summary>
	/// 更新：SHOT
	/// </summary>
	virtual void UpdateShot(void) = 0;
	/// <summary>
	///	 更新：REVERSE
	/// </summary>
	virtual void UpdateReverse(void) = 0;
	/// <summary>
	/// 更新：DESTROY
	/// </summary>
	virtual void UpdateDestroy(void) = 0;

	//当たり判定処理------------------------------------------------------

	/// <summary>
	/// 登録されたコライダーと球体の当たり判定
	/// </summary>
	void CollisionSphere(void);

	//座標制御------------------------------------------------------

	/// <summary>
	/// 移動処理
	/// </summary>
	virtual void Move(void) = 0;

	/// <summary>
	/// 相対座標を親の回転に同期させる
	/// </summary>
	void SyncParentRotate(void);

	//エフェクト------------------------------------------------------

	/// <summary>
	/// 炎エフェクトの再生
	/// </summary>
	void EffectFire(void);

	/// <summary>
	/// 炎エフェクトの位置同期
	/// </summary>
	void EffectFirePositionSync(void);
};

