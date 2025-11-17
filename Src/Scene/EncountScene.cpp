#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Common/Fader.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "EncountScene.h"

EncountScene::EncountScene(
	Player& player,
	Enemy& enemy) : 
	player_(player),
	enemy_(enemy)
{
	state_ = STATE::NONE;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&EncountScene::ChangeStateNone, this));
	stateChanges_.emplace(STATE::FADE, std::bind(&EncountScene::ChangeStateFade, this));
	stateChanges_.emplace(STATE::PLAYER_WALK, std::bind(&EncountScene::ChangeStatePlayerWalk, this));
	stateChanges_.emplace(STATE::PLAYER_ATTENTION, std::bind(&EncountScene::ChangeStatePlayerAttention, this));
	stateChanges_.emplace(STATE::BLACK_OUT, std::bind(&EncountScene::ChangeStateBlackOut, this));
	stateChanges_.emplace(STATE::LOOK_AROUND, std::bind(&EncountScene::ChangeStateLookAround, this));
	stateChanges_.emplace(STATE::ENEMY_SPOTLIGHT, std::bind(&EncountScene::ChangeStateEnemySpotlight, this));
	stateChanges_.emplace(STATE::ENEMY_ATTENTION, std::bind(&EncountScene::ChangeStateEnemyAttention, this));

	intervalTimer_ = 0.0f;
}

EncountScene::~EncountScene(void)
{
}

void EncountScene::LoadData(void)
{
}

void EncountScene::Init(void)
{

	fader_ = std::make_unique<Fader>();
	fader_->Init();

	//初期状態
	ChangeState(STATE::NONE);
}

void EncountScene::Update(void)
{
	fader_->Update();

	//更新ステップ
	stateUpdate_();
}

void EncountScene::Draw(void)
{
	//暗転・明転
	fader_->Draw();


	switch (state_)
	{
	case EncountScene::STATE::NONE:
		break;
	case EncountScene::STATE::FADE:
		DrawFormatString(0, 100, 0xFFFFFF, L"FADE");
		break;
	case EncountScene::STATE::PLAYER_WALK:
		DrawFormatString(0, 100, 0xFFFFFF, L"PLAYER_WALK");
		break;
	case EncountScene::STATE::PLAYER_ATTENTION:
		DrawFormatString(0, 100, 0xFFFFFF, L"PLAYER_ATTENTION");
		break;
	case EncountScene::STATE::BLACK_OUT:
		DrawFormatString(0, 100, 0xFFFFFF, L"BLACK_OUT");
		break;
	case EncountScene::STATE::LOOK_AROUND:
		DrawFormatString(0, 100, 0xFFFFFF, L"LOOK_AROUND");
		break;
	case EncountScene::STATE::ENEMY_SPOTLIGHT:
		DrawFormatString(0, 100, 0xFFFFFF, L"ENEMY_SPOTLIGHT");
		break;
	case EncountScene::STATE::ENEMY_ATTENTION:
		DrawFormatString(0, 100, 0xFFFFFF, L"ENEMY_ATTENTION");
		break;
	default:
		break;
	}
}

void EncountScene::Start(void)
{
	ChangeStateFade();
}

bool EncountScene::IsFadeOutEnd(void)
{
	//フェードアウトが終わったかどうか
	return fader_->GetState() == Fader::STATE::FADE_OUT &&
		fader_->IsEnd();
}

bool EncountScene::IsFadeInEnd(void)
{
	//フェードインが終わったかどうか
	return fader_->GetState() == Fader::STATE::FADE_IN &&
		fader_->IsEnd();
}

void EncountScene::ChangeState(STATE state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void EncountScene::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdateNone, this);
}

void EncountScene::ChangeStateFade(void)
{
	fader_->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFade, this);
}

void EncountScene::ChangeStatePlayerWalk(void)
{
	enemy_.ChangeState(Enemy::STATE::ENCOUNT);
	player_.ChangeState(Player::STATE::ENCOUNT);
	VECTOR startPos = VGet(-50.0f, -210.0f, 1350.0f);
	VECTOR endPos = VGet(-50.0f, -210.0f, 1150.0f);
	mainCamera->SetTrackCamera(startPos, endPos,5.0f);
	mainCamera->ChangeMode(Camera::MODE::TRACK);
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerWalk, this);
}

void EncountScene::ChangeStatePlayerAttention(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerAttention, this);
}

void EncountScene::ChangeStateBlackOut(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdateBlackOut, this);
}

void EncountScene::ChangeStateLookAround(void)
{
    stateUpdate_ = std::bind(&EncountScene::UpdateLookAround, this);
}

void EncountScene::ChangeStateEnemySpotlight(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemySpotlight, this);
}

void EncountScene::ChangeStateEnemyAttention(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyAttention, this);
}

void EncountScene::UpdateNone(void)
{
}

void EncountScene::UpdateFade(void)
{
	if (fader_->GetState() == Fader::STATE::FADE_OUT &&
		fader_->IsEnd())
	{
		fader_->SetFade(Fader::STATE::FADE_IN);
		ChangeState(STATE::PLAYER_WALK);
		return;
	}
}

void EncountScene::UpdatePlayerWalk(void)
{
	if (mainCamera->IsActionEnd())
	{
		intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
		const float intervalTime = 1.0f;
		if(intervalTimer_ >= intervalTime)
		{
			fader_->SetFade(Fader::STATE::FADE_OUT);
			intervalTimer_ = 0.0f;
			ChangeState(STATE::PLAYER_ATTENTION);
			return;
		}
	}
}

void EncountScene::UpdatePlayerAttention(void)
{
	const bool fadeOutEnd = fader_->GetState() == Fader::STATE::FADE_OUT &&
		fader_->IsEnd();
	if (fadeOutEnd)
	{
		fader_->SetFade(Fader::STATE::FADE_IN);

		//相対距離
		const float distance = 100.0f;
		//プレイヤーから見て左斜め前方向
		VECTOR dir = VAdd(player_.GetTransform().GetLeft(), player_.GetTransform().GetForward());
		VECTOR startPos = VAdd(player_.GetTransform().pos, VScale(dir, distance));
		VECTOR endPos = VGet(startPos.x, startPos.y + 100.0f, startPos.z);
		const float moveistance = 200.0f;
		VECTOR target = player_.GetTransform().pos;
		target.y += 50.0f;
		mainCamera->SetCraneUpPos(startPos, moveistance, target);
		mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
		return;
	}

	const bool fadeInEnd = fader_->GetState() == Fader::STATE::FADE_IN &&
		fader_->IsEnd();

	if (fadeInEnd &&
		mainCamera->IsActionEnd())
	{
		ChangeState(STATE::BLACK_OUT);
		return;
	}
}

void EncountScene::UpdateBlackOut(void)
{
	const bool fadeOutEnd = fader_->GetState() == Fader::STATE::FADE_OUT &&
		fader_->IsEnd();
	if (fadeOutEnd)
	{
		fader_->SetFade(Fader::STATE::FADE_IN);

		//相対距離
		const float distance = 100.0f;
		//プレイヤーから見て左斜め前方向
		VECTOR dir = VAdd(player_.GetTransform().GetLeft(), player_.GetTransform().GetForward());
		VECTOR startPos = VAdd(player_.GetTransform().pos, VScale(dir, distance));
		VECTOR endPos = VGet(startPos.x, startPos.y + 100.0f, startPos.z);
		const float moveistance = 200.0f;
		VECTOR target = player_.GetTransform().pos;
		target.y += 50.0f;
		mainCamera->SetCraneUpPos(startPos, moveistance, target);
		mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
		return;
	}
}

void EncountScene::UpdateLookAround(void)
{
}

void EncountScene::UpdateEnemySpotlight(void)
{
}

void EncountScene::UpdateEnemyAttention(void)
{
}
