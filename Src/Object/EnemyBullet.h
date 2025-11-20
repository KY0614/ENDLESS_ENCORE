#pragma once
#include <functional>
#include <map>
#include "ShotBase.h"
#include "PointLight.h"
#include "SpotLight.h"

class Sphere;

class EnemyBullet :  public ShotBase
{
public:
	//状態
	enum class STATE
	{
		NONE,
		READY,
		SHOT,
		REVERSE,
		DESTROY,
	};

	//コンストラクタ
	EnemyBullet(Transform& parent);
	//デストラクタ
	~EnemyBullet(void);

	//初期化
	void Init(void)override;
	//更新
	void Update(void)override;
	//描画
	void Draw(void)override;

	//破棄
	void Destroy(void);

	/// <summary>
	/// 発射状態かどうかを取得
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:発射中　false:発射してない</returns>
	const bool& CheckStateShot(void)const { return state_ == STATE::SHOT; }
	const bool& CheckStateReverse(void)const { return state_ == STATE::REVERSE; }

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

	const VECTOR& GetLocalPos(void)const { return localPos_; }

	/// <summary>
	/// 準備状態に設定
	/// </summary>
	void SetStateReady(void) { state_ = STATE::READY; isAlive_ = true; }

	/// <summary>
	/// 反射状態に設定
	/// </summary>
	void SetStateReverse(void) { state_ = STATE::REVERSE; isAlive_ = true; }

	/// <summary>
	/// 生存状態を設定
	/// </summary>
	/// <param name="isAlive">true:生存中　false:生存してない</param>
	void SetIsAlive(const bool isAlive) { isAlive_ = isAlive; }

	/// <summary>
	/// ターゲット座標を設定
	/// </summary>
	/// <param name="targetPos">指定するターゲット座標</param>
	void SetTargetPos(const VECTOR targetPos) { targetPos_ = targetPos; }

	/// <summary>
	/// オフセット座標を設定
	/// </summary>
	/// <param name="pos">指定するローカル座標</param>
	void SetOffsetPos(const VECTOR offset);

	/// <summary>
	/// ローカル座標を設定
	/// </summary>
	/// <param name="pos">指定するローカル座標</param>
	void SetLocalPos(const VECTOR local);

	/// <summary>
	/// 座標を設定
	/// </summary>
	/// <param name="pos">座標</param>
	void SetPos(const VECTOR pos) { transform_.pos = pos; }

	/// <summary>
	/// 発射する
	/// </summary>
	void Shot(void);

	/// <summary>
	/// リセットする
	/// </summary>
	void Reset(const Transform& transform);

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(const STATE state) { state_ = state; }

private:
	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//親のモデル情報
	Transform& parentTran_;

	VECTOR localPos_;
	VECTOR offsetPos_;
	VECTOR targetPos_;

	//生存状態
	bool isAlive_;

	//球体
	std::unique_ptr<Sphere> sphere_;
	
	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(void);

	/// <summary>
	/// 相対座標を親の回転に同期させる
	/// </summary>
	void SyncParentRotate(void);
};

