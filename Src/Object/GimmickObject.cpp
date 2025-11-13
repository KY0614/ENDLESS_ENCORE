#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "GimmickObject.h"

GimmickObject::GimmickObject(void)
{
}

GimmickObject::~GimmickObject(void)
{
}

void GimmickObject::Init(void)
{
	//ÉÇÉfÉãÇÃäÓñ{ê›íË
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	const float scale = 1.0f;
	transform_.scl = { scale ,scale ,scale };
	transform_.pos = VGet(0.0f,0.0f,0.0f);
	transform_.quaRot = Quaternion();
	const float rotY = 0.0f;
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
}

void GimmickObject::Update(void)
{
	transform_.Update();
}

void GimmickObject::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}
