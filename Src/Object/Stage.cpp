#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "Common/Geometry/Cube.h"
#include "Stage.h"

Stage::Stage(void)
{
	type_ = TYPE::EXPLORE;
}

Stage::~Stage(void)
{
}

void Stage::Init(void)
{
	//3Dモデル初期化
	Init3DModel();

	cube_ = std::make_unique<Cube>(stageTransform_[type_].pos, stageTransform_[type_].quaRot,
		VGet(-1375.0f,-219.0f,834.0f), VGet(-1375.0f, -219.0f, 3375.0f));

}

void Stage::Update(void)
{
	stageTransform_[type_].Update();
	//UpdateDebugImGui();
}

void Stage::Draw(void)
{
	MV1DrawModel(stageTransform_[type_].modelId);
}

void Stage::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Stage");

	ImGui::InputFloat3("position", &stageTransform_[type_].pos.x);
	ImGui::SliderFloat("positionX", &stageTransform_[type_].pos.x,-10000.0f,10000.0f);
	ImGui::SliderFloat("positionY", &stageTransform_[type_].pos.y,-10000.0f,10000.0f);
	ImGui::SliderFloat("positionZ", &stageTransform_[type_].pos.z,-10000.0f,10000.0f);

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
	stageTransform_[TYPE::EXPLORE].quaRotLocal = Quaternion();
	//Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
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