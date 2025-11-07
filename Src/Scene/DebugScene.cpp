#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
{
}

DebugScene::~DebugScene(void)
{
}

void DebugScene::LoadData(void)
{
}

void DebugScene::Init(void)
{
	//ƒvƒŒƒCƒ„[
	player_ = std::make_unique<Player>();
	player_->Init();

	//“G
	enemy_ = std::make_unique<Enemy>(*player_);
	enemy_->Init();

	//ƒJƒƒ‰
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	//mainCamera->ChangeMode(Camera::MODE::MOUSE);
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	floor_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::FLOOR));
	float scale = 1.0f;
	floor_.scl = { scale,scale,scale };
	floor_.pos = { 0.0f, 0.0f, 0.0f };
	floor_.quaRot = Quaternion();
	floor_.quaRotLocal =
		Quaternion::Euler({ 0.0f,0.0f, 0.0f });
	floor_.MakeCollider(Collider::TYPE::STAGE);
	floor_.Update();
	player_->AddCollider(floor_.collider);
}

void DebugScene::Update(void)
{
	player_->DebugUpdate();
	enemy_->DebugUpdate();

	floor_.Update();
}

void DebugScene::Draw(void)
{
	MV1DrawModel(floor_.modelId);

	//“G•`‰æ
	enemy_->Draw();
	//ƒvƒŒƒCƒ„[•`‰æ
	player_->Draw();

	if (enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}
}
