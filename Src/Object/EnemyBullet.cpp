#include <DxLib.h>
#include<EffekseerForDXLib.h>
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

	//火のエフェクトのリソース読み込み
	effectFireResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FIRE_EFKT).handleId_;
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

	//エフェクトの位置を同期
	SetPosPlayingEffekseer3DEffect(
		effectFirePlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);

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
	StopEffekseer3DEffect(effectFirePlayId_);
}

void EnemyBullet::SetStateReady(void)
{
	state_ = STATE::READY;
	isAlive_ = true;
	EffectFire();
}

void EnemyBullet::SetOffsetPos(const VECTOR offset)
{
	offsetPos_ = offset;
}

void EnemyBullet::SetLocalPos(const VECTOR local)
{
	//親の位置+ローカル座標
	localPos_ = local;
	//transform_.pos = VAdd(parentTran_.pos, local);
	//transform_.Update();
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
	//敵の頭あたりをイメージした座標
	VECTOR basePos = VAdd(parentTran_.pos, offsetPos_);
	//予め決めておいた敵の頭からの相対座標を敵の向きに応じて回転させる
	transform_.pos = VAdd(basePos, parentTran_.quaRot.PosAxis(localPos_));
	transform_.quaRot = parentTran_.quaRot;
}

void EnemyBullet::EffectFire(void)
{
	//再生Idを取得
	effectFirePlayId_ = PlayEffekseer3DEffect(effectFireResId_);
	//大きさの設定
	//大きさ
	float EFFEKT_SCALE = 20.0f;
	float EFFEKT_SCALE_Y = 26.0f;
	SetScalePlayingEffekseer3DEffect(
		effectFirePlayId_,
		EFFEKT_SCALE,
		EFFEKT_SCALE_Y,
		EFFEKT_SCALE
	);

}
