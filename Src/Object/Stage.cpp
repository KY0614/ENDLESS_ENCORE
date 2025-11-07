#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "Stage.h"

Stage::Stage(void)
{
}

Stage::~Stage(void)
{
}

void Stage::Init(void)
{
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::STAGE));
	const float scale = 8.0f;
	transform_.scl = { scale ,scale ,scale };
	transform_.pos = {1200.0f,0.0f,-400.0f};
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
		//Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.MakeCollider(Collider::TYPE::STAGE);
	transform_.Update();
}

void Stage::Update(void)
{
}

void Stage::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}
