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
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Common/AnimationController.h"
#include "../Common/Collider/ColliderLine.h"
#include "../Common/Collider/ColliderSphere.h"
#include "../Common/Collider/ColliderCapsule.h"
#include "../Common/Geometry/Capsule.h"
#include "../Common/Geometry/Sphere.h"
#include "../Common/Collider.h"
#include "../UI/HPBar.h"
#include "../UI/ParryBar.h"
#include "PlayerTest.h"

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
	// 衝突判定用カプセル上部球体(ジャンプ時)
	const VECTOR COL_CAPSULE_TOP_JUMP_LOCAL_POS =
	{ 0.0f, 160.0f, 0.0f };
	// 衝突判定用カプセル下部球体(ジャンプ時)
	const VECTOR COL_CAPSULE_DOWN_JUMP_LOCAL_POS =
	{ 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分開始(ジャンプ時)
	const VECTOR COL_LINE_JUMP_START_LOCAL_POS =
	{ 0.0f, 130.0f, 0.0f };
	// 衝突判定用線分終了(ジャンプ時)
	const VECTOR COL_LINE_JUMP_END_LOCAL_POS =
	{ 0.0f, 35.0f, 0.0f };
	// 衝突判定用線分開始
	const VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	const VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	//回転完了までの時間
	const float TIME_ROT = 0.3f;

	//煙エフェクト発生間隔
	const float TERM_FOOT_SMOKE = 0.3f;

	//ジャンプ力
	const float JUMP_POW = 18.0f;
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

PlayerTest::PlayerTest(void)
{
	state_ = STATE::NONE;
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&PlayerTest::ChangeStateNone, this));
	stateChanges_.emplace(STATE::WAKE_UP, std::bind(&PlayerTest::ChangeStateWakeUp, this));
	stateChanges_.emplace(STATE::WAIT, std::bind(&PlayerTest::ChangeStateWait, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&PlayerTest::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BACKSTAB, std::bind(&PlayerTest::ChangeStateBackstab, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&PlayerTest::ChangeStateDead, this));

	gravHitPosDown_ = CommonUtility::VECTOR_ZERO;
	gravHitPosUp_ = CommonUtility::VECTOR_ZERO;

	effectSmokePlayId_ = -1;
	effectSmokeResId_ = -1;
	stepFootSmoke_ = -1.0f;

	effectParryPlayId_ = -1;
	effectParryResId_ = -1;

	stepJump_ = -1.0f;
	isJump_ = false;
	speed_ = -1.0f;
	
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

PlayerTest::~PlayerTest(void)
{
}

void PlayerTest::Init(void)
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

	//足煙エフェクト
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;

	//足煙エフェクトの発生間隔
	stepFootSmoke_ = TERM_FOOT_SMOKE;

	//パリィのエフェクトのリソース読み込み
	effectParryResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PARRY_EFKT).handleId_;

	//初期状態
	ChangeState(STATE::WAKE_UP);
}

void PlayerTest::UpdateState(void)
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
}

void PlayerTest::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//丸影描画
	DrawShadow();

	for (const auto& col : ownColliders_)
	{
		col.second->Draw();
	}
}

void PlayerTest::DrawBarUI(void)
{
	//HPバーの描画
	hpBar_->Draw();
	//パリィバー描画
	parryBar_->Draw();
}

void PlayerTest::DrawDead(void)
{
	//HPが0以下でアニメーションが終了している場合、死亡表記を描画
	if (hp_ <= 0.0f && animationController_->IsEnd())
	{
		//YOU DIEDの画像を描画
		DrawResultImage(diedImg_);
	}
}

void PlayerTest::DrawVictory(void)
{
	//デバッグ用勝利表記
	DrawResultImage(victoryImg_);
}

void PlayerTest::DrawResultImage(const int img)
{
	//画面の比率
	const float& screenAspectRatio =
		SceneManager::GetInstance().GetScreenAspectRatio();

	static int interval = 0;
	//透明度の増加値
	const int alphaSpeed = 5;
	const int maxAlpha = 255;
	const int maxInterval = 120;
	resultImgAlpha_ = std::clamp(resultImgAlpha_, 0, maxAlpha);
	if (resultImgAlpha_ >= maxAlpha)
	{
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

void PlayerTest::ClearCollider(void)
{
	colliders_.clear();
}

Capsule& PlayerTest::GetCapsule(void) const
{
	return *capsule_;
}

const Sphere& PlayerTest::GetSphere(void) const
{
	return *sphere_;
}

bool PlayerTest::IsPlay(void) const
{
	//状態がPLAYかどうかを返す
	return state_ == STATE::PLAY;
}

void PlayerTest::InitSound(void)
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

void PlayerTest::Init3DModel(void)
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
	const float scale = transformData.value(JsonManager::KEY_SCALE, 0.0f);
	transform_.scl = { scale ,scale ,scale };
	transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
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

void PlayerTest::InitCollider(void)
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

	// 主に地面との衝突で仕様する線分コライダ
	std::unique_ptr<ColliderLine> colLine = std::make_unique<ColliderLine>(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), std::move(colLine));

	//カプセルコライダ
	std::unique_ptr<ColliderCapsule> colCap = std::make_unique<ColliderCapsule>(
		ColliderBase::TAG::PLAYER, &transform_,
		localPosTop, localPosDown, capsuleRadius);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), std::move(colCap));

	//球コライダ
	std::unique_ptr<ColliderSphere> colSphere = std::make_unique<ColliderSphere>(
		ColliderBase::TAG::PLAYER_PARRY, &transform_,
		localPos, sphereRadius);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::SPHERE), std::move(colSphere));
}

void PlayerTest::InitAnimation(void)
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

void PlayerTest::InitUI(void)
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

void PlayerTest::InitBattle(void)
{
	//移動中だった場合は移動量を０にする
	movePow_ = CommonUtility::VECTOR_ZERO;
	//Jsonデータ取得
	JsonManager& jsonM = JsonManager::GetInstance();
	const json playerData = jsonM.GetJsonData(
		JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);
	//パラメータを取得
	const json& paramData = playerData[JsonManager::KEY_PARAMETER];
	//座標をステージ上の端(手前側)に設定
	transform_.pos = JsonManager::GetParseVector(paramData, KEY_STAGE_POS);
	//正面を向かせる(Z軸方向)
	transform_.quaRot = Quaternion();
}

void PlayerTest::Damage(float subHp)
{
	if (hp_ <= 0.0f)return;
	hp_ -= subHp;
	//ダメージ音再生
	SoundManager::GetInstance().Play(SoundManager::SOUND::DAMAGE);
}

void PlayerTest::SetBackstabRotY(const Quaternion& rotY)
{
	//敵の方向を向くように回転を設定
	transform_.quaRot = rotY;
	//バックスタブ終了後の回転も同じように設定
	playerRotY_ = rotY;
	goalQuaRot_ = rotY;
}

void PlayerTest::Play(void)
{
	//状態をPLAYに変更
	ChangeState(STATE::PLAY);
}

void PlayerTest::Wait(void)
{
	//戦闘開始前の状態にする
	InitBattle();
	//状態をWAITに変更
	ChangeState(STATE::WAIT);
}

void PlayerTest::Backstab(void)
{
	//状態をBACKSTABに変更
	ChangeState(STATE::BACKSTAB);
}

void PlayerTest::UpdateImGui(void)
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
	//座標
	const float posMin = -10000.0f;
	const float posMax = 10000.0f;
	ImGui::SliderFloat("PosX", &transform_.pos.x, posMin, posMax);
	//保存ボタン(スライドの横に配置)
	ImGui::SameLine();
	if (ImGui::Button(StringUtility::Wstring2UTF8(L"保存").c_str()))
	{
		ImGui::OpenPopup("Save Confirmation");
	}
	//元に戻すボタン(保存ボタンの横に配置)
	ImGui::SameLine();
	if (ImGui::Button(StringUtility::Wstring2UTF8(L"元に戻す").c_str()))
	{
		transform_.pos.x = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION).x;
	}

	//ポップアップの処理
	if (ImGui::BeginPopupModal(
		"Save Confirmation",
		NULL,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(StringUtility::Wstring2UTF8(
			L"変更した内容を保存しますか？").c_str());
		ImGui::Text(StringUtility::Wstring2UTF8(L"変更内容：%.2ff →　%.2ff").c_str(),
			JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION).x,
			transform_.pos.x);
		const float buttonWidth = 120.0f; // ボタンの横幅
		const float windowWidth = ImGui::GetWindowSize().x; // 現在のウィンドウの横幅
		const float posX = (windowWidth - (buttonWidth * 2)) / 2.0f; // 中央位置を計算
		ImGui::SetCursorPosX(posX);// ボタンを中央に配置
		if (ImGui::Button("SAVE", ImVec2(buttonWidth, 0)))
		{
			//保存（データを上書き）
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("CANCEL", ImVec2(buttonWidth, 0)))
		{
			transform_.pos.x = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION).x;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::SliderFloat("PosY", &transform_.pos.y, posMin, posMax);
	ImGui::SliderFloat("PosZ", &transform_.pos.z, posMin, posMax);

	ImGui::SliderFloat("JumpX", &jumpPow_.x, posMin, posMax);
	ImGui::SliderFloat("JumpY", &jumpPow_.y, posMin, posMax);
	ImGui::SliderFloat("JumpZ", &jumpPow_.z, posMin, posMax);

	//Jsonデータに保存するボタン
	if (ImGui::Button("Save to Json"))
	{
		//保存処理
	}

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
	case PlayerTest::STATE::NONE:
		state += "NONE";
		break;
	case PlayerTest::STATE::WAIT:
		state += "WAIT";
		break;
	case PlayerTest::STATE::PLAY:
		state += "PLAY";
		break;
	case PlayerTest::STATE::DEAD:
		state += "DEAD";
		break;
	case PlayerTest::STATE::WAKE_UP:
		state += "WAKE_UP";
		break;
	default:
		break;
	}
	ImGui::Text(state.c_str());
}

void PlayerTest::UpdateProcess(void)
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
	//animationController_->Update();

	//transform_.Update();
}

void PlayerTest::UpdateProcessPost(void)
{
}

void PlayerTest::ChangeState(const STATE& state)
{
	//行動終了判定をリセット
	isActionEnd_ = false;
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void PlayerTest::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&PlayerTest::UpdateNone, this);
}

void PlayerTest::ChangeStateWakeUp(void)
{
	//起き上がりアニメーションの終了時間
	const float wakeUpEnd = 320.0f;
	//起き上がりアニメーションに変更
	animationController_->Play((int)ANIM_TYPE::WAKE_UP, false, 0.0f, wakeUpEnd);
	stateUpdate_ = std::bind(&PlayerTest::UpdateWakeUp, this);
}

void PlayerTest::ChangeStateWait(void)
{
	stateUpdate_ = std::bind(&PlayerTest::UpdateWait, this);
}

void PlayerTest::ChangeStatePlay(void)
{
	animationController_->Play((int)ANIM_TYPE::IDLE, true, 0.0f, -1.0f, false, true);
	stateUpdate_ = std::bind(&PlayerTest::UpdatePlay, this);
}

void PlayerTest::ChangeStateBackstab(void)
{
	//アニメーションがY軸90度分回転しているので合わせる
	const float rotY = -90.0f;
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });

	//アニメーションを途中まで再生
	const float animationEnd = 26.0f;
	animationController_->Play((int)ANIM_TYPE::BACKSTAB, false, 0.0f, animationEnd);
	stateUpdate_ = std::bind(&PlayerTest::UpdateBackstab, this);
}

void PlayerTest::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&PlayerTest::UpdateDead, this);
}

void PlayerTest::UpdateNone(void)
{//何もしない
}

void PlayerTest::UpdateWakeUp(void)
{
	if (isActionEnd_)return;
	if (!clothSE_)
	{
		SoundManager& sound = SoundManager::GetInstance();
		sound.Play(SoundManager::SOUND::WAKE_UP);
		clothSE_ = true;
	}
	//起き上がりアニメーションが終了したら待機状態へ移行
	if (animationController_->IsEnd())
	{
		isActionEnd_ = true;
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}
}

void PlayerTest::UpdateWait(void)
{
	//重力による移動量
	//CalcGravityPow();

	//衝突判定
	//Collision();

	if (animationController_->IsEnd())
	{
		//アニメーションが終了したら待機アニメーションに移行
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}
}

void PlayerTest::UpdatePlay(void)
{
	if (hp_ <= 0.0f)
	{
		ChangeState(STATE::DEAD);
		return;
	}

	//移動処理
	ProcessMove();

	//ジャンプ処理
	ProcessJump();

	//アニメーションごとの線分調整
	CollisionReserve();

	//回避処理
	ProcessDodge();

	//パリィ処理
	ProcessParry();

	//移動方向に応じた回転
	Rotate();

	//歩きエフェクト
	//EffectFootSmoke();

	//パリィエフェクト位置更新
	EffectParryPosUpdate();

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);
}

void PlayerTest::UpdateBackstab(void)
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
			JsonManager::JSON_DATA::PLAYER, KEY_PLAYER);
		const json& transformData = playerData[JsonManager::KEY_TRANSFORM];
		const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
		transform_.quaRotLocal =
			Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });

		stepBackstab_ = 0.0f;
		ChangeState(STATE::PLAY);
		return;
	}
}

void PlayerTest::UpdateDead(void)
{
	hp_ = std::clamp(hp_, 0.0f, maxHp_);
	animationController_->Play((int)ANIM_TYPE::DEATH, false);
}

void PlayerTest::ProcessMove(void)
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

void PlayerTest::ProcessJump(void)
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

void PlayerTest::ProcessDodge(void)
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
		animationController_->Play((int)ANIM_TYPE::DODGE, true, animEndStep, -1.0f, false, true);
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

void PlayerTest::ProcessParry(void)
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
	if (stepParry_ > PARRY_TIME)
	{
		isParry_ = false;
		stepParry_ = 0.0f;
	}
}

void PlayerTest::SetGoalRotate(double rotRad)
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

void PlayerTest::Rotate(void)
{
	//回転時間の減少
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();

	//回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

void PlayerTest::CollisionReserve(void)
{
	// アニメーションごとの線分調整
	if (animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP)&&
		isJump_)
	{
		// ジャンプ中は線分を伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)).get()
				);
			colLine->SetLocalPosStart(COL_LINE_JUMP_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_JUMP_END_LOCAL_POS);
			//カプセルも同様に調整
			ColliderCapsule* colCap = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)).get()
				);
			colCap->SetLocalPosTop(COL_LINE_JUMP_START_LOCAL_POS);
			colCap->SetLocalPosDown(COL_LINE_JUMP_END_LOCAL_POS);	
		}
	}
	else
	{
		// 通常時の線分に戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)).get()
				);
			colLine->SetLocalPosStart(COL_LINE_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_END_LOCAL_POS);
			//カプセルも同様に戻す
			ColliderCapsule* colCap = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)).get()
				);
			const VECTOR localPosTop = { 0.0f, 110.0f, 0.0f };
			const VECTOR localPosDown = { 0.0f, 20.0f, 0.0f };
			colCap->SetLocalPosTop(localPosTop);
			colCap->SetLocalPosDown(localPosDown);
		}
	}
}

void PlayerTest::JumpAnimationPlay(void)
{
	//ジャンプアニメーションを途中から再生
	const float animStartStep = 29.0f;
	const float animEndStep = 45.0f;
	//着地モーション
	animationController_->Play(
		(int)ANIM_TYPE::JUMP, false, animStartStep, animEndStep, false, true);
}

bool PlayerTest::IsEndLanding(void) const
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

bool PlayerTest::IsEndDodge(void) const
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

void PlayerTest::EffectFootSmoke(void)
{
	stepFootSmoke_ -= SceneManager::GetInstance().GetDeltaTime();

	float len = CommonUtility::MagnitudeF(moveDiff_);

	if (len >= 1.0f &&
		stepFootSmoke_ < 0.0f)
	{

		stepFootSmoke_ = TERM_FOOT_SMOKE;

		//エフェクト再生
		effectSmokePlayId_ = PlayEffekseer3DEffect(effectSmokeResId_);

		//大きさ
		const float SCALE = 5.0f;
		SetScalePlayingEffekseer3DEffect(effectSmokePlayId_, SCALE, SCALE, SCALE);

		//位置の設定
		SetPosPlayingEffekseer3DEffect(
			effectSmokePlayId_,
			transform_.pos.x,
			transform_.pos.y,
			transform_.pos.z);
	}
}

void PlayerTest::EffectParry(void)
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

void PlayerTest::EffectParryPosUpdate(void)
{
	//エフェクトの位置をプレイヤーの位置に設定
	SetPosPlayingEffekseer3DEffect(
		effectParryPlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}