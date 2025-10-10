#pragma once
#include "ShotBase.h"

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
		DETSTROY,
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

	/// <summary>
	/// 状態を取得
	/// </summary>
	/// <returns>現在の状態</returns>
	const STATE& GetState(void)const { return state_; }

	/// <summary>
	/// 準備状態に設定
	/// </summary>
	void SetStateReady(void) { state_ = STATE::READY; isAlive_ = true; }

	/// <summary>
	/// 生存状態を設定
	/// </summary>
	/// <param name="isAlive">true:生存中　false:生存してない</param>
	void SetIsAlive(const bool isAlive) { isAlive_ = isAlive; }

	/// <summary>
	/// ローカル座標を設定
	/// </summary>
	/// <param name="pos">指定するローカル座標</param>
	void SetLocalPos(const VECTOR localPos);

	/// <summary>
	/// 発射する
	/// </summary>
	void Shot(void);

	/// <summary>
	/// リセットする
	/// </summary>
	void Reset(void);

private:
	//状態
	STATE state_;

	//親のモデル情報
	Transform& parentTran_;

	//生存状態
	bool isAlive_;

	//球体
	std::unique_ptr<Sphere> sphere_;
	
	//弾の移動処理
	void Move(void);

	void Rotate(void);

	/// <summary>
	/// 状態を変更する
	/// </summary>
	/// <param name="state">指定する状態</param>
	void ChangeState(const STATE state) { state_ = state; }

};

