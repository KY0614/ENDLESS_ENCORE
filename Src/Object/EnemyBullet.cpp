#include <DxLib.h>
#include<EffekseerForDXLib.h>
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Utility/CommonUtility.h"
#include "Common/Geometry/Sphere.h"
#include "EnemyBullet.h"

namespace 
{
	//弾の生存時間
	const float LIFE_TIME = 8.0f;
}

EnemyBullet::EnemyBullet(Transform& parent)
	: parentTran_(parent)
{
	localPos_ = CommonUtility::VECTOR_ZERO;
	offsetPos_ = CommonUtility::VECTOR_ZERO;
	targetPos_ = CommonUtility::VECTOR_ZERO;
	lifeTime_ = 0.0f;
	isAlive_ = false;
	state_ = STATE::NONE;
	effectFireResId_ = -1;
	effectFirePlayId_ = -1;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&EnemyBullet::ChangeStateNone, this));
	stateChanges_.emplace(STATE::READY, std::bind(&EnemyBullet::ChangeStateReady, this));
	stateChanges_.emplace(STATE::SHOT, std::bind(&EnemyBullet::ChangeStateShot, this));
	stateChanges_.emplace(STATE::REVERSE, std::bind(&EnemyBullet::ChangeStateReverse, this));
	stateChanges_.emplace(STATE::DESTROY, std::bind(&EnemyBullet::ChangeStateDestroy, this));
}

EnemyBullet::~EnemyBullet(void)
{
}

void EnemyBullet::Init(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::FIRE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::FIRE_SE).handleId_);

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
		ResourceManager::SRC::FIRE_EFFECT).handleId_;

	//初期状態は非生存状態
	SetIsAlive(false);

	//生存時間の設定
	SetLifeTime(LIFE_TIME);

	//初期状態
	ChangeState(STATE::NONE);
}

void EnemyBullet::Update(void)
{
	//if (!isAlive_)return;

	//発射もしくは反射状態ではない場合は回転の同期を行う
	//if (GetState() == STATE::NONE ||
	//	GetState() == STATE::READY)
	//{
	//	//同期
	//	SyncParentRotate();
	//}

	//更新ステップ
	stateUpdate_();

	transform_.Update();
}
 
void EnemyBullet::Draw(void)
{
	//発射状態でなければ描画しない
	if (!isAlive_)return;

	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//当たり判定用の球の描画
    //sphere_->Draw(); 
}

void EnemyBullet::SetOffsetPos(const VECTOR& offset)
{
	offsetPos_ = offset;
}

void EnemyBullet::SetLocalPos(const VECTOR& local)
{
	//親の位置+ローカル座標
	localPos_ = local;
}

void EnemyBullet::Reset(const Transform& transform)
{
	//初期状態へ
	ChangeState(STATE::NONE);
	transform_.pos = transform.pos;
	transform_.quaRot = transform.quaRot;
	//諸々モデルの初期化
	const VECTOR ARROW_LOCAL_POS = { 0.0f, 185.0f, 0.0f };
	VECTOR localPos = transform_.quaRot.PosAxis(ARROW_LOCAL_POS);
	transform_.pos = VAdd(transform_.pos, localPos);
}

void EnemyBullet::ChangeState(const STATE& state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void EnemyBullet::ChangeStateNone(void)
{
	//生存していない
	SetIsAlive(false);
	stateUpdate_ = std::bind(&EnemyBullet::UpdateNone, this);
}

void EnemyBullet::ChangeStateReady(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::PARRY, 70);
	sound.Play(SoundManager::SOUND::FIRE);
	SetIsAlive(true);
	EffectFire();
	stateUpdate_ = std::bind(&EnemyBullet::UpdateReady, this);
}

void EnemyBullet::ChangeStateShot(void)
{
	stateUpdate_ = std::bind(&EnemyBullet::UpdateShot, this);
}

void EnemyBullet::ChangeStateReverse(void)
{
	//エフェクトの色変更(水色っぽく変更)
	SetColorPlayingEffekseer3DEffect(
		effectFirePlayId_,
		0,
		128,
		255,
		255
	);
	stateUpdate_ = std::bind(&EnemyBullet::UpdateReverse, this);
}

void EnemyBullet::ChangeStateDestroy(void)
{
	SetIsAlive(false);
	StopEffekseer3DEffect(effectFirePlayId_);
	lifeTime_ = LIFE_TIME;
	stateUpdate_ = std::bind(&EnemyBullet::UpdateDestroy, this);
}

void EnemyBullet::UpdateNone(void)
{
}

void EnemyBullet::UpdateReady(void)
{	
	//エフェクトの位置同期
	EffectFirePositionSync();

	//同期
	SyncParentRotate();
}

void EnemyBullet::UpdateShot(void)
{
	lifeTime_ -= SceneManager::GetInstance().GetDeltaTime();
	if (lifeTime_ <= 0.0f)
	{
		SetStateDestroy();
		return;
	}

	//エフェクトの位置同期
	EffectFirePositionSync();

	//弾の移動処理
	Move();
}

void EnemyBullet::UpdateReverse(void)
{
	//エフェクトの位置同期
	EffectFirePositionSync();

	//弾の移動処理
	Move();
}

void EnemyBullet::UpdateDestroy(void)
{
}

void EnemyBullet::Move(void)
{
	//前方向を取得
	VECTOR forward = VNorm(VSub(targetPos_,transform_.pos));

	//下方向の取得
	VECTOR downward = transform_.GetDown();
	const float speed = 20.0f;
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
	float EFFEKT_SCALE = 15.0f;		//X,Z方向のスケール
	SetScalePlayingEffekseer3DEffect(
		effectFirePlayId_,
		EFFEKT_SCALE,
		EFFEKT_SCALE,
		EFFEKT_SCALE
	);
}

void EnemyBullet::EffectFirePositionSync(void)
{
	//エフェクトの位置を同期
	SetPosPlayingEffekseer3DEffect(
		effectFirePlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}
