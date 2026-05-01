#include "CollisionManager.h"

CollisionManager* CollisionManager::instance_ = nullptr;

//インスタンスの生成
void CollisionManager::CreateInstance(void)
{
	if (instance_ == nullptr) {
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

void CollisionManager::Init(void)
{

}

void CollisionManager::Release(void)
{

}

void CollisionManager::Destroy(void)
{

}