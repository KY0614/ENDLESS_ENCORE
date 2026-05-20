#include <EffekseerForDXLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Libs/nlohmann/json.hpp"
#include "../Utility/CommonUtility.h"
#include "../Utility/StringUtility.h"
#include "../Common/Easing.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "../Manager/GameSystem/InputManager.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Common/AnimationController.h"
#include "../Common/Geometry/Capsule.h"
#include "../Common/Geometry/Sphere.h"
#include "../Common/Collider.h"
#include "../UI/HPBar.h"
#include "../UI/ParryBar.h"
#include "../ImGuiComponent/ImGuiComponentPlayer.h"
#include "Player.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONのデータのオブジェクト指定キー
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

	//行動時間
	const float DODGE_TIME = 0.4f;		//回避(無敵)時間
	const float PARRY_TIME = 0.3f;		//パリィ時間

	//ステージを歩き始める位置(Z座標)
	const float WALK_STAGE_POS_Z = 1150.0f;
	//攻撃を受けるZ座標
	const float ATTACKED_POS_Z = 2244.0f;
	const float POS_Z = -2510.0f;
	//ステージを歩くスピード
	const float WALK_SPEED_SLOW = 1.0f;
	//パリィ音量
	const int PARRY_SE_VOLUME = 70;	

	//UIの座標
	const Vector2 HP_BAR_POS = { 20, 20 };		//HPバーの位置
	const Vector2 PARRY_BAR_POS = { 50, 50 };	//パリィバーの位置

	//UIのサイズ
	const Vector2 PARRY_BAR_SIZE = { 200, 20 };	//パリィバーのサイズ
}

Player::Player(void)
{
	state_ = STATE::NONE;
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&Player::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::BATTLE_START_WAIT, std::bind(&Player::ChangeStateBattleStartWait, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BACKSTAB, std::bind(&Player::ChangeStateBackstab, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Player::ChangeStateDead, this));

	gravHitPosDown_ = CommonUtility::VECTOR_ZERO;
	gravHitPosUp_ = CommonUtility::VECTOR_ZERO;

	effectParryPlayId_ = -1;
	effectParryResId_ = -1;

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
	resultImgAlpha_ = 0;
	isActionEnd_ = false;
	stepBackstab_ = 0.0f;
	clothSE_ = false;
	victoryImg_ = -1;
	diedImg_ = -1;
}

Player::~Player(void)
{
}

void Player::Init(void)
{
	//サウンドの初期化
	InitSound();

	//コライダーの初期化
	colliders_.clear();

	victoryImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::VICTORY).handleId_;
	diedImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::YOU_DIED).handleId_;

	//3Dモデルの初期化
	Init3DModel();

	//当たり判定の初期化
	InitCollider();

	//アニメーションの設定
	InitAnimation();

	//UIの初期化
	InitUI();

	//パリィのエフェクトのリソース読み込み
	effectParryResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PARRY_EFKT).handleId_;

	imGuiComponent_ = std::make_unique<ImGuiComponentPlayer>();

	//初期状態
	ChangeState(STATE::WAKE_UP);
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

	if (transform_.pos.z <= POS_Z)transform_.pos.z = POS_Z;

	//更新ステップ
	stateUpdate_();

	//アニメーション再生
	animationController_->Update();

	transform_.Update();
}

void Player::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//丸影描画
	DrawShadow();
}

void Player::DrawBarUI(void)
{
	//HPバーの描画
	hpBar_->Draw();
	//パリィバー描画
	parryBar_->Draw();
}

void Player::DrawDead(void)
{
	//HPが0以下でアニメーションが終了している場合、死亡表記を描画
	if (hp_ <= 0.0f && animationController_->IsEnd())
	{
		//YOU DIEDの画像を描画
		DrawResultImage(diedImg_);
	}
}

void Player::DrawVictory(void)
{
	//デバッグ用勝利表記
	DrawResultImage(victoryImg_);
}

void Player::DrawResultImage(const int img)
{
	//画面の比率
	const float& screenAspectRatio =
		SceneManager::GetInstance().GetScreenAspectRatio();

	//透明度の増加値
	const int alphaSpeed = 5;
	const int maxAlpha = 255;
	const int maxInterval = 120;
	resultImgAlpha_ = std::clamp(resultImgAlpha_, 0, maxAlpha);
	if (resultImgAlpha_ >= maxAlpha)
	{
		//結果画像の透明度(静的にしているのは、状態が変わるまで値を保持するため)
		static int interval = 0;
		if (++interval > maxInterval)
		{
			interval = 0;
			SceneManager::GetInstance().ChangeScene(
				SceneManager::SCENE_ID::TITLE);
			return;
		}
	}
	resultImgAlpha_ += alphaSpeed;	//透明度を増加させる
	//透明度の上限設定
	const int AlphaMax = 255;
	if (resultImgAlpha_ > AlphaMax)resultImgAlpha_ = AlphaMax;
	const int fontSize = 64;
	const int defaultFontSize = 16;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, resultImgAlpha_);
	SetFontSize(fontSize);
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2,
		screenAspectRatio,
		0.0f,
		img,
		true);
	SetFontSize(defaultFontSize);
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

void Player::InitSound(void)
{
	//サウンドの登録
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PARRY,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PARRY_SE).handleId_);

	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::WAKE_UP,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::WAKE_UP_SE).handleId_);

	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::DAMAGE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::DAMAGE_SE).handleId_);
}

void Player::Init3DModel(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json&  playerData = jsonM.GetJsonData(
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
	const float scale = transformData.value(JsonManager::KEY_SCALE, 0.0f);
	transform_.scl = { scale ,scale ,scale };
	//transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
	transform_.pos.x = transformData.value(JsonManager::KEY_POSITION_X, 0.0f);
	transform_.pos.y = transformData.value(JsonManager::KEY_POSITION_Y, 0.0f);
	transform_.pos.z = transformData.value(JsonManager::KEY_POSITION_Z, 0.0f);
	transform_.quaRot = Quaternion();
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
	//HPを設定
	const json& paramData = playerData[JsonManager::KEY_PARAMETER];
	SetHP(paramData.value(JsonManager::KEY_HP, 0.0f));
	SetMaxHP(paramData.value(JsonManager::KEY_MAX_HP, 0.0f));
}

void Player::InitCollider(void)
{
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	const VECTOR localPosTop = { 0.0f, 110.0f, 0.0f };
	const VECTOR localPosDown = { 0.0f, 20.0f, 0.0f };
	const float capsuleRadius = 20.0f;
	capsule_->SetLocalPosTop(localPosTop);
	capsule_->SetLocalPosDown(localPosDown);
	capsule_->SetRadius(capsuleRadius);

	//球コライダ
	const VECTOR localPos = { 0.0f, 40.0f, 0.0f };
	const float sphereRadius = 80.0f;
	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos(localPos);
	sphere_->SetRadius(sphereRadius);
}

void Player::InitAnimation(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER,KEY_PLAYER);
	//データが含まれていない場合はエラーメッセージを出す
	if (!playerData.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const json& animPath = playerData[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Player/";
	const char* KEY_EMPTY = "";
	//アニメーション速度
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	//起き上がり
	animationController_->Add((int)ANIM_TYPE::WAKE_UP, path + animPath.value(KEY_WAKE_UP, KEY_EMPTY),
		animSpeed);
	//待機状態
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	//歩く
	animationController_->Add((int)ANIM_TYPE::WALK, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeed);
	//走る
	animationController_->Add((int)ANIM_TYPE::RUN, path + animPath.value(KEY_RUN, KEY_EMPTY),
		animSpeed);
	//ジャンプ
	animationController_->Add((int)ANIM_TYPE::JUMP, path + animPath.value(KEY_JUMP, KEY_EMPTY),
		animSpeed);
	//回避
	animationController_->Add((int)ANIM_TYPE::DODGE, path + animPath.value(KEY_DODGE, KEY_EMPTY),
		animSpeed);
	//バックスタブ
	animationController_->Add((int)ANIM_TYPE::BACKSTAB, path + animPath.value(KEY_BACKSTAB, KEY_EMPTY),
		animSpeed);
	//死亡
	animationController_->Add((int)ANIM_TYPE::DEATH, path + animPath.value(KEY_DEATH, KEY_EMPTY),
		animSpeed);
}

void Player::InitUI(void)
{
	//バーの大きさ（高さ）
	const int barHeight = 20;
	//HPバーの初期化
	hpBar_ = std::make_unique<HPBar>(
		HPBar::HPBarInfo{
			HPBar::TYPE::PLAYER,
			HP_BAR_POS,
			Vector2(static_cast<int>(maxHp_), barHeight)
		}, hp_);
	hpBar_->Init();

	//パリィバーの初期化
	parryBar_ = std::make_unique<ParryBar>(
		ParryBar::ParryBarInfo{
			PARRY_BAR_POS,
			PARRY_BAR_SIZE
		}, stepParry_, PARRY_TIME);
	parryBar_->Init();
}

void Player::InitBattle(void)
{
	//移動中だった場合は移動量を０にする
	movePow_ = CommonUtility::VECTOR_ZERO;
	//Jsonデータ取得
	JsonManager& jsonM = JsonManager::GetInstance();
	const json playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);
	const json& transformData = playerData.at(JsonManager::KEY_TRANSFORM);
	//パラメータを取得
	const json& paramData = playerData[JsonManager::KEY_PARAMETER];
	//座標をステージ上の端(手前側)に設定
	transform_.pos = JsonManager::GetParseVector(paramData, KEY_STAGE_POS);
	//正面を向かせる(Z軸方向)
	transform_.quaRot = Quaternion();
}

void Player::Damage(const float subHp)
{
	if (hp_ <= 0.0f)return;
	hp_ -= subHp;
	//ダメージ音再生
	SoundManager::GetInstance().Play(SoundManager::SOUND::DAMAGE);
}

void Player::SetBackstabRotY(const Quaternion& rotY)
{
	//敵の方向を向くように回転を設定
	transform_.quaRot = rotY;
	//バックスタブ終了後の回転も同じように設定
	playerRotY_ = rotY;
	goalQuaRot_ = rotY;
}

void Player::Play(void)
{
	//状態をPLAYに変更
	ChangeState(STATE::PLAY);
}

void Player::Wait(void)
{
	//戦闘開始前の状態にする
	InitBattle();
	//状態をWAITに変更
	ChangeState(STATE::BATTLE_START_WAIT);
}

void Player::Backstab(void)
{
	//状態をBACKSTABに変更
	ChangeState(STATE::BACKSTAB);
}

void Player::UpdateImGui(void)
{
	//Jsonデータ取得
	JsonManager& jsonM = JsonManager::GetInstance();
	const json& playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);

	//データが含まれていない場合はエラーメッセージを出す
	if (!playerData.contains(JsonManager::KEY_TRANSFORM))
	{
		assert(0 && "データが存在しないか不正なデータです");
	}
	//Transformデータ取得
	const json& transformData = playerData.at(JsonManager::KEY_TRANSFORM);
	//パラメータを取得
	const json& paramData = playerData.at(JsonManager::KEY_PARAMETER);

	//スライダーの説明文
	ImGui::Text(StringUtility::Wstring2UTF8(
		L"Ctrlキーを押しながらスライダーをクリックすると、\n入力ボックスに変換されます").c_str());

	//階層の指定
	std::vector<const char*> hierarchyKeys = { KEY_PLAYER.c_str(), JsonManager::KEY_PARAMETER };
	imGuiComponent_->SetTargetHierarchy(hierarchyKeys);

	//体力の上限と下限
	const float hpMin = 0.0f;
	const float hpMax = 1000.0f;
	//現在体力
	std::string sliderLabel = "HP";
	imGuiComponent_->SliderFloatWithSave(
		sliderLabel.c_str(),
		&hp_,
		hpMin,
		hpMax,
		paramData,
		JsonManager::KEY_HP);
	//最大体力
	sliderLabel = "MaxHP";
	imGuiComponent_->SliderFloatWithSave(
		sliderLabel.c_str(),
		&maxHp_,
		hpMin,
		hpMax,
		paramData,
		JsonManager::KEY_MAX_HP);

	//階層の指定
	hierarchyKeys = { KEY_PLAYER.c_str(), JsonManager::KEY_TRANSFORM };
	imGuiComponent_->SetTargetHierarchy(hierarchyKeys);
	//座標の上限と下限
	const float posMin = -10000.0f;
	const float posMax = 10000.0f;
	//現在の座標X
	sliderLabel = "PositionX";
	imGuiComponent_->SliderFloatWithSave(
		sliderLabel.c_str(),
		&transform_.pos.x,
		posMin,
		posMax,
		transformData,
		JsonManager::KEY_POSITION_X);
	//Y座標
	sliderLabel = "PositionY";
	imGuiComponent_->SliderFloatWithSave(
		sliderLabel.c_str(),
		&transform_.pos.y,
		posMin,
		posMax,
		transformData,
		JsonManager::KEY_POSITION_Y);
	//Z座標
	sliderLabel = "PositionZ";
	imGuiComponent_->SliderFloatWithSave(
		sliderLabel.c_str(),
		&transform_.pos.z,
		posMin,
		posMax,
		transformData,
		JsonManager::KEY_POSITION_Z);

	//ダメージを受けるボタン(10ダメージ)
	if (ImGui::Button("Damage"))
	{
		const float damage = 10.0f;
		Damage(damage);
	}

	std::string state = "STATE : ";
	//状態表示
	switch (state_)
	{
	case Player::STATE::NONE:
		state += "NONE";
		break;
	case Player::STATE::BATTLE_START_WAIT:
		state += "BATTLE_START_WAIT";
		break;
	case Player::STATE::PLAY:
		state += "PLAY";
		break;
	case Player::STATE::DEAD:
		state += "DEAD";
		break;
	case Player::STATE::WAKE_UP:
		state += "WAKE_UP";
		break;
	default:
		break;
	}
	ImGui::Text(state.c_str());
}

void Player::ChangeState(const STATE& state)
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

void Player::ChangeStateWakeUp(void)
{
	//起き上がりアニメーションの終了時間
	const float wakeUpEnd = 320.0f;
	//起き上がりアニメーションに変更
	animationController_->Play((int)ANIM_TYPE::WAKE_UP, false, 0.0f,wakeUpEnd);
	stateUpdate_ = std::bind(&Player::UpdateWakeUp, this);
}

void Player::ChangeStateBattleStartWait(void)
{
	stateUpdate_ = std::bind(&Player::UpdateBattleStartWait, this);
}

void Player::ChangeStatePlay(void)
{
	animationController_->Play((int)ANIM_TYPE::IDLE, true, 0.0f, -1.0f, false, true);
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateBackstab(void)
{
	//アニメーションがY軸90度分回転しているので合わせる
	const float rotY = -90.0f;
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });

	//アニメーションを途中まで再生
	const float animationEnd = 26.0f;
	animationController_->Play((int)ANIM_TYPE::BACKSTAB, false,0.0f, animationEnd);
	stateUpdate_ = std::bind(&Player::UpdateBackstab, this);
}

void Player::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Player::UpdateDead, this);
}

void Player::UpdateNone(void)
{//何もしない
}

void Player::UpdateWakeUp(void)
{
	if (isActionEnd_)return;
	if (!clothSE_)
	{
		SoundManager& sound = SoundManager::GetInstance();
		sound.Play(SoundManager::SOUND::WAKE_UP);
		clothSE_ = true;
	}
	//起き上がりアニメーションが終了したら待機状態へ移行
	if(animationController_->IsEnd())
	{
		isActionEnd_ = true;
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}
}

void Player::UpdateBattleStartWait(void)
{
	//重力による移動量
	CalcGravityPow();

	//衝突判定
	Collision();

	if(animationController_->IsEnd())
	{
		//アニメーションが終了したら待機アニメーションに移行
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}
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

	//パリィエフェクト位置更新
	EffectParryPosUpdate();

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);
}

void Player::UpdateBackstab(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//続きを再生させるための待ち時間
	const float waitTime = 0.6f;

	//途中までの再生が終わったら経過時間まで待ち、
	//残りのアニメーションを再生する
	if (animationController_->IsEnd())
	{
		stepBackstab_ += SceneManager::GetInstance().GetDeltaTime();
		if (stepBackstab_ > waitTime && !isActionEnd_)
		{
			isActionEnd_ = true;
			//アニメーションの続きを再生
			const float animStartStep = 26.0f;
			const float animEndStep = 100.0f;
			animationController_->Play((int)ANIM_TYPE::BACKSTAB, false, animStartStep, animEndStep, false, true);
		}
	}
	//バックスタブアニメーションが終了したらローカル回転を元に戻す
	if (isActionEnd_ && animationController_->IsEnd())
	{
		//アニメーション用に変更したローカル回転を元に戻す
		const json playerData = jsonM.GetJsonData(
			JsonManager::JSON_DATA::PLAYER,KEY_PLAYER);
		const json& transformData = playerData[JsonManager::KEY_TRANSFORM];
		const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
		transform_.quaRotLocal =
			Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
		
		stepBackstab_ = 0.0f;
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

		//ダッシュジャンプの飛距離を出すために、水平方向の移動速度を初速に加算
		jumpPow_.x = movePow_.x * JUMP_POW_DECEL_RATE;
		jumpPow_.z = movePow_.z * JUMP_POW_DECEL_RATE;

		//無理やりアニメーション
		const float animStartStep = 13.0f;
		const float animEndStep = 25.0f;
		animationController_->Play((int)ANIM_TYPE::JUMP, true, animStartStep, animEndStep);
		const float animLoopStep = 23.0f;
		const float animLoopSpeed = 5.0f;
		animationController_->SetEndLoop(animLoopStep, animEndStep, animLoopSpeed);
	}
}

void Player::ProcessDodge(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Dodge");
	//アニメーションの途中まで再生してループさせるための値
	const float animEndStep = 4.0f;		//回避アニメーションの終了位置
	const float animLoopSpeed = 5.0f;	//ループ再生速度

	//回避
	if (isHit && !isJump_)
	{
		//回避中はスピードを早くする
		isDodge_ = true;
		speed_ = SPEED_DODGE;
		//アニメーションを途中まで再生してループさせる
		animationController_->Play((int)ANIM_TYPE::DODGE, true, 0.0f, animEndStep);
		animationController_->SetEndLoop(animEndStep, animEndStep, animLoopSpeed);
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
		//アニメーションを途中から再生
		animationController_->Play((int)ANIM_TYPE::DODGE,true, animEndStep,-1.0f,false,true);
	}
	//速度減速処理
	if (isDecelerate_)
	{
		stepWalk_ = STEP_WALK2RUN;
		//減速処理
		//１を最大として、減速時間に応じて0から1までの割合を求める
		float decelRate = (stepDodge_ - DODGE_TIME) / DODGE_DECELERATION_TIME;
		const float maxRate = 1.0f;	//最大値
		float currentSpeed = Easing::CubicOut(decelRate,
			maxRate, SPEED_DODGE, SPEED_RUN);
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
	SoundManager& sound = SoundManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Parry");
	if (isHit && !isParry_)
	{
		sound.AdjustVolume(SoundManager::SOUND::PARRY, PARRY_SE_VOLUME);
		sound.Play(SoundManager::SOUND::PARRY);
		isParry_ = true;
		//パリィエフェクト再生
		EffectParry();
	}

	if (!isParry_)return;
	//パリィ時間経過判定
	stepParry_ += SceneManager::GetInstance().GetDeltaTime();
	//パリィクールタイム終了
	if(stepParry_ > PARRY_TIME)
	{
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
			rotRad, CommonUtility::AXIS_Y);
	
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

	//衝突(重力)
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
	for (const std::weak_ptr<Collider> c : colliders_)
	{
		MV1_COLL_RESULT_POLY_DIM hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());
		//衝突した複数のポリゴンと衝突回避するまで、
		//プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			MV1_COLL_RESULT_POLY hit = hits.Dim[i];
			//地面と異なり、衝突回避位置が不明なため、何度か移動させる
			//この時、移動させる方向は、移動前座標に向いた方向であったり、
			//衝突したポリゴンの法線方向だったりする
			const int maxTryCnt = 10;
			for (int tryCnt = 0; tryCnt < maxTryCnt; tryCnt++)
			{
				//再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				//最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					//法線の方向にちょっとだけ移動させる
					const float adjustDist = 2.0f;
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, adjustDist));
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
	//ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	//重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;

	//重力方向の反対
	VECTOR dirUpGravity = CommonUtility::DIR_U;

	//重力の強さ
	float gravityPow = GRAVITY_POW;
	//重力落下チェック用の長さ
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const std::weak_ptr<Collider> c : colliders_)
	{
		//地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = CommonUtility::VECTOR_ZERO;\
			if (isJump_)
			{
				//ジャンプアニメーションを途中から再生
				const float animStartStep = 29.0f;
				const float animEndStep = 45.0f;
				//着地モーション
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, animStartStep, animEndStep, false, true);
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
		//地面にいる場合はジャンプ力をリセット
		jumpPow_ = CommonUtility::VECTOR_ZERO;

		//重力方向
		VECTOR dirGravity = CommonUtility::DIR_D;

		//重力の強さ
		float gravityPow = GRAVITY_POW;

		//重力
		VECTOR gravity = VScale(dirGravity, gravityPow);
		jumpPow_ = VAdd(jumpPow_, gravity);

		//内積
		float dot = VDot(dirGravity, jumpPow_);
		if (dot >= 0.0f)
		{
			//重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
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

void Player::EffectParry(void)
{
	//すでに再生中なら何もしない
	if (IsEffekseer3DEffectPlaying(effectParryPlayId_) > -1)return;

	//再生Idを取得
	effectParryPlayId_ = PlayEffekseer3DEffect(effectParryResId_);

	//再生速度の設定(少し早めに設定）
	const float effectParrySpeed = 6.0f;
	SetSpeedPlayingEffekseer3DEffect(effectParryPlayId_, effectParrySpeed);

	//大きさの設定
	//大きさ
	float EFFEKT_SCALE = 20.0f;
	float EFFEKT_SCALE_Y = 26.0f;
	SetScalePlayingEffekseer3DEffect(
		effectParryPlayId_,
		EFFEKT_SCALE,
		EFFEKT_SCALE_Y,
		EFFEKT_SCALE
	);
}

void Player::EffectParryPosUpdate(void)
{
	//エフェクトの位置をプレイヤーの位置に設定
	SetPosPlayingEffekseer3DEffect(
		effectParryPlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}