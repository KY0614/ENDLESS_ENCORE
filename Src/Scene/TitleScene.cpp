#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "../Renderer/ModelMaterial.h"
#include "../Renderer/ModelRenderer.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
{
}

TitleScene::~TitleScene(void)
{

}

void TitleScene::Init(void)
{
	//定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TUTORIAL);
	}
}

void TitleScene::Draw(void)
{
	DrawString(0, 0, L"Title", 0xFFFFFF);

	////ロゴを小さめに縮小しているのでジャギーが目立たないようにバイリニア法で描画
	//SetDrawMode(DX_DRAWMODE_BILINEAR);

	//renderer_->Draw();

	//MV1DrawModel(cafeTran_.modelId);
	//MV1DrawModel(character_.modelId);
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
	material_->AddConstBufPS({ AsoUtility::VECTOR_ONE.x,AsoUtility::VECTOR_ONE.y,AsoUtility::VECTOR_ONE.z,1.0f });

	//ライトの方向
	VECTOR light = GetLightDirection();
	material_->AddConstBufPS({ light.x,light.y,light.z,1.0f });

	//環境光
	material_->AddConstBufPS({ AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR });

	renderer_ = std::make_unique<ModelRenderer>(graoundTran_.modelId, *material_);

}

void TitleScene::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("cafe");

	// 大きさ
	ImGui::Text("scale");
	ImGui::InputFloat("SclX", &graoundTran_.scl.x);
	ImGui::InputFloat("SclY", &graoundTran_.scl.y);
	ImGui::InputFloat("SclZ", &graoundTran_.scl.z);

	//位置
	ImGui::Text("position");
	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("Pos", &cafeTran_.pos.x);
	ImGui::SliderFloat("PosX", &cafeTran_.pos.x, -10000.0f, 10000.0f);
	ImGui::SliderFloat("PosY", &cafeTran_.pos.y, -10000.0f, 10000.0f);
	ImGui::SliderFloat("PosZ", &cafeTran_.pos.z, -10000.0f, 10000.0f);

	//終了処理
	ImGui::End();
}