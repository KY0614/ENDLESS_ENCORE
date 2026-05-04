#include "CollisionManager.h"

CollisionManager* CollisionManager::instance_ = nullptr;

//インスタンスの生成
void CollisionManager::CreateInstance(void)
{
	if (instance_ == nullptr) 
	{
		instance_ = new CollisionManager();
	}
}

//静的インスタンスの取得
CollisionManager& CollisionManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		CollisionManager::CreateInstance();
	}
	return *instance_;
}

void CollisionManager::AddColisonPair(
	const std::weak_ptr<ColliderBase>& colA,
	const std::weak_ptr<ColliderBase>& colB)
{
	//片方でもコライダが無効なら処理を抜ける
	if(colA.lock() == nullptr || colB.lock() == nullptr)
	{
		return;
	}
	//衝突ペアの追加
	CollisionPair pair;
	pair.colA_ = colA;
	pair.colB_ = colB;
	pair.isHit_ = false;
	collisionPairs_.push_back(pair);
}

void CollisionManager::Init(void)
{
}

void CollisionManager::Update(void)
{
}

void CollisionManager::Release(void)
{
}

void CollisionManager::Destroy(void)
{
	//コライダの全削除
	collisionPairs_.clear();

	//自身のインスタンス削除
	delete instance_;
	instance_ = nullptr;
}

CollisionManager::~CollisionManager(void)
{
}

bool CollisionManager::CheckCollision(void)
{
	return false;
}