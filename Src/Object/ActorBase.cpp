#include "../Application.h"
#include "../Object/Common/AnimationController.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
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
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}