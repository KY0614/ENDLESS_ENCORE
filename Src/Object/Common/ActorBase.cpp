#include "../Application.h"
#include "../Object/Common/AnimationController.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "Collider/ColliderBase.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
{
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::Init(void)
{

}

void ActorBase::Update(void)
{
}

void ActorBase::Draw(void)
{
#ifdef _DEBUG
	//所有しているコライダの描画
	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}
#endif // _DEBUG
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}

const VECTOR ActorBase::GetFramePos(const std::wstring& frameName) const
{
	VECTOR ret = {};
	//フレームIDを取得して、フレームの座標を取得する
	const int frameId = MV1SearchFrame(transform_.modelId, frameName.c_str());
	ret = MV1GetFramePosition(transform_.modelId, frameId);
	return ret;
}

void ActorBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void ActorBase::AddHitCollider(const std::weak_ptr<ColliderBase> hitCollider)
{
	for (const auto& c : hitColliders_)
	{
		if (c.lock() == hitCollider.lock())
		{
			return;
		}
	}
	hitColliders_.emplace_back(hitCollider);
}

void ActorBase::ClearHitCollider(void)
{
	hitColliders_.clear();
}

const std::weak_ptr<ColliderBase> ActorBase::GetOwnCollider(int key) const
{
	//指定されたキーに対応する自身の衝突情報が存在しない場合は空を返す
	if (ownColliders_.count(key) == 0)
	{
		static std::weak_ptr<ColliderBase> nullPtr;
		return nullPtr;
	}
	return ownColliders_.find(key)->second;
}