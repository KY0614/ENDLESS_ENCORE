#include "../Libs/ImGui/imgui.h"
#include "../Object/Player.h"
#include "../Object/Enemy.h"
#include "../Common/Fader.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/InputManager.h"
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
	stateChanges_.emplace(STATE::ENEMY_CAST_SPELL, std::bind(&EncountScene::ChangeStateEnemyCastSpell, this));
	stateChanges_.emplace(STATE::ENEMY_ATTACK, std::bind(&EncountScene::ChangeStateEnemyAttack, this));
	stateChanges_.emplace(STATE::LETS_PARRY, std::bind(&EncountScene::ChangeStateLetsParry, this));
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
	ChangeStateFade();
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
	case EncountScene::STATE::ENEMY_CAST_SPELL:
		ImGui::Text("ENEMY_CAST_SPELL");
		break;
	case EncountScene::STATE::ENEMY_ATTACK:
		ImGui::Text("ENEMY_ATTACK");
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
	std::weak_ptr<Fader> fader = SceneManager::GetInstance().GetFader();
	fader.lock()->SetFade(Fader::STATE::FADE_OUT);
	stateUpdate_ = std::bind(&EncountScene::UpdateFade, this);
}

void EncountScene::ChangeStatePlayerWalk(void)
{
	//プレイヤーをステージ上で歩かせる
	player_.ChangeState(Player::STATE::STAGE_WALK);
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
		player_.GetTransform().GetBack(), player_.GetTransform().GetLeft());
	VECTOR pPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(playerBackLeft), CAMERA_PLAYER_HEAD_OFFSET_Y));
	//カメラの高さ調整
	pPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点もプレイヤーの高さに合わせる
	VECTOR targetPos = player_.GetTransform().pos;
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
	//敵をエンカウント状態へ遷移
	enemy_.ChangeState(Enemy::STATE::ENCOUNT);
	//カメラをプレイヤーの後方に固定
	float cameraOffsetY = 70.0f;//カメラの高さ調整
	VECTOR pPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(player_.GetTransform().GetBack()), cameraOffsetY));
	//カメラ位置調整(少し左後ろに)
	const float cameraOffsetX = -30.0f;
	pPos.x += cameraOffsetX;
	pPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = enemy_.GetTransform().pos;
	targetPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;//カメラの高さ調整
	mainCamera->SetFixedPointPos(pPos, targetPos);
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemySpotlight, this);
}

void EncountScene::ChangeStateEnemyAttention(void)
{
	//カメラをプレイヤーの後方に固定
	float cameraOffsetY = 70.0f;//カメラの高さ調整
	VECTOR startPos = VAdd(
		player_.GetTransform().pos,
		VScale(VNorm(player_.GetTransform().GetBack()), cameraOffsetY));
	//カメラ位置調整(少し左後ろに)
	const float cameraOffsetX = -30.0f;
	startPos.x += cameraOffsetX;
	startPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//終了座標(目的位置)を計算
	//被写体から距離を取った位置を終了座標とする
	VECTOR endPos = VSub(
		enemy_.GetTransform().pos,
		VScale(VNorm(VSub(enemy_.GetTransform().pos, startPos)), CAMERA_PLAYER_HEAD_OFFSET_Y));
	endPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = enemy_.GetTransform().pos;
	targetPos.y += CAMERA_ENEMY_HEAD_OFFSET_Y;
	//ドリーインを行う合計の時間
	const float dollyInTotalTime = 5.0f;
	mainCamera->SetDollyQuadOut(startPos, endPos,targetPos, CAMERA_PLAYER_HEAD_OFFSET_Y, dollyInTotalTime);
	mainCamera->ChangeMode(Camera::MODE::DOLLY);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyAttention, this);
}

void EncountScene::ChangeStateEnemyCastSpell(void)
{
	//カメラを敵の右斜め前からスタート
	float distance = 80.0f;//カメラとの距離
	const VECTOR& enemyForwardRight = VAdd(
		enemy_.GetTransform().GetForward(), enemy_.GetTransform().GetRight());
	VECTOR startPos = VAdd(
		enemy_.GetTransform().pos,
		VScale(enemyForwardRight, distance));
	//カメラの高さ調整
	startPos.y += CAMERA_ENEMY_HEAD_OFFSET_Y;
	//終了座標(目的位置)を計算
	//被写体から距離を取った位置を終了座標とする
	distance = 180.0f;
	VECTOR endPos = VAdd(
		enemy_.GetTransform().pos,
		VScale(enemyForwardRight, distance));
	endPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = enemy_.GetFramePos(L"mixamorig:Spine2");
	//ドリーを行う合計の時間
	const float dollyTotalTime = 5.0f;
	mainCamera->SetDollyQuadOut(startPos, endPos, targetPos, CAMERA_PLAYER_HEAD_OFFSET_Y, dollyTotalTime);
	mainCamera->ChangeMode(Camera::MODE::DOLLY);
	enemy_.ChangeState(Enemy::STATE::CAST_SPELL);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyCastSpell, this);
}

void EncountScene::ChangeStateEnemyAttack(void)
{
	//カメラを敵の右斜め前からスタート
	const VECTOR& enemyForward = enemy_.GetTransform().GetForward();
	//終了座標(目的位置)を計算
	//被写体から距離を取った位置を終了座標とする
	const float distance = 160.0f;
	VECTOR endPos = VAdd(
		enemy_.GetTransform().pos,
		VScale(enemyForward, distance));
	endPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
	//注視点を敵の位置にセット
	VECTOR targetPos = enemy_.GetFramePos(L"mixamorig:Head");
	mainCamera->SetFixedPointPos(endPos, targetPos);
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);
	stateUpdate_ = std::bind(&EncountScene::UpdateEnemyAttack, this);
}

void EncountScene::ChangeStateLetsParry(void)
{
	stateUpdate_ = std::bind(&EncountScene::UpdateLetsParry, this);
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
		player_.IsActionEnd())
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
		VECTOR dir = VAdd(player_.GetTransform().GetLeft(), player_.GetTransform().GetForward());
		VECTOR startPos = VAdd(player_.GetTransform().pos, VScale(dir, distance));
		VECTOR endPos = VGet(startPos.x, startPos.y + CAMERA_PLAYER_HEAD_OFFSET_Y, startPos.z);
		//カメラのクレーンアップ移動距離
		const float moveDistance = 100.0f;
		VECTOR target = player_.GetTransform().pos;
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
		player_.GetState() != Player::STATE::LOOK_AROUND)
	{
		intervalTimer_ = 0.0f;
		player_.ChangeState(Player::STATE::LOOK_AROUND);
	}

	//キョロキョロが終わったら次の状態へ
	const float playerLookAroundIntervalTime = 1.5f;
	if (intervalTimer_ >= playerLookAroundIntervalTime &&
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
		player_.ChangeState(Player::STATE::WAIT);
		ChangeState(STATE::ENEMY_CAST_SPELL);
		return;
	}
	//経過時間が一定時間たったら敵は振り向き状態へ遷移
	if (intervalTimer_ >= intervalLightUp	&&
		enemy_.GetState() == Enemy::STATE::ENCOUNT)
	{
		intervalTimer_ = 0.0f;
		enemy_.ChangeState(Enemy::STATE::TURN);
	}
}

void EncountScene::UpdateEnemyCastSpell(void)
{
	if (mainCamera->IsActionEnd())
	{
		ChangeState(STATE::ENEMY_ATTACK);
		return;
	}
}

void EncountScene::UpdateEnemyAttack(void)
{
	//一定時間経過
	intervalTimer_ += SceneManager::GetInstance().GetDeltaTime();
	const float moveInterval = 1.8f;
	if(intervalTimer_ >= moveInterval && !IsSlowMotion())
	{
		StartSlowMotion();
		enemy_.ChangeState(Enemy::STATE::ATTACK_PLAYER);
		player_.ChangeState(Player::STATE::ATTACKED_ENEMY);
		//カメラを敵の前からスタート
		const VECTOR& enemyForward = enemy_.GetTransform().GetForward();
		//被写体から距離を取った位置を開始座標とする
		const float distance = 160.0f;
		VECTOR startPos = VAdd(
			enemy_.GetTransform().pos,
			VScale(enemyForward, distance));
		startPos.y += CAMERA_PLAYER_HEAD_OFFSET_Y;
		const VECTOR& playerBackLeft = VAdd(
			player_.GetTransform().GetBack(), player_.GetTransform().GetLeft());
		VECTOR endPos = VAdd(
			player_.GetTransform().pos,
			VScale(playerBackLeft, 80.0f));
		endPos.y += 50.0f;
		//注視点を敵の頭にセット
		const VECTOR& targetPos = enemy_.GetFramePos(L"mixamorig:Head");
		//ドリーを行う合計の時間
		const float dollyTotalTime = 2.0f;
		//ズームアウトのFOV値
		const float zoomOutFov = 110.0f;
		mainCamera->SetZoomOutDolly(
			zoomOutFov,startPos, endPos, targetPos, dollyTotalTime);
		mainCamera->ChangeMode(Camera::MODE::ZOOM_OUT_DOLLY);
		intervalTimer_ = 0.0f;
	}

	if (mainCamera->IsActionEnd())
	{
		ChangeState(STATE::LETS_PARRY);
		return;
	}
}

void EncountScene::UpdateLetsParry(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Parry"))
	{
		ChangeState(STATE::PLAYER_PARRY);
		return;
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