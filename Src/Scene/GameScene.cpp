#include <DxLib.h>
#include "../Application.h"
#include "../Utility/DrawUtiity.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Stage.h"
#include "PauseScene.h"
#include "GameScene.h"

GameScene::GameScene(void) : 
	update_(&GameScene::UpdateGame),
	draw_(&GameScene::DrawGame)
{
	shakeFrame_ = 0;
	shakeRate_ = 0.0f;
	RT_ = -1;
}

GameScene::~GameScene(void)
{
}

void GameScene::LoadData(void)
{
}

void GameScene::Init(void)
{
	//プレイヤー
	stage_ = std::make_unique<Stage>();
	stage_->Init();

	//プレイヤー
	player_ = std::make_unique<Player>();
	player_->Init();

	//敵
	enemy_ = std::make_unique<Enemy>(*player_);
	enemy_->Init();

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	//mainCamera->ChangeMode(Camera::MODE::MOUSE);
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

	//floor_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
	//	ResourceManager::SRC::FLOOR));
	//float scale = 10.0f;
	//floor_.scl = { scale,scale,scale };
	//floor_.pos = { 0.0f, 0.0f, 0.0f };
	//floor_.quaRot = Quaternion();
	//floor_.quaRotLocal =
	//	Quaternion::Euler({ 0.0f,0.0f, 0.0f });
	//floor_.MakeCollider(Collider::TYPE::STAGE);
	//floor_.Update();
	//player_->AddCollider(floor_.collider);
	//enemy_->AddCollider(floor_.collider);
	player_->AddCollider(stage_->GetTransform().collider);
	enemy_->AddCollider(stage_->GetTransform().collider);
}

void GameScene::Update(void)
{
	(this->*update_)();
}

void GameScene::Draw(void)
{
	(this->*draw_)();
}

void GameScene::UpdateBattleStart(void)
{
}

void GameScene::DrawBattleStart(void)
{
}

void GameScene::UpdateGame(void)
{
	InputManager& ins = InputManager::GetInstance();

	player_->Update();
	enemy_->Update();

#ifdef _DEBUG
	if (ins.IsInputPressed("Reset"))
	{
		this->Init();
	}

	if (ins.IsInputTriggered("CameraShake"))
	{
		SceneManager::GetInstance().SetShakeScreen(true);
	}

#endif // _DEBUG

	if (ins.IsInputTriggered("Pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::PAUSE);
		return;
	}

	if (ins.IsInputTriggered("Back"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
	}

	VECTOR backDir = enemy_->GetTransform().GetBack();
	float distance = 60.0f;
	VECTOR target = VAdd(enemy_->GetTransform().pos, VScale(backDir, distance));
	targetPos_ = target;
	//ダウン中のバックスタブ判定
	if (enemy_->GetIsDown() && enemy_->CheckBackstab())
	{
		if (player_->GetIsParry())
		{
			player_->SetPos(target);
			player_->SetRotateY(enemy_->GetTransform().quaRot);
			player_->ChangeState(Player::STATE::BACKSTAB);
			enemy_->ChangeState(Enemy::STATE::BACKSTAB);
		}
	}

	floor_.Update();
}

void GameScene::DrawGame(void)
{
	MV1DrawModel(floor_.modelId);

	//プレイヤー描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();
	//敵描画
	enemy_->Draw();

	if(enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	DrawSphere3D(targetPos_, 10.0f, 16, 0x0000FF, 0x0000FF, true);
}