#include <random>
#include <EffekseerForDXLib.h>
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
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
	static const std::string KEY_TURN = "Turn";
	static const std::string KEY_WALK = "Walk";
	static const std::string KEY_WALK_RIGHT = "Walk Right";
	static const std::string KEY_WALK_LEFT = "Walk Left";
	static const std::string KEY_RUN = "Run";
	static const std::string KEY_ATK_NEAR = "Attack_Near";
	static const std::string KEY_ATK_FAR_ONE = "Attack_Far_One";
	static const std::string KEY_ATK_FAR_ALL = "Attack_Far_All";
	static const std::string KEY_ATK_CHARGE = "Attack_Charge";
	static const std::string KEY_DAMAGE = "Damage";
	static const std::string KEY_DOWN = "Down";
	static const std::string KEY_BACKSTAB = "Backstab";
	static const std::string KEY_MAGIC_IDLE = "Magic Idle";
	static const std::string KEY_CAST_SPELL = "Cast Spell";
	static const std::string KEY_STAND_UP = "Stand Up";
	static const std::string KEY_DEATH = "Death";
	static const std::string KEY_MAX_HP = "maxHp";
	static const std::string KEY_MAX_POS = "maxPosition";
	static const std::string KEY_MIN_POS = "minPosition";
	//近接攻撃当たり判定球のローカル座標
	const VECTOR ATTACK_NEAR_SPHERE_POS = { 0.0f, 80.0f, 50.0f };
	//チャージ攻撃当たり判定球のローカル座標
	const VECTOR ATTACK_CHARGE_SPHERE_POS = { 0.0f, 40.0f, 0.0f };
	//遠距離攻撃当たり判定球の半径
	const float ATTACK_NEAR_SPHERE_RADIUS = 30.0f;
	//回転にかける時間
	const float TIME_ROT = 0.1f;
	//敵の基本パラメータ
	const float MOVE_SPEED = 2.0f;		//移動速度
	const float FOLLOW_SPEED = 7.0f;	//追従速度
	//距離の基準値
	const float ATTACK_NEAR_DISTANCE = 350.0f;	//近距離攻撃判定距離
	const float ATTACK_FAR_DISTANCE = 800.0f;	//遠距離攻撃判定距離
	const float PLAYER_DISTANCE = 750.0f;		//維持するプレイヤーとの距離
	const float FOLLOW_DISTANCE = 900.0f;		//追従距離
	//状態ごとの時間
	const float FOLLOW_TIME = 3.0f;			//追従時間
	const float MOVE_TIME = 3.0f;			//移動時間
	const float ATTACK_TIME = 1.0f;			//攻撃後の待機時間
	const float ATTACK_FAR_TIME = 15.0f;	//遠距離攻撃後の待機時間
	const float ATTACK_CHARGE_TIME = 30.0f;	//ため攻撃後の待機時間
	//ダメージ
	const float ATTACK_DAMAGE = 10.0f;		//近接攻撃ダメージ
	const float NORMAL_DAMAGE = 10.0f;		//遠距離攻撃ダメージ
	const float BACKSTAB_DAMAGE = 50.0f;	//バックスタブダメージ
	const float CHARGE_DAMAGE = 100.0f;		//ため攻撃ダメージ
	//ダウンする時間
	const float DOWN_TIME = 4.0f;
	//視野角・視野範囲
	const float VIEW_ANGLE = 40.0f;
	const float VIEW_RANGE = 100.0f;
	//重力加速度
	const float GRAVITY_POW = 15.0f;
	//バックスタブSE音量
	const int BACKSTAB_SE_VOLUME = 70;
}

Enemy::Enemy(Player& player):
	player_(player)
{
	hp_ = 0.0f;
	maxHp_ = 0.0f;
	stateStep_ = 0.0f;
	changeDirStep_ = 0.0f;
	gravHitPosDown_ = CommonUtility::VECTOR_ZERO;
	gravHitPosUp_ = CommonUtility::VECTOR_ZERO;
	movedPos_ = CommonUtility::VECTOR_ZERO;
	state_ = STATE::NONE;
	prevState_ = STATE::NONE;
	isDown_ = false;
	isStepActioned_ = false;
	isBackstab_ = false;
	isChargeAtk_ = false;
	hitCount_ = 0;
	currentAngle_ = 0.0f;               // 初期角度は適当に設定 (atan2で初期化しても良い)
	stepDownTime_ = 0.0f;
	// 例: 1秒で 90度（π/2 ラジアン）回転する速度
	circlingSpeedRad_ = DX_PI_F / 2.0f  * 0.1f;

	stepRotTime_ = 0.0f;
	chargeRadius_ = 0.0f;
	moveDir_ = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	isEncount_ = false;

	effectChargeResId_ = -1;
	effectChargePlayId_ = -1;
	effectChargeAtkResId_ = -1;
	effectChargeAtkPlayId_ = -1;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Enemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&Enemy::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::TURN, std::bind(&Enemy::ChangeStateTurn, this));
	stateChanges_.emplace(STATE::ENCOUNT_FINISH, std::bind(&Enemy::ChangeStateEncountFinish, this));
	stateChanges_.emplace(STATE::WAIT, std::bind(&Enemy::ChangeStateWait, this));
	stateChanges_.emplace(STATE::FOLLOW, std::bind(&Enemy::ChangeStateFollow, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Enemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK_NEAR, std::bind(&Enemy::ChangeStateAttackNear, this));
	stateChanges_.emplace(STATE::SHOT_ONE, std::bind(&Enemy::ChangeStateShotOne, this));
	stateChanges_.emplace(STATE::SHOT_ALL, std::bind(&Enemy::ChangeStateShotAll, this));
	stateChanges_.emplace(STATE::CHARGE, std::bind(&Enemy::ChangeStateCharge, this));
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
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::BACKSTAB,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::BACKSTAB_SE).handleId_);
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::FLAME,
	ResourceManager::GetInstance().Load(ResourceManager::SRC::FLAME_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::EXPLORE, 70);

	//3Dモデルの初期化
	Init3DModel();

	//当たり判定の初期化
	InitCollider();

	//アニメーションの初期化
	InitAnimation();

	//火のエフェクトのリソース読み込み
	effectChargeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::CHARGE_EFFECT).handleId_;

	effectChargeAtkResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::EXPLOSIVE_EFFECT).handleId_;

	//初期の状態を設定
	ChangeState(STATE::NONE);
}

void Enemy::Update(void)
{
	if (hp_ <= 0.0f)hp_ = 0.0f;

	//更新ステップ
	stateUpdate_();

	animationController_->Update();
	transform_.Update();
}

void Enemy::Draw(void)
{
	//プレイヤーとエンカウントしていなかったら描画しない
	if (!isEncount_)return;

	//モデルの描画
	MV1DrawModel(transform_.modelId);

	VECTOR pos = ConvWorldPosToScreenPos(transform_.pos);

	for(const std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		bullet->Draw();
	}

	//丸影描画
	DrawShadow();
}

void Enemy::ChangeState(const STATE& state)
{
	stateStep_ = 0.0f;
	isStepActioned_ = false;
	//状態変更
	prevState_ = state_;
	state_ = state;
	//死亡判定
	if (hp_ <= 0.0f)
	{
		hitCount_ = 0;
		bullets_.clear();
		state_ = STATE::DEAD;
	}

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

const bool Enemy::GetIsDead(void) const
{
	return state_ == STATE::DEAD && animationController_->IsEnd();
}

void Enemy::Init3DModel(void)
{
	//Jsonデータ取得
	const json data = GetJsonData();

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const json& transformData = data[JsonManager::KEY_TRANSFORM];

	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ENEMY));
	//モデルの大きさ(Jsonデータから取得できなかったら1.0f)
	const float scale = transformData.value(JsonManager::KEY_SCALE, 1.0f);
	transform_.scl = { scale ,scale ,scale };
	//モデルの初期位置
	transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
	//モデルの初期回転(度数法で保存されているのでラジアンに変換)
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();

	//HPを設定
	const json& paramData = data[JsonManager::KEY_PARAMETER];
	SetHP(paramData.value(JsonManager::KEY_HP, 0.0f));
	SetMaxHP(paramData.value(JsonManager::KEY_MAX_HP, 0.0f));
}

void Enemy::InitCollider(void)
{
	//カプセルコライダのパラメータ
	const VECTOR cupsulePosTop = { 0.0f, 140.0f, 0.0f };
	const VECTOR cupsulePosDown = { 0.0f, 20.0f, 0.0f };
	const float cupsuleRadius = 30.0f;
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop(cupsulePosTop);
	capsule_->SetLocalPosDown(cupsulePosDown);
	capsule_->SetRadius(cupsuleRadius);

	//近接攻撃用の球体コライダ
	sphereNear_ = std::make_unique<Sphere>(transform_);
	sphereNear_->SetLocalPos(ATTACK_NEAR_SPHERE_POS);
	sphereNear_->SetRadius(ATTACK_NEAR_SPHERE_RADIUS);
}

void Enemy::InitAnimation(void)
{
	//Jsonデータ取得w
	const json& data = GetJsonData();
	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const json& animPath = data[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/Animation/";
	const char* KEY_EMPTY = "";
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	const float animSpeedSlow = animSpeed / 2.0f;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::TURN, path + animPath.value(KEY_TURN, KEY_EMPTY),
		animSpeedSlow);
	animationController_->Add((int)ANIM_TYPE::WALK, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::WALK_RIGHT, path + animPath.value(KEY_WALK_RIGHT, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::WALK_LEFT, path + animPath.value(KEY_WALK_LEFT, KEY_EMPTY),
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
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK_CHARGE, path + animPath.value(KEY_ATK_CHARGE, KEY_EMPTY),
		animSpeed);
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

void Enemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateNone, this);
}

void Enemy::ChangeStateEncount(void)
{
	isEncount_ = true;
	stateUpdate_ = std::bind(&Enemy::UpdateEncount, this);
}

void Enemy::ChangeStateTurn(void)
{
	animationController_->Play((int)ANIM_TYPE::TURN, false);
	stateUpdate_ = std::bind(&Enemy::UpdateTurn, this);
}

void Enemy::ChangeStateEncountFinish(void)
{
	//敵の向きを戦闘開始時の向きに設定
	const float battleRotY = 180.0f;
	transform_.quaRot =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(-battleRotY), 0.0f });
	stateUpdate_ = std::bind(&Enemy::UpdateEncountFinish, this);
}

void Enemy::ChangeStateWait(void)
{
	animationController_->Play((int)ANIM_TYPE::IDLE);
	stateUpdate_ = std::bind(&Enemy::UpdateWait, this);
}

void Enemy::ChangeStateFollow(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateFollow, this);
}

void Enemy::ChangeStateMove(void)
{
	if (!isEncount_)isEncount_ = true;
	moveDir_ = transform_.GetRight();
	animationController_->Play((int)ANIM_TYPE::WALK_RIGHT);
	stateUpdate_ = std::bind(&Enemy::UpdateMove, this);
}

void Enemy::ChangeStateAttackNear(void)
{
	//攻撃アニメーション再生
	animationController_->Play((int)ANIM_TYPE::ATTACK_NEAR, false);
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

void Enemy::ChangeStateCharge(void)
{
	//当たり判定球の初期化
	sphereNear_->SetRadius(0.0f);
	sphereNear_->SetLocalPos(ATTACK_CHARGE_SPHERE_POS);
	//アニメーションを途中まで再生
	const float animEndStep = 26.0f;
	animationController_->Play((int)ANIM_TYPE::ATTACK_CHARGE, false, 0.0f, animEndStep);
	EffectCharge();
	stateUpdate_ = std::bind(&Enemy::UpdateCharge, this);
}

void Enemy::ChangeStateAttackCharge(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.Play(SoundManager::SOUND::FLAME);
	//アニメーションを途中から再生
	const float animStartStep = 26.0f;
	animationController_->Play((int)ANIM_TYPE::ATTACK_CHARGE, false, animStartStep, -1.0f, false, true);
	EffectChargeAtk();
	stateUpdate_ = std::bind(&Enemy::UpdateChargeAttack, this);
}

void Enemy::ChangeStateBackstab(void)
{
	//アニメーションを途中まで再生
	const float animEndStep = 26.0f;
	animationController_->Play((int)ANIM_TYPE::BACKSTAB, false, 0.0f, animEndStep);
	stateUpdate_ = std::bind(&Enemy::UpdateBackstab, this);
}

void Enemy::ChangeStateDown(void)
{
	//ダウンアニメーション再生(途中まで再生してループさせる)
	const float animEndStep = 9.0f;
	const float animLoopSpeed = 10.0f;
	animationController_->Play((int)ANIM_TYPE::DOWN, true, 0.0f, animEndStep);
	const float animStartStep = 1.0f;
	animationController_->SetEndLoop(animStartStep, animEndStep, animLoopSpeed);
	isDown_ = true;
	stepDownTime_ = 0.0f;
	stateUpdate_ = std::bind(&Enemy::UpdateDown, this);
}

void Enemy::ChangeStateDead(void)
{
	//死亡アニメーション再生
	animationController_->Play((int)ANIM_TYPE::DEATH, false);
	//バックスタブからの遷移だったら倒れたままのアニメーションを再生しておく
	const float animStartStep = 110.0f;
	if (prevState_ == STATE::BACKSTAB)animationController_->Play(
		(int)ANIM_TYPE::BACKSTAB, false, animStartStep, -1.0f);
	stateUpdate_ = std::bind(&Enemy::UpdateDead, this);
}

void Enemy::UpdateNone(void)
{//何もしない
}

void Enemy::UpdateEncount(void)
{
}

void Enemy::UpdateTurn(void)
{
	if (animationController_->IsEnd())
	{
		animationController_->Play((int)ANIM_TYPE::IDLE);
		ChangeState(STATE::ENCOUNT_FINISH);
		return;
	}
}

void Enemy::UpdateEncountFinish(void)
{
}

void Enemy::UpdateWait(void)
{
	//プレイヤーのほうを向いて待機
	RotateToPlayer();
}

void Enemy::UpdateFollow(void)
{
	//状態時間更新
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > FOLLOW_TIME)
	{
		hitCount_ = 0;
		ChangeState(STATE::MOVE);
		return;
	}

	//追従処理
	FollowPlayer(transform_.pos);

	Collision();

	//回転処理
	Rotate();
}

void Enemy::UpdateMove(void)
{
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();

	//プレイヤーがいる方向を見続ける
	RotateToPlayer();

	//移動処理
	Move();
	Collision();
	if (stateStep_ > MOVE_TIME)
	{
		//HPが最大の半分以下になっていたらチャージ攻撃状態に遷移
		if (!isChargeAtk_ &&
			hp_ <= maxHp_ / 2.0f)
		{
			isChargeAtk_ = true;
			ChangeState(STATE::CHARGE);
			return;
		}
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
			std::random_device rd; //非決定的な乱数生成器
			std::mt19937 engine(rd()); //メルセンヌ・ツイスタ法による乱数生成器
			std::shuffle(attackState.begin(), attackState.end(), engine);
			//遠距離攻撃
			const int bulletNum = 5;
			CreateBullet(bulletNum);
			ChangeState(attackState[0]);
			return;
		}
	}

	//離れていたら追従状態に遷移
	if (CheckPlayerDistance() > FOLLOW_DISTANCE)
	{
		animationController_->Play((int)ANIM_TYPE::RUN);
		ChangeState(STATE::FOLLOW);
	}
}

void Enemy::UpdateAttackNear(void)
{
	//回転処理
	Rotate();

	//アニメーションが終わったら移動状態へ戦記
	if (animationController_->IsEnd())
	{
		ChangeState(STATE::MOVE);
		return;
	}

	//パリィされたらダメージを受けてダウン状態へ遷移
	if (CommonUtility::IsHitSpheres(
		sphereNear_->GetPos(),
		sphereNear_->GetRadius(),
		player_.GetSphere().GetPos(),
		player_.GetSphere().GetRadius()))
	{
		if (player_.GetIsParry())
		{
			Damage(NORMAL_DAMAGE);
			ChangeState(STATE::DOWN);
			return;
		}
	}

	//既に行動済みだったら処理しない
	if (isStepActioned_)return;

	//当たり判定
	if (CommonUtility::IsHitSphereCapsule(
		sphereNear_->GetPos(),
		sphereNear_->GetRadius(),
		player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(),
		player_.GetCapsule().GetRadius()))
	{
		//回避中だったらダメージを受けない
		if (player_.GetIsDodge())return;
		player_.Damage(ATTACK_DAMAGE);
		//画面揺らし
		SceneManager::GetInstance().StartShakeScreen();
		isStepActioned_ = true;
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

	for (const std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		bullet->Update();
	}

	//弾を順々に準備状態にする
	const float bulletInterval = 0.7f;
	for (std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::NONE)continue;
		if (stateStep_ > bulletInterval)
		{
			bullet->SetStateReady();
			stateStep_ = 0.0f;
		}
	}
	//弾が全部準備できたらプレイヤーに向けて発射する
	for (const std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		if (CheckBulletDestroy())break;
		if (CheckBulletReady())animationController_->Play((int)ANIM_TYPE::ATTACK_FAR_ONE, false);
		if (stateStep_ > bulletInterval && bullet->GetState() == EnemyBullet::STATE::READY)
		{
			//ターゲットに発射
			bullet->SetStateShot();
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
				//ターゲット座標に高さを加えて胸のあたりを狙う
				const float targetOffY = 80.0f;
				VECTOR targetPos = VAdd(transform_.pos, VGet(0.0f, targetOffY, 0.0f));
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
			if (player_.GetIsDodge())continue;
			//ダメージ処理(当たった弾は破棄)
			player_.Damage(ATTACK_DAMAGE);
			SceneManager::GetInstance().StartShakeScreen();
			bullet->SetStateDestroy();
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), capsule_->GetPosTop(),
			capsule_->GetPosDown(), capsule_->GetRadius()))
		{
			if (bullet->GetState() != EnemyBullet::STATE::REVERSE)continue;
			//ダメージ処理(当たった弾は破棄)
			Damage(NORMAL_DAMAGE);
			bullet->SetStateDestroy();
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

	//弾を順々に準備状態にする
	const float bulletInterval = 0.4f;
	for (std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::NONE)continue;
		if (stateStep_ > bulletInterval)
		{
			bullet->SetStateReady();
			stateStep_ = 0.0f;
			continue;
		}
	}
	if (CheckBulletReady())animationController_->Play((int)ANIM_TYPE::ATTACK_FAR_ALL, false);

	//弾が全部準備できたらプレイヤーに向けて発射する
	for (const std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		if (CheckBulletDestroy())break;
		if (stateStep_ > bulletInterval &&
			bullet->GetState() == EnemyBullet::STATE::READY)
		{
			//ターゲットに発射
			bullet->SetStateShot();
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
				//ターゲット座標に高さを加えて胸のあたりを狙う
				const float targetOffY = 80.0f;
				VECTOR targetPos = VAdd(transform_.pos, VGet(0.0f, targetOffY, 0.0f));
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
			SceneManager::GetInstance().StartShakeScreen();
			bullet->SetStateDestroy();
		}

		//当たり判定
		if (CommonUtility::IsHitSphereCapsule(bullet->GetSphere().GetPos(),
			bullet->GetSphere().GetRadius(), capsule_->GetPosTop(),
			capsule_->GetPosDown(), capsule_->GetRadius()))
		{
			if (bullet->GetState() != EnemyBullet::STATE::REVERSE)continue;
			//ダメージ処理(当たった弾は破棄)
			Damage(NORMAL_DAMAGE);
			bullet->SetStateDestroy();
			hitCount_++;
			continue;
		}
	}

	//全弾命中でダウン状態へ
	if (hitCount_ >= static_cast<int>(bullets_.size()))
	{
		stateStep_ = 0.0f;
		ChangeState(STATE::DOWN);
		hitCount_ = 0;
		return;
	}

	//生成した弾が全部消滅したら移動遷移
	if (CheckBulletDestroy())
	{
		ChangeState(STATE::MOVE);
		hitCount_ = 0;
		return;
	}
}

void Enemy::UpdateCharge(void)
{
	//チャージで範囲を大きくする
	const float chargeSpeed = 1.5f;
	chargeRadius_ += chargeSpeed;
	sphereNear_->SetRadius(chargeRadius_);
	const float chargeRad = 250.0f;
	if (chargeRadius_ > chargeRad)
	{
		chargeRadius_ = chargeRad;
		ChangeState(STATE::ATTACK_CHARGE);
		return;
	}

	if (IsEffekseer3DEffectPlaying(effectChargePlayId_) < 0)
	{
		EffectCharge();
	}
}

void Enemy::UpdateChargeAttack(void)
{
	if (animationController_->IsEnd())
	{
		chargeRadius_ = 0.0f;
		//球体を近接用に戻す
		sphereNear_->SetLocalPos(ATTACK_NEAR_SPHERE_POS);
		sphereNear_->SetRadius(ATTACK_NEAR_SPHERE_RADIUS);
		ChangeState(STATE::MOVE);
		return;
	}

	//既に行動済みだったら処理しない
	if (isStepActioned_)return;
	//球体判定
	if (CommonUtility::IsHitSphereCapsule(
		sphereNear_->GetPos(),
		sphereNear_->GetRadius(),
		player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(),
		player_.GetCapsule().GetRadius()))
	{
		//回避中だったらダメージを受けない
		if (player_.GetIsDodge())
		{
			
		}
		player_.Damage(CHARGE_DAMAGE);
		isStepActioned_ = true;
	}
}

void Enemy::UpdateBackstab(void)
{
	//続きを再生させるための待ち時間
	const float stopTime = 0.1f;
	//途中までの再生が終わったら経過時間まで待ち、
	//残りのアニメーションを再生する
	if (animationController_->IsEnd())
	{
		stateStep_ += SceneManager::GetInstance().GetDeltaTime();
		if (stateStep_ > stopTime && !isBackstab_)
		{
			//途中から再生
			const float animStartStep = 26.0f;
			const float animEndStep = 110.0f;
			animationController_->Play((int)ANIM_TYPE::BACKSTAB, false,
				animStartStep, animEndStep, false, true);
			Damage(BACKSTAB_DAMAGE);
			isBackstab_ = true;	
			SoundManager& sound = SoundManager::GetInstance();
			sound.AdjustVolume(SoundManager::SOUND::BACKSTAB, BACKSTAB_SE_VOLUME);
			sound.Play(SoundManager::SOUND::BACKSTAB);
		}
	}
	//アニメーションが最後まで再生されたら移動状態へ遷移
	if (isBackstab_ && animationController_->IsEnd())
	{
		isBackstab_ = false;
		ChangeState(STATE::MOVE);
		return;
	}
}

void Enemy::UpdateDown(void)
{
	if (stepDownTime_ > DOWN_TIME && isDown_)
	{
		isDown_ = false;
		//途中から再生
		const float animStartStep = 9.0f;
		animationController_->Play((int)ANIM_TYPE::DOWN, false, animStartStep, -1.0f, false, true);
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
{//何もしない
}

void Enemy::Damage(const float damage)
{
	//ダメージ処理
	hp_ -= damage;
}

void Enemy::Move(void)
{
	//移動方向変更の経過時間
	changeDirStep_ += SceneManager::GetInstance().GetDeltaTime();
	//一定時間経過したら移動方向をランダムで変更
	const float changeInterval = 1.0f;
	if (changeDirStep_ >= changeInterval)
	{
		changeDirStep_ = 0.0f;
		std::vector<VECTOR> moveDir =
		{ transform_.GetRight(), transform_.GetLeft() };
		// 乱数生成器の初期化
		std::random_device rd; //非決定的な乱数生成器
		std::mt19937 engine(rd()); //メルセンヌ・ツイスタ法による乱数生成器
		std::shuffle(moveDir.begin(), moveDir.end(), engine);
		moveDir_ = moveDir[0];

		//移動方向に応じて歩行アニメーションを変更
		if (CommonUtility::Equals(moveDir_, transform_.GetLeft()))
		{
			animationController_->Play((int)ANIM_TYPE::WALK_LEFT);
		}
		if (CommonUtility::Equals(moveDir_, transform_.GetRight()))
		{
			animationController_->Play((int)ANIM_TYPE::WALK_RIGHT);
		}
	}
	movePow_ = VScale(moveDir_, MOVE_SPEED);
	//移動処理
	movedPos_ = VAdd(transform_.pos, movePow_);
	transform_.pos = movedPos_;
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

	//アニメーションが終了しているか
	if (animationController_->IsEnd() && 
		animationController_->GetPlayType() == (int)ANIM_TYPE::CAST_SPELL)
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
	float viewRange = std::pow(VIEW_RANGE, 2.0f);
	//視野範囲内に入っているか
	if (distance <= viewRange)
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
	if (size < FOLLOW_SPEED)
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
		pos.x += static_cast<float>(dirNorm.x * FOLLOW_SPEED);
		pos.z += static_cast<float>(dirNorm.z * FOLLOW_SPEED);

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

void Enemy::Collision(void)
{
	//重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;

	//重力の強さ
	float gravityPow = GRAVITY_POW;

	//重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	transform_.pos = VAdd(transform_.pos, gravity);

	//現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	//衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();
	//移動後座標を反映
	transform_.pos = movedPos_;
}

void Enemy::CollisionCapsule(void)
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

void Enemy::CollisionGravity(void)
{
	//重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;

	//重力方向の反対
	VECTOR dirUpGravity = CommonUtility::DIR_U;

	//重力
	const float checkPow = 10.0f;
	//2倍の長さで線を引く
	const float checkLength = 2.0f;
	//上方向のチェック開始位置
	const float dotThreshold = 0.9f;
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * checkLength));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		//地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
		
		if (hit.HitFlag > 0 && VDot(dirGravity, CommonUtility::VECTOR_ZERO) > dotThreshold)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, checkLength));
		}
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
	bullets_.resize(createNum);
	//破棄済みの弾を探して再利用する
	for (const std::unique_ptr<EnemyBullet>& bullet : bullets_)
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
}

bool Enemy::CheckBulletReady(void)
{
	for(const std::unique_ptr<EnemyBullet>& bullet : bullets_)
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
	for (const std::unique_ptr<EnemyBullet>& bullet : bullets_)
	{
		if (bullet->GetState() != EnemyBullet::STATE::DESTROY)
		{
			return false;
		}
	}
	return true;
}

void Enemy::EffectCharge(void)
{
	//再生Idを取得
	effectChargePlayId_ = PlayEffekseer3DEffect(effectChargeResId_);

	//大きさの設定
	float EFFEKT_SCALE = 80.0f;		//X,Z方向のスケール
	SetScalePlayingEffekseer3DEffect(
		effectChargePlayId_,
		EFFEKT_SCALE,
		EFFEKT_SCALE,
		EFFEKT_SCALE
	);

	//エフェクトの位置を同期
	SetPosPlayingEffekseer3DEffect(
		effectChargePlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}

void Enemy::EffectChargeAtk(void)
{
	//再生Idを取得
	effectChargeAtkPlayId_ = PlayEffekseer3DEffect(effectChargeAtkResId_);

	//大きさの設定
	float EFFEKT_SCALE = 40.0f;		//X,Z方向のスケール
	SetScalePlayingEffekseer3DEffect(
		effectChargeAtkPlayId_,
		EFFEKT_SCALE,
		EFFEKT_SCALE,
		EFFEKT_SCALE
	);
	//再生速度の設定(少し早めに設定）
	SetSpeedPlayingEffekseer3DEffect(effectChargeAtkPlayId_, 2.0f);
	//エフェクトの位置を同期
	SetPosPlayingEffekseer3DEffect(
		effectChargeAtkPlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}

const json Enemy::GetJsonData(void)const
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(
		JsonManager::JSON_DATA::ENEMY,KEY_ENEMY);

	return data;
}

void Enemy::DrawHPBar(void)
{
	VECTOR pos = ConvWorldPosToScreenPos(transform_.pos);
	const float barOffset = 100.0f;
	const int HP_BAR_X = static_cast<int>(pos.x - barOffset);// HPバーの左上X座標
	const int HP_BAR_Y = static_cast<int>(pos.z + barOffset);// HPバーの左上Y座標

	const int HP_BAR_WIDTH = static_cast<int>(maxHp_);    // HPバーの最大幅
	const int HP_BAR_HEIGHT = 30;		// HPバーの高さ
	float hp = hp_ / maxHp_;
	int barWidth = static_cast<int>(HP_BAR_WIDTH * hp);
	const int posX = Application::SCREEN_SIZE_X / 2 - HP_BAR_WIDTH / 2;
	const int posY = Application::SCREEN_SIZE_Y - (HP_BAR_HEIGHT * 3);
	//色の設定
	const int barBackColor = GetColor(100, 100, 100);	//背景（グレー）
	const int barColor = GetColor(255, 0, 0);			//現在HP（赤）
	// 背景（グレー）
	DrawBox(posX,
		posY,
		posX + HP_BAR_WIDTH,
		posY + HP_BAR_HEIGHT,
		barBackColor, TRUE);
	// 現在HP（赤）
	DrawBox(posX,
		posY,
		posX + barWidth,
		posY + HP_BAR_HEIGHT,
		barColor, TRUE);

}
