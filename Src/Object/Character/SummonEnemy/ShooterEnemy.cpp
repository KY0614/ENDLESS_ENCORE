#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../../Common/Geometry/Sphere.h"
#include "../../Common/Geometry/Capsule.h"
#include "../../EnemyBullet.h"
#include "ShooterEnemy.h"

ShooterEnemy::ShooterEnemy(Player& player) :
	SummonEnemyBase(player)
{
	state_ = STATE::NONE;
	stepRotTime_ = 0.0f;
	isSummoned_ = false;
	bulletInterval_ = 0.0f;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&ShooterEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&ShooterEnemy::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&ShooterEnemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK, std::bind(&ShooterEnemy::ChangeStateAttack, this));
}

ShooterEnemy::~ShooterEnemy(void)
{
}

void ShooterEnemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();

	bullet_ = std::make_unique<EnemyBullet>(transform_);
	bullet_->Init();
}

void ShooterEnemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	transform_.Update();
}

void ShooterEnemy::Draw(void)
{
	//球体を仮で描画
	const float rad = 30.0f;
	const int div = 16;
	//青：遠距離攻撃タイプ
	int col = GetColor(0, 0, 255);
	DrawSphere3D(
		transform_.pos,
		rad,
		div,
		col,
		col,
		true);

	VECTOR forward = VAdd(transform_.pos, VScale(transform_.GetForward(), 40.0f));
	DrawLine3D(
		transform_.pos,
		forward,
		GetColor(0, 0, 255));

	VECTOR right = VAdd(transform_.pos, VScale(transform_.GetRight(), 40.0f));
	DrawLine3D(
		transform_.pos,
		right,
		GetColor(255, 0, 0));
}

void ShooterEnemy::Init3DModel(void)
{
	//モデルの基本設定
	//transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
	//	ResourceManager::SRC::ENEMY));
	//モデルの大きさ(Jsonデータから取得できなかったら1.0f)
	const float scale = 1.0f;
	transform_.scl = { scale ,scale ,scale };
	//モデルの初期位置
	transform_.pos = CommonUtility::VECTOR_ZERO;
	//モデルの初期回転(度数法で保存されているのでラジアンに変換)
	const float rotY = 0.0f;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
}

void ShooterEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateNone, this);
}

void ShooterEnemy::ChangeStateSummon(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateSummon, this);
}

void ShooterEnemy::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateMove, this);
}

void ShooterEnemy::ChangeStateAttack(void)
{
	VECTOR headPos = VAdd(transform_.pos, VScale(transform_.GetUp(), 60.0f));
	VECTOR ofssetPos = VSub(headPos, transform_.pos);
	bullet_->SetOffsetPos(ofssetPos);
	bullet_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	bullet_->SetPos(headPos);
	//コライダを弾に追加
	for (const std::weak_ptr<Collider> c : colliders_)
	{
		bullet_->AddCollider(c);
	}
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateAttack, this);
}

void ShooterEnemy::UpdateNone(void)
{//何もしない
}

void ShooterEnemy::UpdateSummon(void)
{
	transform_.pos.y++;
	if (transform_.pos.y >= -100.0f)
	{
		transform_.pos.y = -100.0f;
		IsSummoned();
		ChangeState(STATE::MOVE);
	}
}

void ShooterEnemy::UpdateMove(void)
{
	bulletInterval_ += SceneManager::GetInstance().GetDeltaTime();

	if(bulletInterval_ >= 3.0f)
	{
		bulletInterval_ = 0.0f;
		ChangeState(STATE::ATTACK);
		return;
	}

	//プレイヤーを見続ける
	Rotate2Player();

	//回転処理
	Rotate();
}

void ShooterEnemy::UpdateAttack(void)
{
	//攻撃
	Shoot();

	//プレイヤーを見続ける
	Rotate2Player();

	//回転処理
	Rotate();

	bullet_->Update();
}

void ShooterEnemy::Shoot(void)
{
	bulletInterval_ += SceneManager::GetInstance().GetDeltaTime();

	const float interval = 1.0f;	//弾の発射間隔
	if (bullet_->GetState() == EnemyBullet::STATE::NONE &&
		bulletInterval_ >= interval)
	{
		bullet_->SetStateReady();
		bulletInterval_ = 0.0f;
	}

	if(bulletInterval_ >= interval &&
		bullet_->GetState() == EnemyBullet::STATE::READY)
	{
		//弾を発射
		bullet_->SetStateShot();
		//弾のターゲット座標をプレイヤーの位置に設定
		bullet_->SetTargetPos(player_.GetTransform().pos);
	}

	if (CommonUtility::IsHitSphereCapsule(
		bullet_->GetSphere().GetPos(),
		bullet_->GetSphere().GetRadius(),
		player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(),
		player_.GetCapsule().GetRadius()))
	{
		//プレイヤーにダメージを与える
		player_.Damage(10.0f);
		//弾を消す
		bullet_->SetStateDestroy();
		bulletInterval_ = 0.0f;
		ChangeState(STATE::MOVE);
	}

	
}