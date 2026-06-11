#include "../Libs/ImGui/imgui.h"
#include "../Object/Character/EventScene/EncountPlayer.h"
#include "../Object/Character/EventScene/EncountEnemy.h"
#include "../Common/Fader.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/GameSystem/InputManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "EncountScene.h"

namespace
{
	//プレイヤーの歩行開始位置
	const VECTOR PLAYER_WALK_START_POS = { -50.0f, -210.0f, 1350.0f };
	//プレイヤーの歩行終了位置
	const VECTOR PLAYER_WALK_END_POS = { -50.0f, -210.0f, 1150.0f };
	//カメラの移動最大時間
	const float CAMERA_MOVE_TIME_MAX = 3.0f;
	//カメラの各注視点オフセット値	
	const float CAMERA_PLAYER_HEAD_OFFSET_Y = 100.0f;	//プレイヤーの頭の高さ
	const float CAMERA_PLAYER_CHEST_OFFSET_Y = 80.0f;	//プレイヤーの胸の高さ
	const float CAMERA_ENEMY_HEAD_OFFSET_Y = 150.0f;	//敵の頭の高さ

	//サウンドの最大音量
	const int SOUND_VOLUME_MAX = 256;
	//ライトアップSE音量（パーセンテージ)
	const int LIGHT_UP_SE_VOLUME = 50;
}

EncountScene::EncountScene(
	EncountEnemy& encountEnemy,
	EncountPlayer& encountPlayer) :
	encountEnemy_(encountEnemy),
	encountPlayer_(encountPlayer)
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
	isLightUp_ = false;
	isSlowMotion_ = false;
	isFinish_ = false;
	isStop_ = false;
}

EncountScene::~EncountScene(void)
{
}

void EncountScene::Init(void)
{
	//サウンド初期化
	InitSound();

	//初期状態
	ChangeState(STATE::NONE);
}

void EncountScene::Update(void)
{
	//更新ステップ
	stateUpdate_();
}

void EncountScene::Draw(void)
{
}

void EncountScene::Start(void)
{
	//エンカウント演出開始のために
	//フェードアウトから開始
	ChangeState(STATE::FADE);
}

void EncountScene::UpdateImGui(void)
{
	switch (state_)
	{
	case EncountScene::STATE::NONE:
		ImGui::Text("NONE");
		break;
	case EncountScene::STATE::FADE:
		ImGui::Text("FADE");
		break;
	case EncountScene::STATE::PLAYER_WALK:
		ImGui::Text("PLAYER_WALK");
		break;
	case EncountScene::STATE::PLAYER_ATTENTION:
		ImGui::Text("PLAYER_ATTENTION");
		break;
	case EncountScene::STATE::BLACK_OUT:
		ImGui::Text("BLACK_OUT");
		break;
	case EncountScene::STATE::LOOK_AROUND:
		ImGui::Text("LOOK_AROUND");
		break;
	case EncountScene::STATE::ENEMY_SPOTLIGHT:
		ImGui::Text("ENEMY_SPOTLIGHT");
		break;
	case EncountScene::STATE::ENEMY_ATTENTION:
		ImGui::Text("ENEMY_ATTENTION");
		break;
	case EncountScene::STATE::FINISH:
		ImGui::Text("FINISH");
		break;
	default:
		break;
	}
}

void EncountScene::InitSound(void)
{
	//サウンド設定
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::LIGHT_UP,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::LIGHT_UP_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, SOUND_VOLUME_MAX / 2);
}

void EncountScene::ChangeState(STATE state)
{
	intervalTimer_ = 0.0f;
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
	//フェードアウト開始
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFade, this);
}

void EncountScene::ChangeStatePlayerWalk(void)
{
	//プレイヤーのエンカウント演出開始
	encountPlayer_.IsEncountStart();
	encountPlayer_.StageWalk();//プレイヤーを歩かせる
	//カメラをトラック移動させる(ステージからプレイヤーに向かって)
	//足元を映すように
	mainCamera->SetTrackQuadOut(
		PLAYER_WALK_START_POS,
		PLAYER_WALK_END_POS,
		CAMERA_MOVE_TIME_MAX);
	mainCamera->ChangeMode(Camera::MODE::TRACK);
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerWalk, this);
}

void EncountScene::ChangeStatePlayerAttention(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdatePlayerAttention, this);
}

void EncountScene::ChangeStateBlackOut(void)
{
	//カメラをプレイヤーの左斜め後ろに固定
	const VECTOR& playerBackLeft = VAdd(
		encountPlayer_.GetTransform().GetBack(), encountPlayer_.GetTransform().GetLeft());
	VECTOR pPos = VAdd(
		encountPlayer_.GetTransform().pos,
		VScale(VNorm(playerBackLeft), CAMERA_PLAYER_HEAD_OFFSET_Y));
	//カメラの高さ調整
	pPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点もプレイヤーの高さに合わせる
	VECTOR targetPos = encountPlayer_.GetTransform().pos;
	targetPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//カメラ位置セット&固定
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
	//敵のエンカウント演出開始
	encountEnemy_.EncountStart();
	//カメラをプレイヤーの後方に固定
	float cameraOffsetY = 70.0f;//カメラの高さ調整
	VECTOR pPos = VAdd(
		encountPlayer_.GetTransform().pos,
		VScale(VNorm(encountPlayer_.GetTransform().GetBack()), cameraOffsetY));
	//カメラ位置調整(少し左後ろに)
	const float cameraOffsetX = -30.0f;
	pPos.x += cameraOffsetX;
	pPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = encountEnemy_.GetTransform().pos;
	targetPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;//カメラの高さ調整
	mainCamera->SetFixedPointPos(pPos, targetPos);
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemySpotlight, this);
}

void EncountScene::ChangeStateEnemyAttention(void)
{
	//カメラをプレイヤーの後方に固定
	float cameraOffsetY = 70.0f;//カメラの高さ調整
	const Transform& playerTransform = encountPlayer_.GetTransform();
	const Transform& enemyTransform = encountEnemy_.GetTransform();
	VECTOR startPos = VAdd(
		playerTransform.pos,
		VScale(VNorm(playerTransform.GetBack()), cameraOffsetY));
	//カメラ位置調整(少し左後ろに)
	const float cameraOffsetX = -30.0f;
	startPos.x += cameraOffsetX;
	startPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//終了座標(目的位置)を計算
	//被写体から距離を取った位置を終了座標とする
	VECTOR endPos = VSub(
		enemyTransform.pos,
		VScale(VNorm(VSub(enemyTransform.pos, startPos)), CAMERA_PLAYER_HEAD_OFFSET_Y));
	endPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = enemyTransform.pos;
	targetPos.y += CAMERA_ENEMY_HEAD_OFFSET_Y;
	//ドリーインを行う合計の時間
	const float dollyInTotalTime = 5.0f;
	mainCamera->SetDollyQuadOut(startPos, endPos,targetPos, CAMERA_PLAYER_HEAD_OFFSET_Y, dollyInTotalTime);
	mainCamera->ChangeMode(Camera::MODE::DOLLY);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyAttention, this);
}

void EncountScene::ChangeStateFinish(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFinish, this);
}

void EncountScene::UpdateNone(void)
{
}

void EncountScene::UpdateFade(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	if (SceneManager::GetInstance().IsFadeOutEnd())
	{
		fader.lock()->SetFade(Fader::STATE::FADE_IN);
		ChangeState(STATE::PLAYER_WALK);
		return;
	}
}

void EncountScene::UpdatePlayerWalk(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//カメラのトラック移動とプレイヤーの歩行が終わったら
	//一定時間経過させてから次の状態へ遷移&フェードアウト
	if (mainCamera->IsActionEnd() &&
		encountPlayer_.GetState()==EncountPlayer::STATE::STAGE_WAIT)
	{
		intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
		const float intervalTime = 1.0f;
		if(intervalTimer_ >= intervalTime)
		{
			fader.lock()->SetFade(Fader::STATE::FADE_OUT);
			intervalTimer_ = 0.0f;
			//プレイヤーに注目状態へ遷移
			ChangeState(STATE::PLAYER_ATTENTION);
			return;
		}
	}
}

void EncountScene::UpdatePlayerAttention(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	if (SceneManager::GetInstance().IsFadeOutEnd())
	{
		//フェードイン開始
		fader.lock()->SetFade(Fader::STATE::FADE_IN);

		//相対距離
		const float distance = 80.0f;
		//プレイヤーから見て左斜め前方向
		const Transform& playerTransform = encountPlayer_.GetTransform();
		VECTOR dir = VAdd(playerTransform.GetLeft(), playerTransform.GetForward());
		VECTOR startPos = VAdd(playerTransform.pos, VScale(dir, distance));
		VECTOR endPos = VGet(startPos.x, startPos.y + CAMERA_PLAYER_HEAD_OFFSET_Y, startPos.z);
		//カメラのクレーンアップ移動距離
		const float moveDistance = 100.0f;
		VECTOR target = playerTransform.pos;
		target.y += CAMERA_PLAYER_CHEST_OFFSET_Y;
		//クレーンアップ移動速度
		const float craneUpSpeed = 0.5f;
		mainCamera->SetCraneUpPos(startPos, moveDistance, target, craneUpSpeed);
		mainCamera->ChangeMode(Camera::MODE::CRANE_UP);
		intervalTimer_ = 0.0f;
		return;
	}

	//フェードイン終了後、カメラのクレーンアップが終わったら
	//インターバル時間を経過させる
	if (SceneManager::GetInstance().IsFadeInEnd() &&
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
		//ライトアップSE再生
		sound.AdjustVolume(SoundManager::SOUND::LIGHT_UP, LIGHT_UP_SE_VOLUME);
		sound.Play(SoundManager::SOUND::LIGHT_UP);
		//暗転処理(フォグの開始距離と終了距離を設定)
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
	//プレイヤーが周りをキョロキョロする状態へ移行するまでの時間(少し経ってからアニメーションさせたいので)
	const float intervalLookAround = 0.7f;
	if (intervalTimer_ >= intervalLookAround &&
		encountPlayer_.GetState() != EncountPlayer::STATE::LOOK_AROUND)
	{
		intervalTimer_ = 0.0f;
		encountPlayer_.LookAround();
	}

	//キョロキョロが終わったら次の状態へ
	const float playerLookAroundIntervalTime = 1.5f;
	if (intervalTimer_ >= playerLookAroundIntervalTime &&
		encountPlayer_.GetState() == EncountPlayer::STATE::LOOK_AROUND)
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
	if (!isLightUp_ &&
		intervalTimer_ >= intervalLightUp)
	{
		isLightUp_ = true;
		//ライトアップSE再生
		sound.AdjustVolume(SoundManager::SOUND::LIGHT_UP, LIGHT_UP_SE_VOLUME);
		sound.Play(SoundManager::SOUND::LIGHT_UP);
		//フォグをリセット(明るくする)
		SceneManager::GetInstance().ResetFog();
	}
	//ライトアップから一定時間経ったら次の状態へ
	const float intervalStateChange = intervalLightUp + 1.0f;
	if (isLightUp_ &&
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
	//状態遷移するまでに少し間隔をあける時間
	const float changeInterval = 5.0f;
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	//振り向きが終わったらプレイヤーは待機状態
	//エンカウント演出は終了状態へ
	if (/*enemy_.GetState() == Enemy::STATE::ENCOUNT_FINISH &&*/
		intervalTimer_ >= changeInterval &&
		mainCamera->IsActionEnd())
	{
		//player_.ChangeState(Player::STATE::WAIT);
		ChangeState(STATE::FINISH);
		//ChangeState(STATE::ENEMY_CAST_SPELL);
		return;
	}
	//経過時間が一定時間たったら敵は振り向き状態へ遷移
	if (intervalTimer_ >= intervalLightUp	&&
		encountEnemy_.GetState() == EncountEnemy::STATE::ENCOUNT)
	{
		intervalTimer_ = 0.0f;
		//敵振り向き開始
		encountEnemy_.Turn();
	}
}

void EncountScene::UpdateFinish(void)
{
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	//フェードアウトが終わったら演出終了フラグを立てる
	if (SceneManager::GetInstance().IsFadeOutEnd())
	{
		isFinish_ = true;
	}
}