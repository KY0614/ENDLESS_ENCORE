#include <random>
#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "Common/AnimationController.h"
#include "Common/Geometry/Capsule.h"
#include "Common/Geometry/Sphere.h"
#include "Player.h"
#include "EnemyBullet.h"
#include "Enemy.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_ENEMY = "Enemy";
	static const std::string KEY_IDLE = "Idle";
	static const std::string KEY_WALK = "Walk";
	static const std::string KEY_RUN = "Run";
	static const std::string KEY_ATK_NEAR = "Attack_Near";
	static const std::string KEY_ATK_FAR_ONE = "Attack_Far_One";
	static const std::string KEY_ATK_FAR_ALL = "Attack_Far_All";
	static const std::string KEY_DAMAGE = "Damage";
	static const std::string KEY_DOWN = "Down";
	static const std::string KEY_BACKSTAB = "Backstab";
	static const std::string KEY_MAGIC_IDLE = "Magic Idle";
	static const std::string KEY_CAST_SPELL = "Cast Spell";
	static const std::string KEY_STAND_UP = "Stand Up";
	static const std::string KEY_DEATH = "Death";
	static const std::string KEY_MAX_HP = "maxHp";
	//回転にかける時間
	const float TIME_ROT = 0.1f;
	//敵の基本パラメータ
	const float HP_MAX = 100.0f;	//最大HP	
	const float MOVE_SPEED = 13.0f;	//移動速度
	//距離の基準値
	const float ATTACK_NEAR_DISTANCE = 350.0f;	//近距離攻撃判定距離
	const float ATTACK_FAR_DISTANCE = 800.0f;	//遠距離攻撃判定距離
	const float PLAYER_DISTANCE = 600.0f;		//維持するプレイヤーとの距離
	const float FOLLOW_DISTANCE = 800.0f;		//追従距離
	//重力加速度
	const float FOLLOW_TIME = 5.0f;
	const float MOVE_TIME = 3.0f;
	const float ATTACK_TIME = 1.0f;
	const float ATTACK_FAR_TIME = 15.0f;
	const float ATTACK_CHARGE_TIME = 30.0f;

	const float ATTACK_DAMAGE = 10.0f;

	//アニメーション再生速度
	const float ANIM_SPEED = 30.0f;
	//ダウンする時間
	const float DOWN_TIME = 4.0f;

	const float VIEW_ANGLE = 40.0f;
	const float VIEW_RANGE = 100.0f;
}

Enemy::Enemy(Player& player):player_(player)
{
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	stateStep_ = 0.0f;
	state_ = STATE::NONE;
	col_ = 0xff0000;
	isAttackedNear_ = false;
	isCast_ = false;
	isDown_ = false;
	hitCount_ = 0;
	currentAngle_ = 0.0f;               // 初期角度は適当に設定 (atan2で初期化しても良い)
	stepDownTime_ = 0.0f;
	// 例: 1秒で 90度（π/2 ラジアン）回転する速度
	circlingSpeedRad_ = DX_PI_F / 2.0f  * 0.1f;
	moveDir_ = CommonUtility::VECTOR_ZERO;
	stepRotTime_ = 0.0f;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Enemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::FOLLOW, std::bind(&Enemy::ChangeStateFollow, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Enemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK_NEAR, std::bind(&Enemy::ChangeStateAttackNear, this));
	stateChanges_.emplace(STATE::SHOT_ONE, std::bind(&Enemy::ChangeStateShotOne, this));
	stateChanges_.emplace(STATE::SHOT_ALL, std::bind(&Enemy::ChangeStateShotAll, this));
	stateChanges_.emplace(STATE::ATTACK_CHARGE, std::bind(&Enemy::ChangeStateAttackCharge, this));
	stateChanges_.emplace(STATE::BACKSTAB, std::bind(&Enemy::ChangeStateBackstab, this));
	stateChanges_.emplace(STATE::DOWN, std::bind(&Enemy::ChangeStateDown, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Enemy::ChangeStateDead, this));
}

Enemy::~Enemy(void)
{
}

void Enemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();

	//当たり判定の初期化
	InitCollider();

	//アニメーションの初期化
	InitAnimation();

	//初期の状態を設定
	ChangeState(STATE::MOVE);
}

void Enemy::Update(void)
{
	//死亡判定
	if(hp_ <= 0.0f)
	{
		hitCount_ = 0;
		bullets_.clear();
		ChangeState(STATE::DEAD);
	}

	//更新ステップ
	stateUpdate_();

	animationController_->Update();
	transform_.Update();
}

void Enemy::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	VECTOR pos = ConvWorldPosToScreenPos(transform_.pos);

	for(const auto& bullet : bullets_)
	{
		bullet->Draw();
	}

#ifdef _DEBUG
	VECTOR linePos = VAdd(transform_.pos, VGet(0.0f, 150.0f, 0.0f));
	VECTOR forward = VScale(transform_.GetForward(), 100.0f);
	VECTOR right = VScale(transform_.GetRight(), 120.0f);
	forward.y += 150.0f;
	right.y += 150.0f;
	DrawLine3D(linePos, VAdd(transform_.pos, forward), 0x00ffff);
	DrawLine3D(linePos, VAdd(transform_.pos, right), 0xff0000);
	
	DrawSphere3D(VAdd(transform_.pos, right), 10.0f, 16, 0xFFFFFF, 0xFFFFFF, true);
	if (!bullets_.empty()) {
		DrawFormatString(0, 200, 0xffffff, L"E X: %.2f Y: %.2f Z: %.2f",
			bullets_[0]->GetTransform().pos.x, bullets_[0]->GetTransform().pos.y, bullets_[0]->GetTransform().pos.z);
	}
	switch (state_)
	{
	case Enemy::STATE::NONE:
		break;
	case Enemy::STATE::FOLLOW:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"FOLLOW");
		break;
	case Enemy::STATE::MOVE:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"MOVE");
		break;
	case Enemy::STATE::ATTACK_NEAR:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"ATTACK_NEAR");
		break;
	case Enemy::STATE::SHOT_ONE:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"SHOT_ONE");
		break;
	case Enemy::STATE::SHOT_ALL:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"SHOT_ALL");
		break;
	case Enemy::STATE::ATTACK_CHARGE:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"ATTACK_CHARGE");
		break;
	case Enemy::STATE::DOWN:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"DOWN");
		break;
	default:
		break;
	}
	DrawFormatString(0, 160, 0xffffff, L"E HP : %.2f", hp_);

	if(!isAttackedNear_)col_= 0x00ff00;
	else col_ = 0xff0000;
	sphereNear_->Draw(col_);

	const int HP_BAR_X = pos.x;         // HPバーの左上X座標
	const int HP_BAR_Y = pos.z + 100.0f;         // HPバーの左上Y座標
	const int HP_BAR_WIDTH = 200;    // HPバーの最大幅
	const int HP_BAR_HEIGHT = 20;    // HPバーの高さ
	float hp = hp_ / HP_MAX;
	int barWidth = static_cast<int>(HP_BAR_WIDTH * hp);
	// 背景（グレー）
	DrawBox(HP_BAR_X, HP_BAR_Y, HP_BAR_X + HP_BAR_WIDTH, HP_BAR_Y + HP_BAR_HEIGHT, GetColor(100, 100, 100), TRUE);
	// 現在HP（赤）
	DrawBox(HP_BAR_X, HP_BAR_Y, HP_BAR_X + barWidth, HP_BAR_Y + HP_BAR_HEIGHT, GetColor(255, 0, 0), TRUE);

	DrawDebug();
#endif // _DEBUG

}

void Enemy::DebugUpdate(void)
{
	//更新ステップ
	stateUpdate_();

	animationController_->Update();
	transform_.Update();

	UpdateDebugImGui();
}

void Enemy::ChangeState(const STATE state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

const bool Enemy::GetIsDead(void) const
{
	return state_ == STATE::DEAD && animationController_->IsEnd();
}

void Enemy::Init3DModel(void)
{
	auto& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::ENEMY);

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(KEY_ENEMY))assert(0 && "データが存在しないか不正なデータです");
	const auto& param = data[KEY_ENEMY];

	//データが含まれていない場合はエラーメッセージを出す
	if (!param.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const auto& transformData = param[JsonManager::KEY_TRANSFORM];

	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ENEMY));
	MV1SetMaterialDifColor(transform_.modelId, 0, GetColorF(
		175.0f / 255.0f, 175.0f / 255.0f, 125.0f / 255.0f, 1.0f));
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

void Enemy::InitCollider(void)
{
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 140.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 20.0f, 0.0f });
	capsule_->SetRadius(30.0f);

	//近接攻撃用の球体コライダ
	sphereNear_ = std::make_unique<Sphere>(transform_);
	sphereNear_->SetLocalPos({ 0.0f, 80.0f, 50.0f });
	sphereNear_->SetRadius(30.0f);

	col_ = 0x000000;
}

void Enemy::InitAnimation(void)
{
	auto& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得w
	const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::ENEMY);
	const auto& param = data[KEY_ENEMY];
	//データが含まれていない場合はエラーメッセージを出す
	if (!param.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const auto& animPath = param[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/Animation/";
	const char* KEY_EMPTY = "";
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::WALK, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::RUN, path + animPath.value(KEY_RUN, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK_NEAR, path + animPath.value(KEY_ATK_NEAR, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::MAGIC_ILDE, path + animPath.value(KEY_MAGIC_IDLE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::CAST_SPELL, path + animPath.value(KEY_CAST_SPELL, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK_FAR_ONE, path + animPath.value(KEY_ATK_FAR_ONE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK_FAR_ALL, path + animPath.value(KEY_ATK_FAR_ALL, KEY_EMPTY),
		60.0f);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + animPath.value(KEY_DAMAGE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::BACKSTAB, path + animPath.value(KEY_BACKSTAB, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::DOWN, path + animPath.value(KEY_DOWN, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + animPath.value(KEY_DEATH, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::STAND_UP, path + animPath.value(KEY_STAND_UP, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::STAND_UP, path + animPath.value(KEY_STAND_UP, KEY_EMPTY),
		animSpeed);
	//初期アニメーションはアイドルを再生
	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Enemy::Damage(void)
{
	//ダメージ処理
	hp_ -= 10.0f;
}

void Enemy::Move(void)
{
	if(CheckPlayerDistance() > PLAYER_DISTANCE)
	{
		//プレイヤーに近づく
		FollowPlayer(transform_.pos);
		ChangeState(STATE::FOLLOW);
	}
	else
	{
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}

	static float stepTime = 0.0f;
	stepTime += SceneManager::GetInstance().GetDeltaTime();

	if(stepTime > 2.0f)
	{
		stepTime = 0.0f;
		moveDir_ = CommonUtility::DIR_R;
	}

	//// 1. 角度を更新する
	////時間経過で角度を変化させます。プレイヤーの周りを右回り（時計回り）で動く。
	//currentAngle_ += circlingSpeedRad_ * SceneManager::GetInstance().GetDeltaTime();

	//// 角度が一周したらリセット (省略可)
	//if (currentAngle_ > DX_PI_F * 2.0f)
	//{
	//	currentAngle_ -= DX_PI_F * 2.0f;
	//}

	//// 2. プレイヤーの周りの円上の座標を計算する
	//VECTOR playerPos = player_.GetTransform().pos;

	//// X-Z平面での円運動の計算 (極座標からデカルト座標への変換)
	//// X = R * sin(θ)
	//// Z = R * cos(θ)

	//// プレイヤーからの相対位置
	//VECTOR relativePos;
	//relativePos.x = PLAYER_DISTANCE * sinf(currentAngle_);
	//relativePos.y = 0.0f; // プレイヤーの高さと合わせる
	//relativePos.z = PLAYER_DISTANCE * cosf(currentAngle_);

	////3. 敵のワールド座標を決定する
	////プレイヤーの位置 + プレイヤーからの相対位置
	//transform_.pos = VAdd(playerPos, relativePos);

	////4.プレイヤーの方を向く処理
	////移動した新しい位置からプレイヤーの方を向くように回転角度を計算し直す

	////敵からプレイヤーへのベクトル (このベクトルは原点(0,0,0)を向くベクトルと180度ずれている)
	////正しいターゲット方向ベクトルは relativePos の逆ベクトルになる
	//VECTOR posE2P = VScale(relativePos, -1.0f);

	////atan2 で角度を計算
	//float angle = atan2(posE2P.x, posE2P.z);

	//SetGoalRotate(angle);
}

float Enemy::CheckPlayerDistance(void)
{
	//プレイヤーとの距離を測る
	VECTOR playerToEnemy = VSub(player_.GetTransform().pos, transform_.pos);
	//ベクトルの大きさを測る
	float distance = VSize(playerToEnemy);
	return distance;	//距離を返す
}

bool Enemy::IsCastSpell(void)
{
	bool ret = true;

	// アニメーションが終了しているか
	if (animationController_->IsEnd() && animationController_->GetPlayType() == (int)ANIM_TYPE::CAST_SPELL)
	{
		return ret;	//終了している
	}

	return false;
}

bool Enemy::CheckBackstab(void)
{
	// プレイヤーの座標を取得
	VECTOR pPos = player_.GetTransform().pos;

	//エネミーからプレイヤーまでのベクトル
	VECTOR diff = VSub(pPos, transform_.pos);

	//視野範囲にはいっているか判断(ピタゴラスの定理）
	float distance = std::pow(diff.x, 2.0f) + std::pow(diff.z, 2.0f);
	if (distance <= (std::pow(VIEW_RANGE, 2.0f)))
	{

		//自分から見たプレイヤーの角度を求める
		float rad = atan2(pPos.x - transform_.pos.x, pPos.z - transform_.pos.z);
		float viewRad = rad - transform_.rot.y;
		float viewDeg = static_cast<float>
			(CommonUtility::DegIn360(CommonUtility::Rad2DegF(viewRad)));

		//視野角内に入っているか判断
		//if (viewDeg <= VIEW_ANGLE || viewDeg >= (360.0f - VIEW_ANGLE))
		if (viewDeg >= (180.0f - VIEW_ANGLE) && viewDeg <= (180.0f + VIEW_ANGLE))
		{
			return true;
		}
	}
	return false;
}

void Enemy::FollowPlayer(VECTOR& pos)
{
	animationController_->Play((int)ANIM_TYPE::WALK);

	// プレイヤーの位置
	VECTOR playerPos = player_.GetTransform().pos;

	//敵とプレイヤーの位置ベクトルを作成
	//プレイヤーの座標から敵の座標を引く
	VECTOR lookAt;
	lookAt = VSub(playerPos, transform_.pos);

	//位置ベクトルを正規化して方向ベクトルを作る
	// posE2P → direction
	//大きさ √をとる関数 sqrt    float用  sqrtf
	float size = sqrtf(lookAt.x * lookAt.x + lookAt.z * lookAt.z);

	//敵の移動処理
	if (size < MOVE_SPEED)
	{
		//ぶるぶるしないように
		//移動量よりも位置差が短い場合はプレイヤーに重なる
		pos = { static_cast<float>(playerPos.x) ,
			static_cast<float>(playerPos.y),
			static_cast<float>(playerPos.z) };
	}
	else
	{

		//正規化　位置ベクトルを大きさで割る
		VECTOR dirNorm = { lookAt.x / size, lookAt.y / size,lookAt.z / size };

		//位置ベクトルを使って敵を移動
		pos.x += static_cast<float>(dirNorm.x * MOVE_SPEED);
		pos.z += static_cast<float>(dirNorm.z * MOVE_SPEED);

		//向き画像を決める
		//水平か鉛直を選択する
		//※数値を絶対値(abs関数)としてみる

		VECTOR dir = CommonUtility::VECTOR_ZERO;

		if (abs(dirNorm.x) < abs(dirNorm.y))
		{
			//鉛直の向き(UP or DOWN)
			if (dirNorm.y < 0.0F)
			{
				dir = CommonUtility::DIR_F;
			}
			else
			{
				dir = CommonUtility::DIR_B;
			}
		}
		else
		{
			//水平の向き(RIHGT or LEFT)
			if (dirNorm.x < 0.0F)
			{
				dir = CommonUtility::DIR_L;
			}
			else
			{
				dir = CommonUtility::DIR_R;
			}
		}

		//敵からプレイヤーへの位置ベクトルを作成
		float angle = atan2(lookAt.x, lookAt.z);
		float angleDegrees = CommonUtility::Rad2DegF(angle);
		SetGoalRotate(angle);
	}
}

void Enemy::SetGoalRotate(double rotRad)
{
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

void Enemy::Rotate(void)
{
	//回転時間の減少
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();

	//回転の球面補間
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(enemyRotY_);
}

void Enemy::RotateToPlayer(void)
{
	//プレイヤーの座標から敵の座標を引く
	VECTOR lookAt;
	lookAt = VSub(player_.GetTransform().pos, transform_.pos);
	//atan2 で角度を計算
	float angle = atan2(lookAt.x, lookAt.z);
	SetGoalRotate(angle);

	//回転処理
	Rotate();
}

void Enemy::CreateBullet(const int createNum)
{
	if (createNum <= 0)return;
	//未生成だったら弾を生成する
	if (bullets_.empty())
	{
		for (int i = 0; i < createNum; i++)
		{
			bullets_.emplace_back(std::make_unique<EnemyBullet>(transform_));
			bullets_.back()->Init();
		}
	}
	//破棄済みの弾を探して再利用する
	for (const auto& bullet : bullets_)
	{
		//弾が消滅していたら再利用する
		if (bullet->GetState() != EnemyBullet::STATE::DESTROY)continue;
		bullet->Reset(transform_);
	}
	//敵の座標から調整位置を計算
	VECTOR headPos = capsule_->GetPosTop();
	VECTOR offSetPos = VSub(headPos, transform_.pos);
	//敵の上半身に半円を描く
	const float RANGE_ANGLE_DEG = 180.0f;
	//円弧の開始調整角度
	const float ANGLE_OFFSET_DEG = 15.0f;
	//弾の間隔角度
	const float ANGLE_STEP_DEG =
		(RANGE_ANGLE_DEG - (ANGLE_OFFSET_DEG * 2.0f)) / static_cast<float>(createNum - 1);
	//敵から弾座標の距離
	const float distance = 120.0f;
	//起点座標
	VECTOR baseShotPos = VECTOR();
	baseShotPos.x = distance;

	//他の弾の座標を設定
	const float angleStepDeg = 45.0f;
	for (int i = 0; i < createNum; ++i)
	{
		//
		float sngleStepDeg = ANGLE_OFFSET_DEG + (static_cast<float>(i) * ANGLE_STEP_DEG);
		Quaternion rot = Quaternion::AngleAxis(
			CommonUtility::Deg2RadD(sngleStepDeg), CommonUtility::AXIS_Z);
		VECTOR rotLocalPos = rot.PosAxis(baseShotPos);
		//弾の相対座標にセットする
		bullets_[i]->SetOffsetPos(offSetPos);
		bullets_[i]->SetLocalPos(rotLocalPos);
		bullets_[i]->SetPos(
			VAdd(headPos, transform_.quaRot.PosAxis(rotLocalPos)));
	}
	
	bullets_.resize(createNum);
}

bool Enemy::CheckBulletReady(void)
{
	for(const auto& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::READY)
		{
			return false;
		}
	}
	return true;
}

bool Enemy::CheckBulletDestroy(void)
{
	for (const auto& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::DESTROY)
		{
			return false;
		}
	}
	return true;
}

void Enemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateDead, this);
}

void Enemy::ChangeStateFollow(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateFollow, this);
}

void Enemy::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateMove, this);
}

void Enemy::ChangeStateAttackNear(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateAttackNear, this);
}

void Enemy::ChangeStateShotOne(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateShotOne, this);
}

void Enemy::ChangeStateShotAll(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateShotAll, this);
}

void Enemy::ChangeStateAttackCharge(void)
{
	sphereNear_->SetRadius(80.0f);
	sphereNear_->SetLocalPos({ 0.0f, 40.0f, 0.0f });
	stateUpdate_ = std::bind(&Enemy::UpdateChargeAttack, this);
}

void Enemy::ChangeStateBackstab(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateBackstab, this);
}

void Enemy::ChangeStateDown(void)
{
	animationController_->Play((int)ANIM_TYPE::DOWN, true, 0.0f,9.0f);
	animationController_->SetEndLoop(1.0f, 9.0f, 10.0f);
	isDown_ = true;
	stateUpdate_ = std::bind(&Enemy::UpdateDown, this);
}

void Enemy::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateDead, this);
}

void Enemy::UpdateNone(void)
{//何もしない
}

void Enemy::UpdateFollow(void)
{
	//状態時間更新
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > FOLLOW_TIME)
	{
		stateStep_ = 0.0f;
		hitCount_ = 0;
		ChangeState(STATE::MOVE);
		return;
	}

	FollowPlayer(transform_.pos);
	Rotate();

	//プレイヤーとの距離を測り、一定以上近づいたら追従をやめる
	if (CheckPlayerDistance() < PLAYER_DISTANCE)
	{
		ChangeState(STATE::MOVE);
		return;
	}
}

void Enemy::UpdateMove(void)
{
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > MOVE_TIME)
	{
		stateStep_ = 0.0f;
		//プレイヤーとの距離を測り、一定以上離れていたら遠距離攻撃
		//それ以外は近距離攻撃
		if (CheckPlayerDistance() < ATTACK_NEAR_DISTANCE)
		{
			ChangeState(STATE::ATTACK_NEAR);
		}
		else
		{
			animationController_->Play((int)ANIM_TYPE::CAST_SPELL, false);
			std::vector<STATE> attackState = { STATE::SHOT_ONE, STATE::SHOT_ALL };
			// 乱数生成器の初期化
			std::random_device rd; // 非決定的な乱数生成器
			std::mt19937 engine(rd()); // メルセンヌ・ツイスタ法による乱数生成器
			std::shuffle(attackState.begin(), attackState.end(), engine);
			//遠距離攻撃
			const int bulletNum = 5;
			CreateBullet(bulletNum);
			ChangeState(attackState[0]);
			return;
		}
	}

	//移動処理
	Move();

	RotateToPlayer();

	//すごく離れていたら追従状態に遷移
	if(CheckPlayerDistance() > FOLLOW_DISTANCE)
	{
		animationController_->Play((int)ANIM_TYPE::RUN);
		ChangeState(STATE::FOLLOW);
	}
}

void Enemy::UpdateAttackNear(void)
{
	Rotate();
	//プレイヤーとの距離を測り、一定以上離れていたら近づく
	VECTOR distance = VSub(player_.GetTransform().pos, transform_.pos);
	if (VSize(distance) > ATTACK_NEAR_DISTANCE)
	{
		isAttackedNear_ = false;
		FollowPlayer(transform_.pos);
		return;
	}
	isAttackedNear_ = true;

	//状態時間更新
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (animationController_->IsEnd())
	{
		stateStep_ = 0.0f;
		isAttackedNear_ = false;
		ChangeState(STATE::MOVE);
		return;
	}

	//攻撃アニメーション再生
	animationController_->Play((int)ANIM_TYPE::ATTACK_NEAR,false);

	//パリィ判定
	if (CommonUtility::IsHitSpheres(sphereNear_->GetPos(),sphereNear_->GetRadius(),
		player_.GetSphere().GetPos(),player_.GetSphere().GetRadius()))
	{
		if (player_.GetIsParry())
		{
			ChangeState(STATE::DOWN);
			Damage();
			isAttackedNear_ = false;
			return;
		}
	}

	//当たり判定
	if(CommonUtility::IsHitSphereCapsule(sphereNear_->GetPos(),
		sphereNear_->GetRadius(),player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(), player_.GetCapsule().GetRadius()))
	{
		//回避中だったらダメージを受けない
		if (player_.GetIsDodge())return;
		player_.Damage(ATTACK_DAMAGE);
		SceneManager::GetInstance().SetShakeScreen(true);
		ChangeState(STATE::MOVE);
	}
}

void Enemy::UpdateShotOne(void)
{
	if (IsCastSpell())
	{
		animationController_->Play((int)ANIM_TYPE::MAGIC_ILDE);
	}

	//回転処理
	RotateToPlayer();

	//
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > ATTACK_FAR_TIME)
	{
		stateStep_ = 0.0f;
		hitCount_ = 0;
		ChangeState(STATE::MOVE);
		return;
	}

	for (auto& bullet : bullets_)
	{
		bullet->Update();
	}

	//弾を順々に準備状態にする
	const float bulletInterval = 0.7f;
	for(auto& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::NONE)continue;
		if (stateStep_ > bulletInterval)
		{
			bullet->SetStateReady();
			stateStep_ = 0.0f;
		}
	}
	//弾が全部準備できたらプレイヤーに向けて発射する
	for (const auto& bullet : bullets_)
	{
		if (CheckBulletDestroy())break;
		if (stateStep_ > bulletInterval && bullet->GetState() == EnemyBullet::STATE::READY)
		{
			animationController_->Play((int)ANIM_TYPE::ATTACK_FAR_ONE, false);

			//ターゲットに発射
			bullet->Shot();
			bullet->SetTargetPos(player_.GetTransform().pos);
			stateStep_ = 0.0f;
		}
		if (bullet->GetState() == EnemyBullet::STATE::SHOT)bullet->SetTargetPos(player_.GetTransform().pos);

		//パリィ判定
		if (CommonUtility::IsHitSpheres(
			bullet->GetSphere().GetPos(), bullet->GetSphere().GetRadius(),
			player_.GetSphere().GetPos(), player_.GetSphere().GetRadius()))
		{
			//破棄状態の弾は無視
			if (bullet->GetState() == EnemyBullet::STATE::DESTROY)continue;
			if (player_.GetIsParry())
			{
				bullet->SetStateReverse();
				VECTOR targetPos = VAdd(transform_.pos,VGet(0.0f,80.0f,0.0f));
				bullet->SetTargetPos(targetPos);
				continue;
			}
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), player_.GetCapsule().GetPosTop(),
			player_.GetCapsule().GetPosDown(), player_.GetCapsule().GetRadius()))
		{
			//破棄状態の弾は無視
			if (bullet->GetState() == EnemyBullet::STATE::DESTROY)continue;
			//回避中だったらダメージを受けない
			if (player_.GetIsDodge())
			{
				continue;
			}
			//ダメージ処理(当たった弾は破棄)
			player_.Damage(ATTACK_DAMAGE);
			SceneManager::GetInstance().SetShakeScreen(true);
			bullet->Destroy();
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), capsule_->GetPosTop(),
			capsule_->GetPosDown(), capsule_->GetRadius()))
		{
			if (bullet->GetState() != EnemyBullet::STATE::REVERSE)continue;
			//ダメージ処理(当たった弾は破棄)
			Damage();
			bullet->Destroy();
			hitCount_++;
			continue;
		}
	}

	//全弾命中でダウン状態へ
	if (hitCount_ >= static_cast<int>(bullets_.size()))
	{
		ChangeState(STATE::DOWN);
		hitCount_ = 0;
		return;
	}
	
	//生成した弾が全部消滅したら移動遷移
	if (CheckBulletDestroy())
	{
		stateStep_ = 0.0f;
		ChangeState(STATE::MOVE);
		hitCount_ = 0;
		return;
	}
}

void Enemy::UpdateShotAll(void)
{
	if (IsCastSpell())
	{
		animationController_->Play((int)ANIM_TYPE::MAGIC_ILDE);
	}

	//回転処理
	RotateToPlayer();

	//遠距離攻撃の状態
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > ATTACK_FAR_TIME)
	{
		stateStep_ = 0.0f;
		hitCount_ = 0;
		ChangeState(STATE::MOVE);
		return;
	}

	//弾を順々に準備状態にする
	const float bulletInterval = 0.4f;
	for (auto& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::NONE)continue;
		if (stateStep_ > bulletInterval)
		{
			bullet->SetStateReady();
			stateStep_ = 0.0f;
		}
	}

	//弾が全部準備できたらプレイヤーに向けて発射する
	for (const auto& bullet : bullets_)
	{
		if (CheckBulletDestroy())break;
		if (stateStep_ > bulletInterval && bullet->GetState() == EnemyBullet::STATE::READY)
		{
			animationController_->Play((int)ANIM_TYPE::ATTACK_FAR_ALL, false);

			//ターゲットに発射
			bullet->Shot();
			bullet->SetTargetPos(player_.GetTransform().pos);
		}
		if (bullet->GetState() == EnemyBullet::STATE::SHOT)bullet->SetTargetPos(player_.GetTransform().pos);
		bullet->Update();

		//パリィ判定
		if (CommonUtility::IsHitSpheres(
			bullet->GetSphere().GetPos(), bullet->GetSphere().GetRadius(),
			player_.GetSphere().GetPos(), player_.GetSphere().GetRadius()))
		{
			//破棄状態の弾は無視
			if (bullet->GetState() == EnemyBullet::STATE::DESTROY)continue;
			//パリィ中にあたったら弾を跳ね返す
			if (player_.GetIsParry())
			{
				bullet->SetStateReverse();
				VECTOR targetPos = VAdd(transform_.pos, VGet(0.0f, 80.0f, 0.0f));
				bullet->SetTargetPos(targetPos);
				continue;
			}
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), player_.GetCapsule().GetPosTop(),
			player_.GetCapsule().GetPosDown(), player_.GetCapsule().GetRadius()))
		{
			//破棄状態の弾は無視
			if (bullet->GetState() == EnemyBullet::STATE::DESTROY)continue;
			//回避中だったらダメージを受けない
			if (player_.GetIsDodge())
			{
				continue;
			}
			//ダメージ処理(当たった弾は破棄)
 			player_.Damage(ATTACK_DAMAGE);
			SceneManager::GetInstance().SetShakeScreen(true);
			bullet->Destroy();
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), capsule_->GetPosTop(),
			capsule_->GetPosDown(), capsule_->GetRadius()))
		{
			if (bullet->GetState() != EnemyBullet::STATE::REVERSE)continue;
			//ダメージ処理(当たった弾は破棄)
			Damage();
			bullet->Destroy();
			hitCount_++;
			continue;
		}
	}

	//全弾命中でダウン状態へ
	if (hitCount_ >= static_cast<int>(bullets_.size()))
	{
		ChangeState(STATE::DOWN);
		hitCount_ = 0;
		return;
	}

	//生成した弾が全部消滅したら移動遷移
	if (CheckBulletDestroy())
	{
		stateStep_ = 0.0f;
		ChangeState(STATE::MOVE);
		hitCount_ = 0;
		return;
	}
}

void Enemy::UpdateChargeAttack(void)
{
	//溜め攻撃の状態
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > ATTACK_FAR_TIME)
	{
		stateStep_ = 0.0f;
		hitCount_ = 0;
		ChangeState(STATE::MOVE);
		return;
	}

}

void Enemy::UpdateBackstab(void)
{

}

void Enemy::UpdateDown(void)
{
	if(stepDownTime_ > DOWN_TIME && isDown_)
	{
		isDown_ = false;
		animationController_->Play((int)ANIM_TYPE::DOWN, false,9.0f,-1.0f, false, true);
	}
	if (animationController_->IsEnd())
	{
		stepDownTime_ = 0.0f;

		if (hp_ <= maxHp_ / 2)
		{
			ChangeState(STATE::ATTACK_CHARGE);
			return;
		}

		ChangeState(STATE::MOVE);
		return;
	}

	stepDownTime_ += SceneManager::GetInstance().GetDeltaTime();
}

void Enemy::UpdateDead(void)
{
	animationController_->Play((int)ANIM_TYPE::DEATH,false);
}

void Enemy::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Enemy");

	//HP用スライダー
	ImGui::SliderFloat("HP", &hp_, 0.0f,maxHp_);

	if (ImGui::Button("Kick"))
	{
		ChangeState(STATE::ATTACK_NEAR);
	}

	if (ImGui::Button("Shot One"))
	{
		ChangeState(STATE::SHOT_ONE);
	}

	if (ImGui::Button("Shot All"))
	{
		ChangeState(STATE::SHOT_ALL);
	}

	if (ImGui::Button("Charge Attack"))
	{
		ChangeState(STATE::ATTACK_CHARGE);
	}

	if (ImGui::Button("Backstab"))
	{
		ChangeState(STATE::BACKSTAB);
	}

	if (ImGui::Button("Down"))
	{
		ChangeState(STATE::DOWN);
	}

	if (ImGui::Button("Dead"))
	{
		ChangeState(STATE::DEAD);
	}


	//終了処理
	ImGui::End();
}

void Enemy::DrawDebug(void)
{

	//ラジアンに変換
	float viewRad = CommonUtility::Deg2RadF(VIEW_ANGLE);

	//角度から方向を取得
	//自分の座標
	VECTOR centerPos = transform_.pos;

	//前方方向を決める
	float forwardX = sinf(transform_.rot.y);
	float forwardZ = cosf(transform_.rot.y);

	//前方方向の座標
	VECTOR forwardPos = centerPos;
	forwardPos.x += forwardX * VIEW_RANGE;
	forwardPos.z += forwardZ * VIEW_RANGE;
	//後方方向の座標
	VECTOR backPos = centerPos;
	backPos.x -= forwardX * VIEW_RANGE;
	backPos.z -= forwardZ * VIEW_RANGE;

	//右斜め30度方向を決める
	float rightX = sinf(transform_.rot.y + viewRad);
	float rightZ = cosf(transform_.rot.y + viewRad);

	VECTOR rightPos = centerPos;
	rightPos.x -= rightX * VIEW_RANGE;
	rightPos.z -= rightZ * VIEW_RANGE;

	//左斜め30度方向を決める
	float leftX = sinf(transform_.rot.y - viewRad);
	float leftZ = cosf(transform_.rot.y - viewRad);

	//左斜め30度方向の座標
	VECTOR leftPos = centerPos;
	leftPos.x -= leftX * VIEW_RANGE;
	leftPos.z -= leftZ * VIEW_RANGE;

	//DrawSphere3D(backPos, 10.0f, 10, 0x00ff00, 0x00ff00, true);
	//DrawSphere3D(forwardPos, 20.0f, 10, 0x00ff00, 0x00ff00, true);
	//DrawSphere3D(rightPos, 10.0f, 10, 0x00ff00, 0x00ff00, true);
	//DrawSphere3D(leftPos, 10.0f, 10, 0x00ff00, 0x00ff00, true);
	DrawTriangle3D(backPos, centerPos, leftPos, 0xffdead, true);
	DrawTriangle3D(centerPos, backPos, rightPos, 0xffdead, true);

}