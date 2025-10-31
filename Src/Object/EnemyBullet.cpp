#include <DxLib.h>
#include "../Common/DebugDrawFormat.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Utility/CommonUtility.h"
#include "Common/Geometry/Sphere.h"
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
		Quaternion::Euler({ CommonUtility::Deg2RadF(-90.0f),0.0f, 0.0f });
	transform_.Update();

	//当たり判定用の球を生成
    sphere_ = std::make_unique<Sphere>(transform_);
    sphere_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	sphere_->SetRadius(20.0f);
}

void EnemyBullet::Update(void)
{
	if (!isAlive_)return;
	//モデル情報の更新
	transform_.Update();
	//発射もしくは反射状態ではない場合は回転の同期を行う
	if (GetState() == STATE::NONE ||
		GetState() == STATE::READY)
	{
		//同期
		SyncParentRotate();
	}

	//発射状態でなければ移動処理を行わない
	if (!CheckStateShot() && !CheckStateReverse())return;

	//弾の移動処理
	Move();
}
 
void EnemyBullet::Draw(void)
{
	//発射状態でなければ描画しない
	if (!isAlive_)return;

	//モデルの描画
	MV1DrawModel(transform_.modelId);
	//当たり判定用の球の描画
    sphere_->Draw(); 

#ifdef _DEBUG

	int line = 1;
	//DebugDrawFormat::FormatStringRight(L"bulletPos : %.2f,%.2f",
	//	transform_.pos.x, transform_.pos.z,
	//	line);

#endif // _DEBUG
}

void EnemyBullet::Destroy(void)
{
	//破棄状態へ変更
	ChangeState(STATE::DESTROY);
	SetIsAlive(false);
}

void EnemyBullet::SetLocalPos(const VECTOR localPos)
{
	//親の位置+ローカル座標
	localPos_ = localPos;
	transform_.pos = VAdd(parentTran_.pos, localPos);
	transform_.Update();
}

void EnemyBullet::Shot(void)
{
	//発射状態へ変更
	state_ = STATE::SHOT;
	isAlive_ = true;
}

void EnemyBullet::Reset(const Transform& transform)
{
	state_ = STATE::NONE;
	isAlive_ = false;
	transform_.pos = transform.pos;
	transform_.quaRot = transform.quaRot;
	//諸々モデルの初期化
	const VECTOR ARROW_LOCAL_POS = { 0.0f, 185.0f, 0.0f };
	VECTOR localPos = transform_.quaRot.PosAxis(ARROW_LOCAL_POS);
	transform_.pos = VAdd(transform_.pos, localPos);
}

void EnemyBullet::Move(void)
{
	//前方向を取得
	VECTOR forward = VNorm(VSub(targetPos_,transform_.pos));

	//下方向の取得
	VECTOR downward = transform_.GetDown();
	const float speed = 15.0f;
	//横ベクトル
	VECTOR widthMovePow = VScale(forward, speed);

	//移動
	//前方
	transform_.pos =
		VAdd(transform_.pos, widthMovePow);

	//重力加速度
	const float GRAVITY_POW = 0.0f;
	transform_.pos =
		VAdd(transform_.pos, VScale(downward, GRAVITY_POW));
}

void EnemyBullet::SyncParentRotate(void)
{
	VECTOR parentPos = parentTran_.pos;
	parentPos.y = 140.0f;
	VECTOR localPos = VSub(transform_.pos, parentPos);
	//親の回転を考慮したローカル座標を計算
	VECTOR relativePos = parentTran_.quaRot.PosAxis(localPos);
	//親の位置+ローカル座標
	transform_.pos = VAdd(parentTran_.pos,relativePos);
	transform_.quaRot = parentTran_.quaRot;
}