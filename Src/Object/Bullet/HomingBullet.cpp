#include <EffekseerForDXLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Common/Geometry/Sphere.h"
#include "HomingBullet.h"

HomingBullet::HomingBullet(
	Transform& parent,
	VECTOR targetPos):
	BulletBase(parent),
	targetPos_(targetPos)
{
}

HomingBullet::~HomingBullet(void)
{
}

void HomingBullet::Init(void)
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

void HomingBullet::Update(void)
{	
	//更新ステップ
	stateUpdate_();

	transform_.Update();
}

void HomingBullet::InitTransform(void)
{
	//モデルの基本設定
	const float scl = 1.0f;
	transform_.scl = { scl ,scl ,scl };
	transform_.pos = parentTran_.pos;
	transform_.quaRot = parentTran_.quaRot;
	transform_.quaRotLocal = Quaternion();
	transform_.Update();
}

void HomingBullet::InitCollider(void)
{
	//当たり判定用の球を生成
	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	const float sphereRadius = 20.0f;
	sphere_->SetRadius(sphereRadius);
}

void HomingBullet::ChangeStateNone(void)
{
	//生存していない
	SetIsAlive(false);
	stateUpdate_ = std::bind(&HomingBullet::UpdateNone, this);
}

void HomingBullet::ChangeStateReady(void)
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
	stateUpdate_ = std::bind(&HomingBullet::UpdateReady, this);
}

void HomingBullet::ChangeStateShot(void)
{
	stateUpdate_ = std::bind(&HomingBullet::UpdateShot, this);
}

void HomingBullet::ChangeStateReverse(void)
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
	stateUpdate_ = std::bind(&HomingBullet::UpdateReverse, this);
}

void HomingBullet::ChangeStateDestroy(void)
{
	SetIsAlive(false);
	StopEffekseer3DEffect(effectFirePlayId_);
	lifeTime_ = 0.0f;
	stateUpdate_ = std::bind(&HomingBullet::UpdateDestroy, this);
}

void HomingBullet::UpdateNone(void)
{
}

void HomingBullet::UpdateReady(void)
{	
	//エフェクトの位置同期
	EffectFirePositionSync();

	//同期
	SyncParentRotate();
}

void HomingBullet::UpdateShot(void)
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

	//当たり判定処理
	CollisionSphere();
}

void HomingBullet::UpdateReverse(void)
{	
	//エフェクトの位置同期
	EffectFirePositionSync();

	//弾の移動処理
	Move();
}

void HomingBullet::UpdateDestroy(void)
{
}

void HomingBullet::Move(void)
{
	//前方向を取得
	VECTOR forward = VNorm(VSub(targetPos_, transform_.pos));

	//下方向の取得
	VECTOR downward = transform_.GetDown();
	//横ベクトル
	VECTOR widthMovePow = VScale(forward, speed_);

	//移動
	//前方
	transform_.pos =
		VAdd(transform_.pos, widthMovePow);

	//重力加速度
	const float GRAVITY_POW = 0.0f;
	transform_.pos =
		VAdd(transform_.pos, VScale(downward, GRAVITY_POW));
}