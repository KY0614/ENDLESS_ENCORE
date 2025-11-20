#include <cassert>
#include<EffekseerForDXLib.h>
#include "../Application.h"
#include "../Libs/nlohmann/json.hpp"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Common/DebugDrawFormat.h"
#include "../Common/FpsController.h"
#include "../Common/Easing.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "../Manager/Generic/Camera.h"
#include "Common/AnimationController.h"
#include "Common/Geometry/Capsule.h"
#include "Common/Geometry/Sphere.h"
#include "Common/Collider.h"
#include "Player.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_PLAYER = "Player";
	static const std::string KEY_IDLE = "Idle";
	static const std::string KEY_WALK = "Walk";
	static const std::string KEY_LOOK_AROUND = "LookAround";
	static const std::string KEY_RUN = "Run";
	static const std::string KEY_JUMP = "Jump";
	static const std::string KEY_DODGE = "Dodge";
	static const std::string KEY_BACKSTAB = "Backstab";
	static const std::string KEY_DEATH = "Death";

	//ジャンプ力
	const float JUMP_POW = 9.0f; 
	//重力加速度
	const float GRAVITY_POW = 15.0f;

	//アニメーション再生速度
	const float ANIM_SPEED = 30.0f;

	//移動
	const float STEP_WALK2RUN = 1.5f;	//歩きから走りに切り替わる時間
	const float SPEED_DODGE = 23.0f;	//回避時のスピード
	const float DODGE_DECELERATION_TIME = 0.4f; //回避後の減速にかける時間
	const float SPEED_WALK = 6.0f;		//歩きスピード
	const float SPEED_RUN = 10.0f;		//走るスピード

	//行動時間
	const float DODGE_TIME = 0.4f;		//回避(無敵)時間
	const float PARRY_TIME = 0.3f;		//パリィ時間
}

Player::Player(void)
{
	animationController_ = nullptr;
	state_ = STATE::NONE;
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::STAGE_WALK, std::bind(&Player::ChangeStateStageWalk, this));
	stateChanges_.emplace(STATE::LOOK_AROUND, std::bind(&Player::ChangeStateLookAround, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&Player::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BACKSTAB, std::bind(&Player::ChangeStateBackstab, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Player::ChangeStateDead, this));

	gravHitPosDown_ = CommonUtility::VECTOR_ZERO;
	gravHitPosUp_ = CommonUtility::VECTOR_ZERO;

	effectSmokePlayId_ = -1;
	effectSmokeResId_ = -1;
	stepFootSmoke_ = -1.0f;

	stepJump_ = -1.0f;
	isJump_ = false;
	speed_ = -1.0f;

	movedPos_ = CommonUtility::VECTOR_ZERO;
	moveDir_ = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	moveDiff_ = CommonUtility::VECTOR_ZERO;
	jumpPow_ = CommonUtility::VECTOR_ZERO;
	playerRotY_ = Quaternion::Quaternion();
	goalQuaRot_ = Quaternion::Quaternion();
	stepRotTime_ = 0.0f;
	stepParry_ = 0.0f;
	isJumpUnlimited_ = false;
	isDodge_ = false;
	isDecelerate_ = false;
	stepDodge_ = 0.0f;
	isParry_ = false;
	stepWalk_ = 0.0f;
	stringAlpha_ = 0;
	isActionEnd_ = false;
}

Player::~Player(void)
{
}

void Player::Init(void)
{
	colliders_.clear();
	//3Dモデルの初期化
	Init3DModel();

	//当たり判定の初期化
	InitCollider();

	//アニメーションの設定
	InitAnimation();

	//足煙エフェクト
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;

	//足煙エフェクトの発生間隔
	stepFootSmoke_ = TERM_FOOT_SMOKE;

	//初期状態
	ChangeState(STATE::PLAY);
}

void Player::Update(void)
{
	//HP制限(HPが最大HPを超えないようにする)
	if (hp_ > maxHp_)
	{
		hp_ = maxHp_;
	}
	//下限設定
	if (hp_ <= 0.0f)hp_ = 0.0f;
	//更新ステップ
	stateUpdate_();

	//アニメーション再生
	animationController_->Update();

	transform_.Update();

	UpdateDebugImGui();
}

void Player::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//丸影描画
	DrawShadow();

#ifdef _DEBUG
	DebugDraw();
#endif // _DEBUG
}

void Player::DebugUpdate(void)
{
	//HP制限(HPが最大HPを超えないようにする)
	if (hp_ > maxHp_)
	{
		hp_ = maxHp_;
	}

	transform_.pos.x = 100.0f;
	transform_.pos.z = 0.0f;

	//更新ステップ
	stateUpdate_();

	//アニメーション再生
	animationController_->Update();

	transform_.Update();

	UpdateDebugImGui();
}

void Player::DrawDead(void)
{
	//デバッグ用死亡表記
	if (hp_ <= 0.0f && animationController_->IsEnd())
	{
		//デバッグ用死亡表記
		DrawResultString(L"YOU DIED", 0xff0000);
	}
}

void Player::DrawVictory(void)
{
	//デバッグ用勝利表記
	DrawResultString(L"VICTORY",0xffff00);
}

void Player::DrawResultString(const std::wstring& str, int col)
{
	static int interval = 0;
	//透明度の増加値
	const int alphaSpeed = 5;
	const int maxAlpha = 255;
	const int maxInterval = 120;
	stringAlpha_ = std::clamp(stringAlpha_, 0, maxAlpha);
	if (stringAlpha_ >= maxAlpha)
	{
		if (++interval > maxInterval)
		{
			interval = 0;
			SceneManager::GetInstance().ChangeScene(
				SceneManager::SCENE_ID::TITLE);
			return;
		}
	}
	stringAlpha_ += alphaSpeed;	//透明度を増加させる
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, stringAlpha_);
	SetFontSize(64);
	int diff = GetDrawStringWidth(str.c_str(), str.size(), NULL);
	DrawString(Application::SCREEN_SIZE_X / 2 - diff / 2,
		Application::SCREEN_SIZE_Y / 2 - diff / 2,
		str.c_str(), col);
	SetFontSize(16);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

Capsule& Player::GetCapsule(void) const
{
	return *capsule_;
}

const Sphere& Player::GetSphere(void) const
{
	return *sphere_;
}

bool Player::IsPlay(void) const
{
	//状態がPLAYかどうかを返す
	return state_ == STATE::PLAY;
}

void Player::LoadData(void)
{
}

void Player::Init3DModel(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::PLAYER);

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(KEY_PLAYER))assert(0 && "データが存在しないか不正なデータです");
	const auto& param = data[KEY_PLAYER];

	//データが含まれていない場合はエラーメッセージを出す
	if (!param.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const auto& transformData = param[JsonManager::KEY_TRANSFORM];

	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	const float scale = transformData.value(JsonManager::KEY_SCALE, 1.0f);
	transform_.scl = { scale ,scale ,scale };
	transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
	transform_.quaRot = Quaternion();
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
	//HPを設定
	const auto& paramData = param[JsonManager::KEY_PARAMETER];
	SetHP(paramData.value(JsonManager::KEY_HP, 0.0f));
	SetMaxHP(paramData.value(JsonManager::KEY_MAX_HP, 0.0f));
}

void Player::InitCollider(void)
{
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 20.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos({ 0.0f, 40.0f, 0.0f });
	sphere_->SetRadius(80.0f);
	//sphere_->SetLocalPos({ 0.0f, 80.0f, 70.0f });
	//sphere_->SetRadius(40.0f);

	col_ = 0x000000;
}

void Player::InitAnimation(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::PLAYER);
	const auto& param = data[KEY_PLAYER];
	//データが含まれていない場合はエラーメッセージを出す
	if (!param.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const auto& animPath = param[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Player/";
	const char* KEY_EMPTY = "";
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	const float animSpeedSlow = animSpeed / 2.0f;
	animationController_->Add((int)ANIM_TYPE::WALK_SLOW, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeedSlow);

	animationController_->Add((int)ANIM_TYPE::LOOK_AROUND, path + animPath.value(KEY_LOOK_AROUND, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::WALK, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::RUN, path + animPath.value(KEY_RUN, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::JUMP, path + animPath.value(KEY_JUMP, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::DODGE, path + animPath.value(KEY_DODGE, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::BACKSTAB, path + animPath.value(KEY_BACKSTAB, KEY_EMPTY),
		animSpeed);

	animationController_->Add((int)ANIM_TYPE::DEATH, path + animPath.value(KEY_DEATH, KEY_EMPTY),
		animSpeed);
	//初期アニメーションはアイドルを再生
	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Player::ChangeState(STATE state)
{
	//行動終了判定をリセット
	isActionEnd_ = false;
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStateStageWalk(void)
{
	//ゆっくり歩くアニメーションに変更
	animationController_->Play((int)ANIM_TYPE::WALK_SLOW);
	transform_.pos = VGet(10.0f, -217.0f, 900.0f);
	transform_.quaRot =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(0.0f), 0.0f });
	stateUpdate_ = std::bind(&Player::UpdateStageWalk, this);
}

void Player::ChangeStateLookAround(void)
{
	//周りを見渡すアニメーションに変更
	animationController_->Play((int)ANIM_TYPE::LOOK_AROUND);
	stateUpdate_ = std::bind(&Player::UpdateLookAround, this);
}

void Player::ChangeStateEncount(void)
{
	//待機アニメーションに変更
	animationController_->Play((int)ANIM_TYPE::IDLE);
	stateUpdate_ = std::bind(&Player::UpdateEncount, this);
}

void Player::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateBackstab(void)
{
	//敵と同じ方向を向く
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(-90.0f), 0.0f });

	//アニメーションを途中まで再生
	animationController_->Play((int)ANIM_TYPE::BACKSTAB, false,0.0f,26.0f);
	stateUpdate_ = std::bind(&Player::UpdateBackstab, this);
}

void Player::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Player::UpdateDead, this);
}

void Player::UpdateNone(void)
{//何もしない
}

void Player::UpdateStageWalk(void)
{
	//目標座標
	const float moveEndZ = 1150.0f;
	if (transform_.pos.z <= moveEndZ)
	{
		//ゆっくり歩く処理
		const float walkSpeed = 1.0f;
		movePow_ = VScale(transform_.GetForward(), walkSpeed);
		movedPos_ = VAdd(transform_.pos, movePow_);
	}
	else
	{
		movePow_ = CommonUtility::VECTOR_ZERO;
		animationController_->Play((int)ANIM_TYPE::IDLE);
		isActionEnd_ = true;
	}

	//衝突判定
	Collision();
}

void Player::UpdateLookAround(void)
{
	//if (animationController_->IsEnd())
	//{
	//	ChangeState(STATE::ENCOUNT);
	//}
}

void Player::UpdateEncount(void)
{
}

void Player::UpdatePlay(void)
{
	if(hp_ <= 0.0f)
	{
		ChangeState(STATE::DEAD);
		return;
	}

	//移動処理
	ProcessMove();

	//ジャンプ処理
	ProcessJump();

	//回避処理
	ProcessDodge();

	//パリィ処理
	ProcessParry();

	//移動方向に応じた回転
	Rotate();

	//重力による移動量
	CalcGravityPow();

	//衝突判定
	Collision();

	//歩きエフェクト
	EffectFootSmoke();

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);
}

void Player::UpdateBackstab(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//続きを再生させるための待ち時間
	const float stopTime = 0.6f;
	//
	static bool isPlay = false;
	//経過時間
	static float stateStep_ = 0.0f;	
	//途中までの再生が終わったら経過時間まで待ち、
	//残りのアニメーションを再生する
	if (animationController_->IsEnd())
	{
		stateStep_ += SceneManager::GetInstance().GetDeltaTime();
		if (stateStep_ > stopTime && !isPlay)
		{
			isPlay = true;
			animationController_->Play((int)ANIM_TYPE::BACKSTAB, false, 26.0f, 100.0f, false, true);
		}
	}

	if (isPlay && animationController_->IsEnd())
	{
		//Jsonデータ取得
		const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::PLAYER);
		const auto& param = data[KEY_PLAYER];
		const auto& transformData = param[JsonManager::KEY_TRANSFORM];
		const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
		transform_.quaRotLocal =
			Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
		isPlay = false;
		stateStep_ = 0.0f;
		ChangeState(STATE::PLAY);
		return;
	}
}

void Player::UpdateDead(void)
{
	hp_ = std::clamp(hp_, 0.0f, maxHp_);
	animationController_->Play((int)ANIM_TYPE::DEATH,false);
}

void Player::ProcessMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	double rotRad = 0.0;

	//WASDで位置を変える
	VECTOR dir = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	if (ins.IsInputPressed("Up"))
	{
		dir = VAdd(dir, cameraRot.GetForward());
	}
	if (ins.IsInputPressed("Left"))
	{
		dir = VAdd(dir, cameraRot.GetLeft());
	}
	if (ins.IsInputPressed("Down"))
	{
		dir = VAdd(dir, cameraRot.GetBack());
	}
	if (ins.IsInputPressed("Right"))
	{
		dir = VAdd(dir, cameraRot.GetRight());
	}

	if (!CommonUtility::EqualsVZero(dir))
	{
		//歩いている時間を加算
		stepWalk_ += SceneManager::GetInstance().GetDeltaTime();

		//ジャンプ中に加速しないように
		if (!isJump_ && !isDodge_)
		{
			if (stepWalk_ >= STEP_WALK2RUN)
			{
				speed_ = SPEED_RUN;
			}
			else speed_ = SPEED_WALK;

			//ダッシュ
			if (ins.IsInputPressed("Dash"))
			{
				stepWalk_ = STEP_WALK2RUN;
			}
		}
		moveDir_ = dir;
		movePow_ = VScale(dir, speed_);
		//プレイヤーの向きを移動方向に合わせる
		double goalRotRad = atan2(dir.x, dir.z); // ラジアン
		SetGoalRotate(goalRotRad);

		if (!isJump_ && !isDodge_ && IsEndLanding() && !isDecelerate_)
		{
			//アニメーション
			if (speed_ == SPEED_RUN)
			{
				animationController_->Play((int)ANIM_TYPE::RUN);
			}
			else
			{
				animationController_->Play((int)ANIM_TYPE::WALK);
			}
		}
	}
	else
	{
		stepWalk_ = 0.0f;
		if (!isJump_ && IsEndLanding() && !isDodge_ && !isDecelerate_)
		{
			animationController_->Play((int)ANIM_TYPE::IDLE);
		}
	}
}

void Player::ProcessJump(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Jump");

	//ジャンプ
	if (isHit && IsEndLanding() && !isDodge_)
	{
		isJump_ = true;
		//ジャンプの初速度を設定
		//ここでは、JUMP_POWを初速としてv0に相当する値を設定します
		jumpPow_.y = JUMP_POW;

		// ダッシュジャンプの飛距離を出すために、水平方向の移動速度を初速に加算
		//jumpPow_.x = movePow_.x;
		//jumpPow_.z = movePow_.z;

		// ダッシュジャンプの飛距離を出すために、水平方向の移動速度を初速に加算
		jumpPow_.x = movePow_.x * 0.01f;
		jumpPow_.z = movePow_.z * 0.01f;

		//無理やりアニメーション
		animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
		animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);
	}
}

void Player::ProcessDodge(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Dodge");

	//回避
	if (isHit && !isJump_)
	{
		//回避中はスピードを早くする
		isDodge_ = true;
		speed_ = SPEED_DODGE;
		animationController_->Play((int)ANIM_TYPE::DODGE, true, 0.0f, 4.0f);
		animationController_->SetEndLoop(4.0f, 4.0f, 5.0f);
	}

	if (!isDodge_ && !isDecelerate_)return;
	stepDodge_ += SceneManager::GetInstance().GetDeltaTime();
	//その場で回避した場合は、向いている方向に進ませる
	movePow_ = VScale(transform_.GetForward(), speed_);
	//回避時間を超えたら回避終了
	if (stepDodge_ > DODGE_TIME && isDodge_)
	{
		//回避終了
		isDodge_ = false;
		//速度減衰開始
		isDecelerate_ = true;
		animationController_->Play((int)ANIM_TYPE::DODGE,true,4.0f,-1.0f,false,true);
	}
	//速度減速処理
	if (isDecelerate_)
	{
		stepWalk_ = STEP_WALK2RUN;
		//減速処理
		float decelRate = (stepDodge_ - DODGE_TIME) / DODGE_DECELERATION_TIME;
		float currentSpeed = Easing::CubicOut(decelRate,
			1.0f, SPEED_DODGE, SPEED_RUN);
		speed_ = currentSpeed;
		//減速が終了したら走るスピードに戻す
		if (stepDodge_ > DODGE_TIME + DODGE_DECELERATION_TIME)
		{
			isDecelerate_ = false;
			speed_ = SPEED_RUN;
			stepDodge_ = 0.0f;
		}
	}
}

void Player::ProcessParry(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Parry");
	if (isHit)
	{
		isParry_ = true;
	}

	if (!isParry_)return;
	stepParry_ += SceneManager::GetInstance().GetDeltaTime();
	col_ = 0xff0000;
	if(stepParry_ > PARRY_TIME)
	{
		col_ = 0x000000;
		isParry_ = false;
		stepParry_ = 0.0f;
	}
}

void Player::SetGoalRotate(double rotRad)
{
	//目標回転にカメラのY軸角度を加算
	VECTOR cameraRot = mainCamera->GetAngles();
	Quaternion axis =
		Quaternion::AngleAxis(
			/*(double)cameraRot.y + */rotRad, CommonUtility::AXIS_Y);
	
	//現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);
	
	//しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}
	//目標回転を設定
	goalQuaRot_ = axis;
}

void Player::Rotate(void)
{
	//回転時間の減少
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();
	
	//回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

void Player::Collision(void)
{
	//現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);
	
	//衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();
	
	//移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;
}

void Player::CollisionCapsule(void)
{
	//カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	//カプセルとの衝突判定
	for (const auto c : colliders_)
	{
		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());
		//衝突した複数のポリゴンと衝突回避するまで、
		//プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];
			//地面と異なり、衝突回避位置が不明なため、何度か移動させる
			//この時、移動させる方向は、移動前座標に向いた方向であったり、
			//衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{
				//再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				//最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					//法線の方向にちょっとだけ移動させる
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 2.0f));
					//カプセルも一緒に移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;
				}
				break;
			}
		}
		//検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void Player::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = CommonUtility::DIR_U;

	// 重力の強さ
	float gravityPow = GRAVITY_POW;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = CommonUtility::VECTOR_ZERO;
			//jumpVelocity_ = CommonUtility::VECTOR_ZERO;
			//stepJump_ = 0.0f;
			if (isJump_)
			{
				// 着地モーション
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
			}
			isJump_ = false;
		}

	}
}

void Player::CalcGravityPow(void)
{
	// ジャンプ中の場合のみ重力を適用
	if (isJump_)
	{
		// 重力による速度の減少
		// v = v0 + at の式に相当
		jumpPow_.y -= GRAVITY_POW * SceneManager::GetInstance().GetDeltaTime();
	}
	else
	{
		// 地面にいる場合はジャンプ力をリセット
		//jumpPow_ = CommonUtility::VECTOR_ZERO;

		// 重力方向
		VECTOR dirGravity = CommonUtility::DIR_D;

		// 重力の強さ
		float gravityPow = GRAVITY_POW;

		//重力
		VECTOR gravity = VScale(dirGravity, gravityPow);
		jumpPow_ = VAdd(jumpPow_, gravity);

		// 内積
		float dot = VDot(dirGravity, jumpPow_);
		if (dot >= 0.0f)
		{
			// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
			jumpPow_ = gravity;
		}
	}
}

bool Player::IsEndLanding(void) const
{
	bool ret = true;
	//無限ジャンプモードの場合は常にtrue
	if (isJumpUnlimited_)return ret;

	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;	//終了している
	}

	return false;
}

bool Player::IsEndDodge(void) const
{
	bool ret = true;
	// アニメーションが回避ではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::DODGE)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;	//終了している
	}

	return false;
}

void Player::EffectFootSmoke(void)
{
	stepFootSmoke_ -= SceneManager::GetInstance().GetDeltaTime();

	float len = CommonUtility::MagnitudeF(moveDiff_);

	if (len >= 1.0f &&
		stepFootSmoke_ < 0.0f)
	{

		stepFootSmoke_ = speed_ == SPEED_RUN ? 0.5f : TERM_FOOT_SMOKE;
		//stepFootSmoke_ = TERM_FOOT_SMOKE;

		//エフェクト再生
		effectSmokePlayId_ = PlayEffekseer3DEffect(effectSmokeResId_);

		//大きさ
		float SCALE = 5.0f;
		SetScalePlayingEffekseer3DEffect(effectSmokePlayId_, SCALE, SCALE, SCALE);

		//位置の設定
		SetPosPlayingEffekseer3DEffect(
			effectSmokePlayId_,
			transform_.pos.x,
			transform_.pos.y,
			transform_.pos.z);
	}
}

void Player::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Player");

	ImGui::InputFloat3("pos", &transform_.pos.x);

	//HP用スライダー
	ImGui::SliderFloat("HP", &hp_, 0.0f, maxHp_);

	static float maxHpMax_ = 500.0f;
	//最大HP用の最大値
	ImGui::InputFloat("MaxHP Max", &maxHpMax_, 0.0f);

	//最大HP用スライダー
	ImGui::SliderFloat("MaxHP", &maxHp_, 0.0f, maxHpMax_);

	//通常ジャンプ・無限ジャンプ切り替えボタン
	if (ImGui::Button("Normal Jump"))
	{
		isJumpUnlimited_ = false;
	}
	if (ImGui::Button("Unlimited Jump"))
	{
		isJumpUnlimited_ = true;
	}

	//状態変更ボタン
	if (ImGui::Button("None"))
	{
		ChangeState(STATE::NONE);
	}
	if (ImGui::Button("Play"))
	{
		ChangeState(STATE::PLAY);
	}
	if (ImGui::Button("Dead"))
	{
		ChangeState(STATE::DEAD);
	}

	//終了処理
	ImGui::End();
}

void Player::DebugDraw(void)
{
	int lineH = 2;
	const int HP_BAR_X = 20;         // HPバーの左上X座標
	const int HP_BAR_Y = 20;         // HPバーの左上Y座標
	const int HP_BAR_WIDTH = maxHp_; // HPバーの最大幅
	const int HP_BAR_HEIGHT = 20;    // HPバーの高さ
	float hp = hp_ / maxHp_;
	int barWidth = static_cast<int>(HP_BAR_WIDTH * hp);
	// 背景（グレー）
	DrawBox(HP_BAR_X, HP_BAR_Y, HP_BAR_X + HP_BAR_WIDTH, HP_BAR_Y + HP_BAR_HEIGHT, GetColor(100, 100, 100), TRUE);
	// 現在HP（緑）
	DrawBox(HP_BAR_X, HP_BAR_Y, HP_BAR_X + barWidth, HP_BAR_Y + HP_BAR_HEIGHT, GetColor(0, 255, 0), TRUE);

	VECTOR linePos = VAdd(transform_.pos, VGet(0.0f, 150.0f, 0.0f));
	VECTOR forward = VScale(transform_.GetForward(), 100.0f);
	VECTOR right = VScale(transform_.GetRight(), 120.0f);
	forward.y += 150.0f;
	right.y += 150.0f;
	DrawLine3D(linePos, VAdd(transform_.pos, forward), 0x00ffff);
	DrawLine3D(linePos, VAdd(transform_.pos, right), 0xff0000);

	//VECTOR dir = VAdd(transform_.GetLeft(), transform_.GetForward());
	//VECTOR pos = VAdd(transform_.pos, VScale(dir,30.0f));
	//DrawSphere3D(pos,15.0f,16,0x00ff00,0x00ff00,true);

	//球体描画（色指定あり）
	sphere_->Draw(col_);
}