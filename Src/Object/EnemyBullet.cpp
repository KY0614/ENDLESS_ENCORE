#include "../Manager/Generic/ResourceManager.h"
#include "../Utility/CommonUtility.h"
#include "Common/Sphere.h"
#include "EnemyBullet.h"

EnemyBullet::EnemyBullet(int num)
	: bulletNum_(num)
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
	transform_.pos = { 0.0f, 0.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ CommonUtility::Deg2RadF(90.0f), 0.0f, 0.0f });
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
