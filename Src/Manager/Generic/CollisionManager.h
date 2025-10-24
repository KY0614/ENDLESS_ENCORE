#pragma once
#include<vector>
#include<memory>
#include<functional>
#include<functional>
#include"../Object/Common/Collider.h"

class CollisionManager
{
	//インスタンス生成
	static void CreateInstance(void);

	//インスタンスの取得
	static CollisionManager& GetInstance(void) { return *instance_; }

	//コライダの追加
	void AddCollider(const std::shared_ptr<Collider> _collider);

	//必要なくなったコライダの削除(更新の最後に置く)
	void Sweep(void);

	//更新
	void Update(void);

	//削除
	void Destroy(void);

private:

	//静的インスタンス
	static CollisionManager* instance_;

	//当たり判定格納
	std::vector<std::shared_ptr<Collider>>colliders_;
};

