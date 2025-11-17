#include <DxLib.h>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Common/Fader.h"
#include "../Utility/DrawUtiity.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Transition/FadeTransitor.h"
#include "../Object/Common/Geometry/Sphere.h"
#include "../Object/Common/Geometry/Cube.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Object/Stage.h"
#include "PauseScene.h"
#include "EncountScene.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	update_ = &GameScene::UpdateExplore;
	draw_ = &GameScene::DrawExplore;
	player_ = nullptr;
	enemy_ = nullptr;
	stage_ = nullptr;
	isFaseChange_ = false;
	//isToutch_ = false;
	//selectList_ = {
	//L"触れる",
	//L"見つめる"
	//};
	//cursorIdx_ = 0;
}

GameScene::~GameScene(void)
{
}

void GameScene::LoadData(void)
{
}

void GameScene::Init(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::EXPLORE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::EXPLORE_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 256 / 3);
	sound.Add(SoundManager::TYPE::BGM, SoundManager::SOUND::BATTLE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::GAME_BGM).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::BATTLE, 256 / 3);
	sound.Play(SoundManager::SOUND::EXPLORE);
	//プレイヤー
	stage_ = std::make_shared<Stage>();
	stage_->Init();

	//プレイヤー
	player_ = std::make_shared<Player>();
	player_->Init();

	//敵
	enemy_ = std::make_shared<Enemy>(*player_);
	enemy_->Init();

	//敵
	encountScene_ = std::make_unique<EncountScene>(*player_,*enemy_);
	encountScene_->Init();

	//選択肢テーブルごとの処理
	//selectFuncTable_ = {
	//{L"触れる",[this]()
	//	{
	//		stage_->ChangeType(Stage::TYPE::BATTLE);
	//		player_->Init();
	//		player_->AddCollider(stage_->GetTransform().collider);
	//		enemy_->AddCollider(stage_->GetTransform().collider);
	//		update_ = &GameScene::UpdateGame;
	//		draw_ = &GameScene::DrawGame;
	//	}
	//},
	//{L"見つめる",[this]()
	//	{
	//		player_->ChangeState(Player::STATE::PLAY);
	//		update_ = &GameScene::UpdateExplore;
	//		draw_ = &GameScene::DrawExplore;
	//	}
	//},
	//};

	//カメラ
	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->SetTarget(&enemy_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);

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
	//start
	DrawSphere3D(VGet(10.0f, -219.0f, 900.0f), 15.0f, 16, 0xffffff, 0xffffff, false);
	//goal
	DrawSphere3D(VGet(10.0f, -219.0f, 1150.0f), 15.0f, 16, 0x00ff00, 0x00ff00, false);
	//camera
	DrawSphere3D(VGet(-200.0f, -219.0f, 1150.0f), 15.0f, 16, 0x0000ff, 0x0000ff, false);

	//enemy
	DrawSphere3D(VGet(10.0f, -219.0f, 3100.0f), 15.0f, 16, 0xff0000, 0xff0000, false);
}

void GameScene::UpdateExplore(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Pause"))
	{
		//ポーズボタンが押されたらポーズシーンへ遷移
		SceneManager::GetInstance().PushScene(SceneManager::SCENE_ID::PAUSE);
		return;
	}

	player_->Update();
	stage_->Update();
	encountScene_->Update();
	//if(ins.IsInputTriggered("Next"))
	//{
	//	update_ = &GameScene::UpdateEncount;
	//	draw_ = &GameScene::DrawEncount;
	//}
	FadeTransitor& fade = FadeTransitor::GetInstance();
	if (ins.IsInputTriggered("Next"))
	{
		update_ = &GameScene::UpdateEncount;
		draw_ = &GameScene::DrawEncount;
		encountScene_->Start();
	}
	//if (SceneManager::GetInstance().GetFade().IsEnd() &&
	//	isFaseChange_)
	//{
		//enemy_->ChangeState(Enemy::STATE::ENCOUNT);
		//player_->ChangeState(Player::STATE::ENCOUNT);
		// 
		//const float distance = 100.0f;
		//VECTOR dir = VAdd(player_->GetTransform().GetLeft(), player_->GetTransform().GetForward());
		//VECTOR startPos = VAdd(player_->GetTransform().pos, VScale(dir, distance));
		//mainCamera->SetCraneUpPos(startPos, 200.0f, player_->GetTransform().pos);
		//mainCamera->ChangeMode(Camera::MODE:::CRANE_UP);

		//mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
		//VECTOR pos = VGet(-50.0f, -210.0f, 1150.0f);
		//mainCamera->SetFixedPointPos(pos, VGet(10.0f, -210.0f, 1150.0f));

		//VECTOR startPos = VGet(-50.0f, -210.0f, 1350.0f);
		//VECTOR endPos = VGet(-50.0f, -210.0f, 1150.0f);
		//mainCamera->SetTrackCamera(startPos, endPos,0.8f);
		//mainCamera->ChangeMode(Camera::MODE::TRACK);
		//SceneManager::GetInstance().SetFadeIn();
		//isFaseChange_ = false;
	//}
	
	//isToutch_ = false;
	//if (CommonUtility::IsHitSpheres(
	//	stage_->GetSphere().GetPos(),
	//	stage_->GetSphere().GetRadius(),
	//	player_->GetSphere().GetPos(),
	//	player_->GetSphere().GetRadius()
	//))
	//{
	//	isToutch_ = true;
	//}

	//if (isToutch_ &&
	//	ins.IsInputTriggered("Parry"))
	//{
	//	player_->ChangeState(Player::STATE::NONE);
	//	update_ = &GameScene::UpdateSelect;
	//	draw_ = &GameScene::DrawSelect;
	//}
}

void GameScene::DrawExplore(void)
{
	//プレイヤー描画
	stage_->Draw();

	//プレイヤー描画
	player_->Draw();

	DrawString(0, 60, L"探索ステージ", 0xAAAAAA);

}

void GameScene::UpdateEncount(void)
{
	InputManager& ins = InputManager::GetInstance();

	//if (mainCamera->IsActionEnd())
	//{
	//	//相対距離
	//	const float distance = 100.0f;
	//	//プレイヤーから見て左斜め前方向
	//	VECTOR dir = VAdd(player_->GetTransform().GetLeft(), player_->GetTransform().GetForward());
	//	VECTOR startPos = VAdd(player_->GetTransform().pos, VScale(dir, distance));
	//	VECTOR endPos = VGet(startPos.x, startPos.y + 100.0f, startPos.z);
	//	const float moveistance = 200.0f;
	//	VECTOR target = player_->GetTransform().pos;
	//	target.y += 50.0f;
	//	mainCamera->SetCraneUpPos(startPos, moveistance, target);
	//	mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
	//}
	encountScene_->Update();
	player_->Update();
	enemy_->Update();
	stage_->Update();
}

void GameScene::DrawEncount(void)
{
	stage_->Draw();

	player_->Draw();
	enemy_->Draw();

	if (enemy_->GetIsDead())
	{
		player_->DrawVictory();
	}

	player_->DrawDead();

	encountScene_->Draw();

	DrawString(0, 60, L"エンカウント", 0xAAAAAA);
}

void GameScene::UpdateGame(void)
{
	InputManager& ins = InputManager::GetInstance();

	//if(player_->GetTransform().pos.y < -50.0f)
	//{
	//	player_->SetHP(0.0f);
	//}

	player_->Update();
	enemy_->Update();
	stage_->Update();
	
#ifdef _DEBUG

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

	VECTOR backDir = enemy_->GetTransform().GetBack();
	float distance = 60.0f;
	VECTOR target = VAdd(enemy_->GetTransform().pos, VScale(backDir, distance));
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
}

void GameScene::DrawGame(void)
{
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

	player_->DrawDead();
}

void GameScene::DrawMessage(void)
{
	const int boxWidth = Application::SCREEN_SIZE_X - 100;
	const int boxHeight = 200;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(100,
		Application::SCREEN_SIZE_Y / 2 - boxHeight / 2,
		boxWidth,
		Application::SCREEN_SIZE_Y / 2 + boxHeight / 2,
		0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	std::wstring str = L"静かに浮かぶ何かがある...。";
	int diff = GetDrawStringWidth(str.c_str(), str.size(), NULL);
	DrawString(Application::SCREEN_SIZE_X / 2 - diff / 2,
		Application::SCREEN_SIZE_Y / 2 - 32,
		str.c_str(), 0xFFFFFF);

	const int lineY = Application::SCREEN_SIZE_Y / 2 + 16;
	int lineX = (Application::SCREEN_SIZE_X / 2 - 150);

	//現在選択している行をずらす幅
	const int currentLineOffset = 20;
	//現在選択している行の文字列
	//std::wstring currentStr = selectList_[cursorIdx_];
	//for (auto& row : selectList_)
	//{
	//	//文字列の幅を取得
	//	int stringWidth = GetDrawStringWidth(row.c_str(), row.size());
	//	unsigned int col = 0xFFFFFF;
	//	if (row == currentStr)
	//	{
	//		DrawString(lineX - currentLineOffset, lineY, L"⇒", 0xFF0000);
	//		col = 0xFF00FF;
	//		//lineX += currentLineOffset;
	//	}

	//	DrawFormatString(lineX + 1, lineY + 1, 0x000000, L"%s", row.c_str());
	//	DrawFormatString(lineX, lineY, col, L"%s", row.c_str());
	//	lineX += 150 + stringWidth;
	//}
}

void GameScene::UpdateDebugImGui(void)
{
	//終了処理
	ImGui::End();
}


//void GameScene::UpdateSelect(void)
//{
//	player_->Update();
//	stage_->Update();
//
//	InputManager& ins = InputManager::GetInstance();

	//if (ins.IsInputTriggered("Left"))
	//{
	//	cursorIdx_ = (cursorIdx_ + 1) % selectList_.size();
	//}
	//if (ins.IsInputTriggered("Right"))
	//{
	//	cursorIdx_ = (cursorIdx_ + selectList_.size() - 1) % selectList_.size();
	//}

	//if (ins.IsInputTriggered("Parry"))
	//{
	//	auto selectedName = selectList_[cursorIdx_];
	//	selectFuncTable_[selectedName]();
	//	return;
	//}
//}

//void GameScene::DrawSelect(void)
//{
//	//プレイヤー描画
//	stage_->Draw();
//
//	//プレイヤー描画
//	player_->Draw();
//
//	DrawMessage();
//}