#include <memory>
#include "../Libs/ImGui/imgui.h"
#include "../Object/Stage.h"
#include "../Object/Common/Collider/ColliderBase.h"
#include "../Object/Character/PlayerTest.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/GameSystem/InputManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
{
}

DebugScene::~DebugScene(void)
{
}

void DebugScene::Init(void)
{
	//ステージ
	stage_ = std::make_unique<Stage>();
	stage_->Init();
	//プレイヤー
	playerTest_ = std::make_unique<PlayerTest>();
	playerTest_->Init();
	playerTest_->Play();

	//カメラ
	mainCamera->SetFollow(&playerTest_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	//コライダー登録
	// ステージモデルのコライダーをプレイヤーに登録
	playerTest_->AddHitCollider(stage_->GetOwnCollider(
		static_cast<int>(Stage::COLLIDER_TYPE::THEATER)));
}

void DebugScene::Update(void)
{
	playerTest_->Update();
	stage_->Update();

	ObjectUpdateImGui();
}

void DebugScene::Draw(void)
{
	stage_->Draw();
	playerTest_->Draw();
}

void DebugScene::UpdateImGui(void)
{
}

void DebugScene::ObjectUpdateImGui(void)
{
	ImGui::Begin("Object");

	if (ImGui::BeginTabBar("TabBar"))
	{
		//プレイヤーのImGui
		if (ImGui::BeginTabItem("Player"))
		{
			playerTest_->UpdateImGui();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}
