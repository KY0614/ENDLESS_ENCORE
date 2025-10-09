#include "../Common/DebugDrawFormat.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Utility/CommonUtility.h"
#include "Common/Sphere.h"
#include "EnemyBullet.h"

EnemyBullet::EnemyBullet(Transform& parent)
	: parentTran_(parent)
{
	isAlive_ = false;
	state_ = STATE::NONE;
}

EnemyBullet::~EnemyBullet(void)
{
}

void EnemyBullet::Init(void)
{
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ENEMY_BULLET));
	const float scl = 1.0f;
	transform_.scl = { scl ,scl ,scl };
	transform_.pos = parentTran_.pos;
	transform_.quaRot = parentTran_.quaRot;
	transform_.quaRotLocal =
		Quaternion::Euler({ CommonUtility::Deg2RadF(-90.0f), 0.0f, 0.0f });
	transform_.Update();

	//当たり判定用の球を生成
    sphere_ = std::make_unique<Sphere>(transform_);
    sphere_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	sphere_->SetRadius(20.0f);
}

void EnemyBullet::Update(void)
{
	//発射状態でなければ更新しない
	if (!CheckBulletStateShot())return;

	Move();

	transform_.Update();
}
 
void EnemyBullet::Draw(void)
{
	//発射状態でなければ描画しない
	if (!CheckBulletStateShot())return;
	//モデルの描画
	MV1DrawModel(transform_.modelId);
	//当たり判定用の球の描画
    sphere_->Draw();
	int line = 1;
	DebugDrawFormat::FormatStringRight(L"bulletPos : %.2f,%.2f",
		transform_.pos.x, transform_.pos.z,
		line);
}

void EnemyBullet::Destroy(void)
{
	//破棄状態へ変更
	ChangeState(STATE::DETSTROY);
	SetIsAlive(false);
}

void EnemyBullet::SetLocalPos(const VECTOR pos)
{
	//親の位置+ローカル座標
	transform_.pos = VAdd(parentTran_.pos, pos);
}

void EnemyBullet::ShotBullet(void)
{
	state_ = STATE::SHOT;
	isAlive_ = true;
}

void EnemyBullet::ResetBullet(void)
{
	state_ = STATE::NONE;
	isAlive_ = false;

	//諸々モデルの初期化
	const VECTOR ARROW_LOCAL_POS = { 0.0f, 185.0f, 0.0f };
	VECTOR localPos = transform_.quaRot.PosAxis(ARROW_LOCAL_POS);
	transform_.pos = VAdd(transform_.pos, localPos);
}

void EnemyBullet::Move(void)
{
	// 前方向を取得
	VECTOR forward = transform_.GetForward();
	//下方向の取得
	VECTOR downward = transform_.GetDown();

	//横ベクトル
	VECTOR widthMovePow = VScale(forward, 3.0f);

	// 移動
	//前方
	transform_.pos =
		VAdd(transform_.pos, widthMovePow);
	//重力加速度
	const float GRAVITY_POW = 0.5f;
	transform_.pos =
		VAdd(transform_.pos, VScale(downward, GRAVITY_POW));
}