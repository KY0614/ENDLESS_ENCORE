#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Common/Fader.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/GameSystem/SoundManager.h"
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
	stateChanges_.emplace(STATE::FINISH, std::bind(&EncountScene::ChangeStateFinish, this));

	intervalTimer_ = 0.0f;
	isStateActioned_ = false;
	isFinish_ = false;
}

EncountScene::~EncountScene(void)
{
}

void EncountScene::LoadData(void)
{
}

void EncountScene::Init(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::LIGHT_UP,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::LIGHT_UP_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 256 / 2);

	//fader_ = std::make_unique<Fader>();
	//fader_->Init();

	//初期状態
	ChangeState(STATE::NONE);
}

void EncountScene::Update(void)
{
	//fader_->Update();

	//更新ステップ
	stateUpdate_();
}

void EncountScene::Draw(void)
{
	//暗転・明転
	//fader_->Draw();

	//DebugDraw();
}

void EncountScene::Start(void)
{
	ChangeStateFade();
}

bool EncountScene::IsFadeOutEnd(void)
{
	//フェードアウトが終わったかどうか
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	return fader.lock()->GetState() == Fader::STATE::FADE_OUT &&
		fader.lock()->IsEnd();
	//return fader_->GetState() == Fader::STATE::FADE_OUT &&
	//	fader_->IsEnd();
}

bool EncountScene::IsFadeInEnd(void)
{
	//フェードインが終わったかどうか
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	return fader.lock()->GetState() == Fader::STATE::FADE_IN &&
		fader.lock()->IsEnd();
	//return fader_->GetState() == Fader::STATE::FADE_IN &&
	//	fader_->IsEnd();
}

void EncountScene::ChangeState(STATE state)
{
	intervalTimer_ = 0.0f;
	isStateActioned_ = false;
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
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	//fader_->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFade, this);
}

void EncountScene::ChangeStatePlayerWalk(void)
{
	player_.ChangeState(Player::STATE::STAGE_WALK);
	VECTOR startPos = VGet(-50.0f, -210.0f, 1350.0f);
	VECTOR endPos = VGet(-50.0f, -210.0f, 1150.0f);
	const float& moveTotalTime = 3.0f;
	mainCamera->SetTrackCameraQuadOut(startPos, endPos, moveTotalTime);
	mainCamera->ChangeMode(Camera::MODE::TRACK);
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerWalk, this);
}

void EncountScene::ChangeStatePlayerAttention(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerAttention, this);
}

void EncountScene::ChangeStateBlackOut(void)
{
	const VECTOR& playerBackLeft = VAdd(
		player_.GetTransform().GetBack(), player_.GetTransform().GetLeft());
	VECTOR pPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(playerBackLeft),100.0f));
	const float cameraOffsetY = 100.0f;
	pPos.y += cameraOffsetY;
	const VECTOR& pos = VGet(-50.0f, -210.0f, 1350.0f);
	VECTOR targetPos = player_.GetTransform().pos;
	targetPos.y += cameraOffsetY;
	mainCamera->SetFixedPointPos(pPos, targetPos);
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
	stateUpdate_ = std::bind(&EncountScene::UpdateBlackOut, this);
}

void EncountScene::ChangeStateLookAround(void)
{
    stateUpdate_ = std::bind(&EncountScene::UpdateLookAround, this);
}

void EncountScene::ChangeStateEnemySpotlight(void)
{
	enemy_.ChangeState(Enemy::STATE::ENCOUNT);
	VECTOR pPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(player_.GetTransform().GetBack()), 70.0f));
	const float cameraOffsetY = 100.0f;
	pPos.x += -30.0f;
	pPos.y += cameraOffsetY;
	const VECTOR& pos = VGet(-50.0f, -210.0f, 1350.0f);
	VECTOR targetPos = enemy_.GetTransform().pos;
	targetPos.y += cameraOffsetY;
	mainCamera->SetFixedPointPos(pPos, targetPos);
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemySpotlight, this);
}

void EncountScene::ChangeStateEnemyAttention(void)
{
	VECTOR pPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(player_.GetTransform().GetBack()), 70.0f));
	const float cameraOffsetY = 100.0f;
	pPos.x += -30.0f;
	pPos.y += cameraOffsetY;
	const VECTOR& pos = VGet(-50.0f, -210.0f, 1350.0f);
	VECTOR targetPos = enemy_.GetTransform().pos;
	targetPos.y += 150.0f;
	mainCamera->SetDollyInQuadOut(pPos, targetPos, 100.0f, 5.0f);
	mainCamera->ChangeMode(Camera::MODE::DOLLY_IN);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyAttention, this);
}

void EncountScene::ChangeStateFinish(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	//fader_->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFinish, this);
}

void EncountScene::UpdateNone(void)
{
}

void EncountScene::UpdateFade(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	if (fader.lock()->GetState() == Fader::STATE::FADE_OUT &&
		fader.lock()->IsEnd())
	{
		fader.lock()->SetFade(Fader::STATE::FADE_IN);
		ChangeState(STATE::PLAYER_WALK);
		return;
	}
}

void EncountScene::UpdatePlayerWalk(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	if (mainCamera->IsActionEnd() &&
		player_.IsActionEnd())
	{
		intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
		const float intervalTime = 1.0f;
		if(intervalTimer_ >= intervalTime)
		{
			fader.lock()->SetFade(Fader::STATE::FADE_OUT);
			intervalTimer_ = 0.0f;
			ChangeState(STATE::PLAYER_ATTENTION);
			return;
		}
	}
}

void EncountScene::UpdatePlayerAttention(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//フェードアウトが終わった判定
	const bool fadeOutEnd = fader.lock()->GetState() == Fader::STATE::FADE_OUT &&
		fader.lock()->IsEnd();
	if (fadeOutEnd)
	{
		//フェードイン開始
		fader.lock()->SetFade(Fader::STATE::FADE_IN);

		//相対距離
		const float distance = 80.0f;
		//プレイヤーから見て左斜め前方向
		VECTOR dir = VAdd(player_.GetTransform().GetLeft(), player_.GetTransform().GetForward());
		VECTOR startPos = VAdd(player_.GetTransform().pos, VScale(dir, distance));
		VECTOR endPos = VGet(startPos.x, startPos.y + 100.0f, startPos.z);
		const float moveDistance = 100.0f;
		VECTOR target = player_.GetTransform().pos;
		target.y += 80.0f;
		mainCamera->SetCraneUpPos(startPos, moveDistance, target);
		mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
		intervalTimer_ = 0.0f;
		return;
	}

	//フェードインが終わった判定
	const bool fadeInEnd = fader.lock()->GetState() == Fader::STATE::FADE_IN &&
		fader.lock()->IsEnd();

	//フェードイン終了後、カメラのクレーンアップが終わったら
	//インターバル時間を経過させる
	if (fadeInEnd &&
		mainCamera->IsActionEnd())
	{
		//一定時間経過
		intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	}

	//一定時間経ったら状態遷移
	const float intervalCraneUp = 0.9f;
	if (intervalTimer_ >= intervalCraneUp)
	{
		ChangeState(STATE::BLACK_OUT);	//ブラックアウト状態へ遷移
		return;
	}
}

void EncountScene::UpdateBlackOut(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	//一定時間経ったら暗転
	const float intervalBlackOut = 1.0f;
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	if (intervalTimer_ >= intervalBlackOut)
	{
		sound.AdjustVolume(SoundManager::SOUND::LIGHT_UP, 50);
		sound.Play(SoundManager::SOUND::LIGHT_UP);
		const float blackOutFogStart = 50.0f;
		const float blackOutFogEnd = 1000.0f;
		SceneManager::GetInstance().SetFog(blackOutFogStart, blackOutFogEnd);
		intervalTimer_ = 0.0f;
		ChangeState(STATE::LOOK_AROUND);
		return;
	}
}

void EncountScene::UpdateLookAround(void)
{
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	//プレイヤーが周りをキョロキョロする状態へ移行するまでの時間
	const float intervalLookAround = 0.7f;
	if (intervalTimer_ >= intervalLookAround &&
		player_.GetState() != Player::STATE::LOOK_AROUND)
	{
		intervalTimer_ = 0.0f;
		player_.ChangeState(Player::STATE::LOOK_AROUND);
	}

	//キョロキョロが終わったら次の状態へ
	if (intervalTimer_ >= 1.5f &&
		player_.GetState() == Player::STATE::LOOK_AROUND)
	{
		ChangeState(STATE::ENEMY_SPOTLIGHT);
		return;
	}
}

void EncountScene::UpdateEnemySpotlight(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	//一定時間経ったらライトアップ
	const float intervalLightUp = 2.0f;
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	if (!isStateActioned_ &&
		intervalTimer_ >= intervalLightUp)
	{
		isStateActioned_ = true;
		sound.AdjustVolume(SoundManager::SOUND::LIGHT_UP, 50);
		sound.Play(SoundManager::SOUND::LIGHT_UP);
		SceneManager::GetInstance().ResetFog();
	}
	//一定時間経ったらライトアップ
	const float intervalStateChange = intervalLightUp + 1.0f;
	if (isStateActioned_ &&
		intervalTimer_ >= intervalStateChange)
	{
		ChangeState(STATE::ENEMY_ATTENTION);
		return;
	}
}

void EncountScene::UpdateEnemyAttention(void)
{
	//一定時間経ったら振り向き開始
	const float intervalLightUp = 1.2f;
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();

	if (enemy_.GetState() == Enemy::STATE::ENCOUNT_FINISH &&
		mainCamera->IsActionEnd())
	{
		player_.ChangeState(Player::STATE::WAIT);
		ChangeState(STATE::FINISH);
		return;
	}
	if (intervalTimer_ >= intervalLightUp	&&
		enemy_.GetState() == Enemy::STATE::ENCOUNT)
	{
		enemy_.ChangeState(Enemy::STATE::TURN);
	}
}

void EncountScene::UpdateFinish(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();

	//フェードアウトが終わった判定
	const bool fadeOutEnd = fader.lock()->GetState() == Fader::STATE::FADE_OUT &&
		fader.lock()->IsEnd();
	if (fadeOutEnd)
	{
		isFinish_ = true;
	}
}

void EncountScene::DebugDraw(void)
{
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
	case EncountScene::STATE::FINISH:
		DrawFormatString(0, 100, 0xFFFFFF, L"FINISH");
		break;
	default:
		break;
	}
}