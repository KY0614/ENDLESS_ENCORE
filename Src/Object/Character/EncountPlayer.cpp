#include <cassert>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "../Common/AnimationController.h"
#include "../Common/Geometry/Capsule.h"
#include "EncountPlayer.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_PLAYER = "Player";
	static const std::string KEY_WAKE_UP = "WakeUp";
	static const std::string KEY_IDLE = "Idle";
	static const std::string KEY_WALK = "Walk";
	static const std::string KEY_LOOK_AROUND = "LookAround";
	static const std::string KEY_ATTACKED = "Attacked";
	static const std::string KEY_RUN = "Run";
	static const std::string KEY_JUMP = "Jump";
	static const std::string KEY_DODGE = "Dodge";
	static const std::string KEY_BACKSTAB = "Backstab";
	static const std::string KEY_DEATH = "Death";
	static const std::string KEY_STAGE_POS = "stageWalkPosition";

	//回転完了までの時間
	const float TIME_ROT = 0.3f;

	//煙エフェクト発生間隔
	const float TERM_FOOT_SMOKE = 0.3f;

	//ジャンプ力
	const float JUMP_POW = 9.0f;
	//XZ方向のジャンプ力減衰率
	const float JUMP_POW_DECEL_RATE = 0.01f;
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

	//ステージを歩き始める位置(Z座標)
	const float WALK_STAGE_POS_Z = 1150.0f;
	//攻撃を受けるZ座標
	const float ATTACKED_POS_Z = 2244.0f;
	const float POS_Z = -2510.0f;
	//ステージを歩くスピード
	const float WALK_SPEED_SLOW = 1.0f;
}

EncountPlayer::EncountPlayer(void)
{
	state_ = STATE::NONE;
	movedPos_ = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	jumpPow_ = CommonUtility::VECTOR_ZERO;
	isActionEnd_ = false;
	isEncountStart_ = false;
	stateChanges_.emplace(STATE::NONE, std::bind(&EncountPlayer::ChangeStateNone, this));
	stateChanges_.emplace(STATE::STAGE_WALK, std::bind(&EncountPlayer::ChangeStateStageWalk, this));
	stateChanges_.emplace(STATE::STAGE_WAIT, std::bind(&EncountPlayer::ChangeStateStageWait, this));
	stateChanges_.emplace(STATE::LOOK_AROUND, std::bind(&EncountPlayer::ChangeStateLookAround, this));
	stateChanges_.emplace(STATE::ATTACKED_ENEMY, std::bind(&EncountPlayer::ChangeStateAttackedEnemy, this));
}

EncountPlayer::~EncountPlayer(void)
{
}

void EncountPlayer::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();

	//当たり判定の初期化
	InitCollider();

	//アニメーションの初期化
	InitAnimation();

	//初期状態
	ChangeState(STATE::NONE);
}

void EncountPlayer::Update(void)
{
	//更新ステップ
	stateUpdate_();

	//アニメーション再生
	animationController_->Update();

	transform_.Update();
}

void EncountPlayer::Draw(void)
{
	if (!isEncountStart_)return;

	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//丸影描画
	DrawShadow();
}

void EncountPlayer::StageWalk(void)
{
	//ステージ上を歩く状態に遷移
	ChangeState(STATE::STAGE_WALK);
}

void EncountPlayer::LookAround(void)
{
	ChangeState(STATE::LOOK_AROUND);
}

void EncountPlayer::AttackedEnemy(void)
{
	ChangeState(STATE::ATTACKED_ENEMY);
}

void EncountPlayer::UpdateImGui(void)
{
	//座標
	ImGui::InputFloat3("Pos", &transform_.pos.x);
	const float posMin = -10000.0f;
	const float posMax = 10000.0f;
	ImGui::SliderFloat("PosX", &transform_.pos.x, posMin, posMax);
	ImGui::SliderFloat("PosY", &transform_.pos.y, posMin, posMax);
	ImGui::SliderFloat("PosZ", &transform_.pos.z, posMin, posMax);

	ImGui::SliderFloat("MovePosX", &movedPos_.x, posMin, posMax);
	ImGui::SliderFloat("MovePosY", &movedPos_.y, posMin, posMax);
	ImGui::SliderFloat("MovePosZ", &movedPos_.z, posMin, posMax);

	ImGui::SliderFloat("MovePowX", &movePow_.x, -1.0f, 50.0f);
	ImGui::SliderFloat("MovePowY", &movePow_.y, -1.0f, 50.0f);
	ImGui::SliderFloat("MovePowZ", &movePow_.z, -1.0f, 50.0f);
}

void EncountPlayer::Init3DModel(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json& playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);

	//データが含まれていない場合はエラーメッセージを出す
	if (!playerData.contains(JsonManager::KEY_TRANSFORM))
	{
		assert(0 && "データが存在しないか不正なデータです");
	}
	//Transformデータ取得
	const json& transformData = playerData.at(JsonManager::KEY_TRANSFORM);

	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	//パラメータを取得
	const json& paramData = playerData[JsonManager::KEY_PARAMETER];
	const float scale = transformData.value(JsonManager::KEY_SCALE, 0.0f);
	transform_.scl = { scale ,scale ,scale };
	//座標をステージ上の端(手前側)に設定
	transform_.pos = JsonManager::GetParseVector(paramData, KEY_STAGE_POS);
	transform_.quaRot = Quaternion();
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
}

void EncountPlayer::InitCollider(void)
{
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	const VECTOR localPosTop = { 0.0f, 110.0f, 0.0f };
	const VECTOR localPosDown = { 0.0f, 20.0f, 0.0f };
	const float capsuleRadius = 20.0f;
	capsule_->SetLocalPosTop(localPosTop);
	capsule_->SetLocalPosDown(localPosDown);
	capsule_->SetRadius(capsuleRadius);
}

void EncountPlayer::InitAnimation(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);
	//データが含まれていない場合はエラーメッセージを出す
	if (!playerData.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const json& animPath = playerData[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Player/";
	const char* KEY_EMPTY = "";
	//アニメーション速度
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	const float animSpeedSlow = animSpeed / 2.0f;	//ゆっくり再生する速度（半分)
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	//待機状態
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	//ゆっくり歩く
	animationController_->Add((int)ANIM_TYPE::WALK_SLOW, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeedSlow);
	//周りを見渡す
	animationController_->Add((int)ANIM_TYPE::LOOK_AROUND, path + animPath.value(KEY_LOOK_AROUND, KEY_EMPTY),
		animSpeed);
	//攻撃をされる
	animationController_->Add((int)ANIM_TYPE::ATTACKED, path + animPath.value(KEY_ATTACKED, KEY_EMPTY),
		animSpeedSlow);
}

void EncountPlayer::ChangeState(const STATE& state)
{
	//行動終了判定をリセット
	isActionEnd_ = false;
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void EncountPlayer::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EncountPlayer::UpdateNone, this);
}

void EncountPlayer::ChangeStateStageWalk(void)
{
	stateUpdate_ = std::bind(&EncountPlayer::UpdateStageWalk, this);
}

void EncountPlayer::ChangeStateStageWait(void)
{
	//待機アニメーション
	animationController_->Play((int)ANIM_TYPE::IDLE);
	stateUpdate_ = std::bind(&EncountPlayer::UpdateStageWait, this);
}

void EncountPlayer::ChangeStateLookAround(void)
{
	//周りを見渡すアニメーションに変更
	animationController_->Play((int)ANIM_TYPE::LOOK_AROUND, false);
	stateUpdate_ = std::bind(&EncountPlayer::UpdateLookAround, this);
}

void EncountPlayer::ChangeStateAttackedEnemy(void)
{
	//攻撃を受けるアニメーションに変更
	const float animEndStep = 20.0f;
	animationController_->Play((int)ANIM_TYPE::ATTACKED, false, 0.0f, animEndStep);
	transform_.pos.z = ATTACKED_POS_Z;
	stateUpdate_ = std::bind(&EncountPlayer::UpdateAttackedEnemy, this);
}

void EncountPlayer::UpdateNone(void)
{
}

void EncountPlayer::UpdateStageWalk(void)
{
	if (transform_.pos.z <= WALK_STAGE_POS_Z)
	{
		//ゆっくり歩くアニメーション
		animationController_->Play((int)ANIM_TYPE::WALK_SLOW);
		//ゆっくり歩く処理
		movePow_ = VScale(transform_.GetForward(), WALK_SPEED_SLOW);
		movedPos_ = VAdd(transform_.pos, movePow_);
	}
	else
	{
		//移動終了
		movePow_ = CommonUtility::VECTOR_ZERO;
		//待機状態へ
		ChangeState(STATE::STAGE_WAIT);
	}

	//移動
	transform_.pos = movedPos_;
}

void EncountPlayer::UpdateStageWait(void)
{
}

void EncountPlayer::UpdateLookAround(void)
{
}

void EncountPlayer::UpdateAttackedEnemy(void)
{
}