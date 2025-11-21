#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "Common/Geometry/Cube.h"
#include "Common/Geometry/Box.h"
#include "Stage.h"

Stage::Stage(void)
{
	type_ = TYPE::EXPLORE;
}

Stage::~Stage(void)
{
}

void Stage::Init(VECTOR pos, VECTOR sPos)
{
	//3Dモデル初期化
	Init3DModel();

	//モデル描画用
	material_ = std::make_unique<ModelMaterial>(
		"StdModelVS.cso", 2,
		"StdModelPS.cso", 7
	);
	//カメラ座標
	VECTOR CameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	material_->AddConstBufVS({ CameraPos.x,CameraPos.y,CameraPos.z,0.0f });
	//フォグ座標
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	material_->AddConstBufVS({ fogStart,fogEnd,0.0f,0.0f });

	//ピクセルシェーダーの定数バッファ設定
	material_->AddConstBufPS({ 1.0f,1.0f,1.0f,1.0f });

	VECTOR lightDir = GetLightDirection();
	material_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,0.0f });

	float anbientCol = 0.0f;
	material_->AddConstBufPS({ anbientCol,anbientCol,anbientCol,1.0f });

	int fogColorR, fogColorG, fogColorB;
	GetFogColor(&fogColorR, &fogColorG, &fogColorB);
	material_->AddConstBufPS({ 0.1f,0.1f,0.1f,1.0f });

	//ポイントライト
	material_->AddConstBufPS({ pos.x,pos.y,pos.z,500.0f });

	//スポットライト
	material_->AddConstBufPS({ sPos.x,sPos.y,sPos.z,600.0f });
	VECTOR spotDir = CommonUtility::DIR_D;
	material_->AddConstBufPS({ spotDir.x,spotDir.y,spotDir.z,120.0f });

	renderer_ = std::make_unique<ModelRenderer>(stageTransform_[type_].modelId, *material_);

	//cube_ = std::make_unique<Cube>(
	//	stageTransform_[type_].pos,
	// stageTransform_[type_].quaRot,
	//	VGet(-1375.0f, -219.0f, 2000.0f),
	//	VGet(1430.0f, 219.0f, 4375.0f));

	//cube_ = std::make_unique<Cube>(stageTransform_[type_].pos,
	//	stageTransform_[type_].quaRot,
	//	VGet(2700.0f,430.0f,2600.0f));

	cube_ = std::make_unique<Box>(stageTransform_[type_]);
	//cube_->SetLocalCenter(VGet(0.0f, -150.0f, 3000.0f));
	cube_->SetLocalCenter(VGet(0.0f, 0.0f, 0.0f));
	cube_->SetSize(VGet(2700.0f/2.0f, 500.0f / 2.0f, 500.0f));
}

void Stage::Update(void)
{
	//カメラ座標
	VECTOR CameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	material_->SetConstBufVS(0, { CameraPos.x,CameraPos.y,CameraPos.z,0.0f });
	//フォグ座標
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	material_->SetConstBufVS(1, { fogStart,fogEnd,0.0f,0.0f });

	//フォグの色
	int fogColorR, fogColorG, fogColorB;
	GetFogColor(&fogColorR, &fogColorG, &fogColorB);
	material_->SetConstBufPS(3, { 0.0f,0.0f,0.0f,0.0f });

	stageTransform_[type_].Update();

	//UpdateDebugImGui();
}

void Stage::Draw(void)
{
	renderer_->Draw();
	cube_->Draw();
	DrawSphere3D(stageTransform_[type_].pos, 30.0f, 32, 0x00ffff, 0x00ffff, true);
}

void Stage::UpdateDebugImGui(void)
{

	//ウィンドウタイトル&開始処理
	ImGui::Begin("Stage");
	VECTOR pos = cube_->GetCenter();
	//ボックスの位置調整
	ImGui::InputFloat3("cubePos", &pos.x);
	ImGui::SliderFloat("cubePosX", &pos.x, -5000.0f, 5000.0f);
	ImGui::SliderFloat("cubePosY", &pos.y, -5000.0f, 5000.0f);
	ImGui::SliderFloat("cubePosZ", &pos.z, -5000.0f, 5000.0f);
	cube_->SetLocalCenter(pos);

	VECTOR size = cube_->GetSize();
	//ボックスの大きさ調整
	ImGui::InputFloat3("cubeSize", &size.x);
	ImGui::SliderFloat("cubeSizeX", &size.x, -5000.0f, 5000.0f);
	ImGui::SliderFloat("cubeSizeY", &size.y, -5000.0f, 5000.0f);
	ImGui::SliderFloat("cubeSizeZ", &size.z, -5000.0f, 5000.0f);

	//cube_->SetSize(VGet(2700.0f / 2.0f, 430.0f / 2.0f, 1000.0f));

	//終了処理
	ImGui::End();
}

void Stage::Init3DModel(void)
{
	stageTransform_[TYPE::EXPLORE].SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::THEATER));
	float scale = 1.5f;
	stageTransform_[TYPE::EXPLORE].scl = { scale ,scale ,scale };
	stageTransform_[TYPE::EXPLORE].pos = { 0.0f,0.0f,-1000.0f };
	stageTransform_[TYPE::EXPLORE].quaRot = Quaternion();
	//stageTransform_[TYPE::EXPLORE].quaRot = Quaternion::Euler({ CommonUtility::Deg2RadF(30.0f), 0.0f, 0.0f });
	stageTransform_[TYPE::EXPLORE].quaRotLocal = Quaternion();
	stageTransform_[TYPE::EXPLORE].MakeCollider(Collider::TYPE::STAGE);
	stageTransform_[TYPE::EXPLORE].Update();

	stageTransform_[TYPE::BATTLE].SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::STAGE));
	scale = 8.0f;
	stageTransform_[TYPE::BATTLE].scl = { scale ,scale ,scale };
	stageTransform_[TYPE::BATTLE].pos = {1600.0f,0.0f,-5500.0f};
	stageTransform_[TYPE::BATTLE].quaRot = Quaternion();
	stageTransform_[TYPE::BATTLE].quaRotLocal = Quaternion();
	//Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	stageTransform_[TYPE::BATTLE].MakeCollider(Collider::TYPE::STAGE);
	stageTransform_[TYPE::BATTLE].Update();
}