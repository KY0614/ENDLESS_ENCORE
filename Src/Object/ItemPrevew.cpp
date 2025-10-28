#include <DxLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "ItemPrevew.h"

ItemPrevew::ItemPrevew(void)
{
	prevewScreen_ = 0;
}

ItemPrevew::~ItemPrevew(void)
{
}

void ItemPrevew::Init(void)
{
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::COIN));
	transform_.pos = { 0.f,-100.0f,0.f };
	transform_.scl = { 1.0f,1.0f,1.0f };
	transform_.quaRot = Quaternion();
	transform_.Update();

	prevewScreen_ = MakeScreen(500, 500);
}

void ItemPrevew::Update(void)
{
	transform_.Update();
}

void ItemPrevew::Draw(void)
{
	//SetDrawScreen(prevewScreen_);
	//ClearDrawScreen();

	//SetupCamera_Perspective(DX_PI_F / 3.0f);//âÊäp
	//SetCameraPositionAndAngle(CommonUtility::VECTOR_ZERO,
	//	0, 0, 0);

	////îíÇ¡Ç€Ç¢ÉZÉçÉtÉ@Éì
	//SetDrawBlendMode(DX_BLENDMODE_ALPHA, 168);
	//DrawBox(0, 0, 200, 200, 0Xffffff, true);
	//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	//DrawBoxAA(0, 0, 200, 200, 0Xffffff, false);

	//MV1DrawModel(transform_.modelId);

	//SetDrawScreen(DX_SCREEN_BACK);
	//ClearDrawScreen();

	//DrawGraph(0, 0, prevewScreen_,true);
}