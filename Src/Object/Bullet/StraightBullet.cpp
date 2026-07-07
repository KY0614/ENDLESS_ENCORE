#include <EffekseerForDXLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Common/Geometry/Sphere.h"
#include "StraightBullet.h"

StraightBullet::StraightBullet(
	Transform& parent,
	VECTOR targetPos) :
	BulletBase(parent),
	targetPos_(targetPos)
{
	moveDir_ = CommonUtility::VECTOR_ZERO;
}

StraightBullet::~StraightBullet(void)
{
}

void StraightBullet::Init(void)
{	
	//サウンドの初期化
	InitSound();

	//モデルの基本設定
	InitTransform();

	//当たり判定用のコライダーの初期化
	InitCollider();

	//火のエフェクトのリソース読み込み
	effectFireResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FIRE_EFFECT).handleId_;

	//初期状態は非生存状態
	SetIsAlive(false);

	//速度の設定
	SetSpeed(BULLET_SPEED);

	//生存時間の設定
	SetLifeTime(LIFE_TIME);

	//初期状態
	ChangeState(STATE::NONE);
}

void StraightBullet::Update(void)
{	
	//更新ステップ
	stateUpdate_();

	//当たり判定処理
	CollisionSphere();

	transform_.Update();
}

void StraightBullet::InitTransform(void)
{
	//モデルの基本設定
	const float scl = 1.0f;
	transform_.scl = { scl ,scl ,scl };
	transform_.pos = parentTran_.pos;
	transform_.quaRot = parentTran_.quaRot;
	transform_.quaRotLocal = Quaternion();
	transform_.Update();
}

void StraightBullet::InitCollider(void)
{
	//当たり判定用の球を生成
	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	sphere_->SetRadius(BULLET_RADIUS);
}

void StraightBullet::ChangeStateNone(void)
{	
	//生存していない
	SetIsAlive(false);
	stateUpdate_ = std::bind(&StraightBullet::UpdateNone, this);
}

void StraightBullet::ChangeStateReady(void)
{
	//SE再生
	SoundManager& sound = SoundManager::GetInstance();
	sound.AdjustVolume(SoundManager::SOUND::FIRE, FIRE_SE_VOLUME);
	sound.Play(SoundManager::SOUND::FIRE);
	//生存状態へ
	SetIsAlive(true);
	//生存時間のリセット
	lifeTime_ = LIFE_TIME;
	EffectFire();
	stateUpdate_ = std::bind(&StraightBullet::UpdateReady, this);
}

void StraightBullet::ChangeStateShot(void)
{
	//移動方向
	moveDir_ = VNorm(VSub(targetPos_, transform_.pos));
	stateUpdate_ = std::bind(&StraightBullet::UpdateShot, this);
}

void StraightBullet::ChangeStateReverse(void)
{
	//エフェクトの色変更(水色っぽく変更)
	const int effectColorG = 128;
	const int effectColorB = 255;
	const int effectAlpha = 255;
	SetColorPlayingEffekseer3DEffect(
		effectFirePlayId_,
		0,
		effectColorG,
		effectColorB,
		effectAlpha
	);
	//移動方向
	moveDir_ = VNorm(VSub(targetPos_, transform_.pos));
	stateUpdate_ = std::bind(&StraightBullet::UpdateReverse, this);
}

void StraightBullet::ChangeStateDestroy(void)
{
	SetIsAlive(false);
	StopEffekseer3DEffect(effectFirePlayId_);
	lifeTime_ = 0.0f;
	stateUpdate_ = std::bind(&StraightBullet::UpdateDestroy, this);
}

void StraightBullet::UpdateNone(void)
{
}

void StraightBullet::UpdateReady(void)
{
	//エフェクトの位置同期
	EffectFirePositionSync();

	//同期
	SyncParentRotate();
}

void StraightBullet::UpdateShot(void)
{
	lifeTime_ -= SceneManager::GetInstance().GetDeltaTime();
	if (lifeTime_ <= 0.0f)
	{
		//生存時間が尽きたら、破棄状態へ遷移
		ChangeState(STATE::DESTROY);
		return;
	}

	//エフェクトの位置同期
	EffectFirePositionSync();

	//弾の移動処理
	Move();
}

void StraightBullet::UpdateReverse(void)
{
	//エフェクトの位置同期
	EffectFirePositionSync();

	//弾の移動処理
	ReserveMove();
}

void StraightBullet::UpdateDestroy(void)
{
}

void StraightBullet::Move(void)
{
	//移動方向
	VECTOR moveDir = VNorm(VSub(targetPos_, transform_.pos));
	//移動量
	VECTOR movePow = VScale(moveDir_, speed_);

	//移動
	VECTOR movedPos =
		VAdd(transform_.pos, movePow);

	transform_.pos = movedPos;
}

void StraightBullet::ReserveMove(void)
{	
	//移動方向
	VECTOR moveDir = VNorm(VSub(parentTran_.pos,transform_.pos));
	//移動量
	VECTOR movePow = VScale(moveDir, speed_);

	//移動
	VECTOR movedPos =
		VAdd(transform_.pos, movePow);

	transform_.pos = movedPos;
}