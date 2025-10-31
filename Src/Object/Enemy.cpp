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
	static const std::string KEY_ATK_NEAR = "Attack_Neaer";
	static const std::string KEY_DAMAGE = "Damage";
	static const std::string KEY_DOWN = "Down";
	static const std::string KEY_DEATH = "Death";
	//回転にかける時間
	const float TIME_ROT = 0.1f;
	//敵の基本パラメータ
	const float HP_MAX = 100.0f;	//最大HP	
	const float MOVE_SPEED = 13.0f;	//移動速度
	//距離の基準値
	const float ATTACK_NEAR_DISTANCE = 350.0f;	//近距離攻撃判定距離
	const float ATTACK_FAR_DISTANCE = 800.0f;	//遠距離攻撃判定距離
	const float PLAYER_DISTANCE = 500.0f;		//維持するプレイヤーとの距離
	const float FOLLOW_DISTANCE = 800.0f;		//追従距離
	//重力加速度
	const float MOVE_TIME = 3.0f;
	const float ATTACK_TIME = 1.0f;
	const float ATTACK_FAR_TIME = 15.0f;

	const float ATTACK_DAMAGE = 10.0f;

	//アニメーション再生速度
	const float ANIM_SPEED = 30.0f;

	const int FAR_SPHERE_NUM = 4;

	const float DOWN_TIME = 4.0f;
}

Enemy::Enemy(Player& player):player_(player)
{
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	stateStep_ = 0.0f;
	state_ = STATE::NONE;
	col_ = 0xff0000;
	isAttackedNear_ = false;
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
	//UpdateDebugImGui();
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
	case Enemy::STATE::DOWN:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"DOWN");
		break;
	case Enemy::STATE::DEAD:
		DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"DEAD");
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

#endif // _DEBUG

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
	SetMaxHP(paramData.value(JsonManager::KEY_HP, 0.0f));
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
	sphereNear_->SetLocalPos({ 0.0f, 80.0f, -50.0f });
	sphereNear_->SetRadius(30.0f);

	col_ = 0x000000;

}

void Enemy::InitAnimation(void)
{
	auto& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(JsonManager::JSON_DATA::ENEMY);
	const auto& param = data[KEY_ENEMY];
	//データが含まれていない場合はエラーメッセージを出す
	if (!param.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const auto& animPath = param[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/";
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
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + animPath.value(KEY_DAMAGE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::DOWN, path + animPath.value(KEY_DOWN, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + animPath.value(KEY_DEATH, KEY_EMPTY),
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
		animationController_->Play((int)ANIM_TYPE::WALK);
		//プレイヤーに近づく
		FollowPlayer(transform_.pos);
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

void Enemy::FollowPlayer(VECTOR& pos)
{
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

	VECTOR headPos = capsule_->GetPosTop();
	//敵から弾座標の距離
	const float distance = 120.0f;
	//敵から見て右側の座標を計算
	VECTOR rightPos = VScale(transform_.GetRight(), distance);
	rightPos.y = headPos.y;		//高さを敵の頭の高さに合わせる
	//最初の弾は右側に配置(敵から見て)
	rightPos = VAdd(transform_.pos, rightPos);
	bullets_[0]->SetPos(rightPos);

	//他の弾の座標を設定
	const float angleStepDeg = 45.0f;
	for (int i = 1; i < createNum; ++i)
	{
		//１つ前の弾の座標を取得
		VECTOR prevPos = bullets_[i - 1]->GetTransform().pos;
		//座標を回転させる(敵の頭座標を中心に前の弾から一定角度回転)
		VECTOR rotPos = CommonUtility::RotXYPos(
			headPos, prevPos, CommonUtility::Deg2RadF(-angleStepDeg));

		bullets_[i]->SetPos(rotPos);

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

void Enemy::SyncBulletPosAxis(void)
{
	for(auto& bullet : bullets_)
	{
		EnemyBullet::STATE state = bullet->GetState();
		if (state != EnemyBullet::STATE::READY && state != EnemyBullet::STATE::NONE)
			continue;

		VECTOR headPos = capsule_->GetPosTop();
		VECTOR localPos = VSub(bullet->GetTransform().pos, headPos);
		VECTOR pos = transform_.quaRot.PosAxis(localPos);
		pos.y += headPos.y;
		bullet->SetLocalPos(pos);
	}
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
{
}

void Enemy::UpdateFollow(void)
{
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
			std::vector<STATE> attackState = { STATE::SHOT_ONE, STATE::SHOT_ALL };
			// 乱数生成器の初期化
			std::random_device rd; // 非決定的な乱数生成器
			std::mt19937 engine(rd()); // メルセンヌ・ツイスタ法による乱数生成器
			std::shuffle(attackState.begin(), attackState.end(), engine);
			//遠距離攻撃
			const int bulletNum = 5;
			CreateBullet(bulletNum);
			ChangeState(attackState[0]);
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

	//
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (animationController_->IsEnd())
	{
		stateStep_ = 0.0f;
		isAttackedNear_ = false;
		ChangeState(STATE::MOVE);
		return;
	}
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
	animationController_->Play((int)ANIM_TYPE::IDLE);
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
	animationController_->Play((int)ANIM_TYPE::IDLE);
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

	//終了処理
	ImGui::End();
}