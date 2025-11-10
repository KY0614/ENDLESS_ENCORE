#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "../Renderer/ModelMaterial.h"
#include "../Renderer/ModelRenderer.h"
#include "TitleScene.h"

namespace
{
	const int ADVERTISE_TIME = 300;
}

TitleScene::TitleScene(void)
{
	toAdvertiseLoopTimer_ = 0;
}

TitleScene::~TitleScene(void)
{

}

void TitleScene::LoadData(void)
{
}

void TitleScene::Init(void)
{
	toAdvertiseLoopTimer_ = ADVERTISE_TIME;
	//定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Enter"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::SELECT);
	}

	if(--toAdvertiseLoopTimer_ <= 0)
	{
		toAdvertiseLoopTimer_ = ADVERTISE_TIME;
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::ADVERTISE);
		return;
	}
}

void TitleScene::Draw(void)
{
	DrawString(0, 0, L"Title", 0xFFFFFF);
	DrawString(Application::SCREEN_SIZE_X/2, 
		Application::SCREEN_SIZE_Y / 2, L"Push Enter or B", 0xFFFFFF);

	////ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	//SetDrawMode(DX_DRAWMODE_BILINEAR);

	//renderer_->Draw();

}

void TitleScene::InitMaterial(void)
{
	//モデル描画用
	material_ = std::make_unique<ModelMaterial>(
		"StdModelVS.cso", 1,
		"StdModelPS.cso", 3
	);
	//タイリングするのでテクスチャアドレスをWRAPに
	material_->SetTextureAddress(ModelMaterial::TEXADDRESS::WRAP);
	material_->SetTextureBuf(0, ResourceManager::GetInstance().
		Load(ResourceManager::SRC::GROUND).handleId_);

	//uvに渡すスケール値
	material_->AddConstBufVS({ TILLING_SIZE ,TILLING_SIZE,TILLING_SIZE,TILLING_SIZE });

	//色の影響度
	material_->AddConstBufPS({ CommonUtility::VECTOR_ONE.x,CommonUtility::VECTOR_ONE.y,CommonUtility::VECTOR_ONE.z,1.0f });

	//ライトの方向
	VECTOR light = GetLightDirection();
	material_->AddConstBufPS({ light.x,light.y,light.z,1.0f });

	//環境光
	material_->AddConstBufPS({ AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR });

	renderer_ = std::make_unique<ModelRenderer>(graoundTran_.modelId, *material_);

}