#pragma once
#include<vector>
#include<memory>
#include<map>
#include "../Common/Transform.h"

class ColliderBase;

class ActorBase
{
public:

	//重力
	static constexpr float GRAVITY = 15.0f;				//重力加速度
	static constexpr float GRAVITY_SCALE = 1.0f;		//重力の減衰率

	//コンストラクタ
	ActorBase(void);

	//デストラクタ
	virtual ~ActorBase(void);

	virtual void Init(void);
	virtual void Update(void);
	virtual void Draw(void);

	//モデルの基本情報を取得する
	const Transform& GetTransform(void) const;

	/// <summary>
	/// 指定されたフレーム名(ボーン)に対応する座標を取得する
	/// </summary>
	/// <param name="frameName">フレームの名前</param>
	/// <returns>フレームの座標</returns>
	const VECTOR GetFramePos(const std::wstring& frameName) const;

	/// <summary>
	/// 重力の強さを取得する
	/// </summary>
	/// <returns>重力の強さ</returns>
	float GetGravityPower(void) const { return GRAVITY * GRAVITY_SCALE; }

	/// <summary>
	/// 衝突判定に用いられるコライダーを追加する
	/// </summary>
	/// <param name="collider">コライダー情報</param>
	void AddCollider(std::weak_ptr<Collider> collider);

	// 衝突対象となるコライダを登録
	void AddHitCollider(const std::weak_ptr<ColliderBase> hitCollider);

	// 衝突対象となるコライダをクリア
	void ClearHitCollider(void);

	/// <summary>
	/// 自身の衝突情報を取得する
	/// </summary>
	/// <returns>自身の衝突情報</returns>
	const std::map<int, std::shared_ptr<ColliderBase>> GetOwnColliders(void) const { return ownColliders_; }

	/// <summary>
	/// 特定の自身の衝突情報を取得する
	/// </summary>
	/// <param name="key">衝突情報のキー</param>
	/// <returns>指定されたキーに対応する自身の衝突情報</returns>
	const std::weak_ptr<ColliderBase> GetOwnCollider(int key) const;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name=""></param>
	virtual void UpdateImGui(void) {}

	/// <summary>
	/// Jsonデータのパラメータを保存する
	/// </summary>
	/// <param name=""></param>
	virtual void SaveParameter(void) {}

protected:
	//モデル制御の基本情報
	Transform transform_;

	//衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	//自身の衝突情報
	std::map<int, std::shared_ptr<ColliderBase>> ownColliders_;

	// 衝突相手の情報
	std::vector<std::weak_ptr<ColliderBase>> hitColliders_;
};
