#include "../Manager/Generic/ResourceManager.h"
#include "../Utility/CommonUtility.h"
#include "Common/Sphere.h"
#include "EnemyBullet.h"

EnemyBullet::EnemyBullet(Transform& parent)
	: parentTran_(parent)
{
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
	transform_.Update();
}
 
void EnemyBullet::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

    sphere_->Draw();
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
	const float GRAVITY_POW = 15.0f;
	transform_.pos =
		VAdd(transform_.pos, VScale(downward, GRAVITY_POW));
}