#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Common/Geometry/Sphere.h"
#include "BulletBase.h"

BulletBase::BulletBase(Transform& parent):
	parentTran_(parent)
{
	localPos_ = CommonUtility::VECTOR_ZERO;
	offsetPos_ = CommonUtility::VECTOR_ZERO;
	lifeTime_ = 0.0f;
	isAlive_ = false;
	state_ = STATE::NONE;
	speed_ = 0.0f;
	effectFireResId_ = -1;
	effectFirePlayId_ = -1;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&BulletBase::ChangeStateNone, this));
	stateChanges_.emplace(STATE::READY, std::bind(&BulletBase::ChangeStateReady, this));
	stateChanges_.emplace(STATE::SHOT, std::bind(&BulletBase::ChangeStateShot, this));
	stateChanges_.emplace(STATE::REVERSE, std::bind(&BulletBase::ChangeStateReverse, this));
	stateChanges_.emplace(STATE::DESTROY, std::bind(&BulletBase::ChangeStateDestroy, this));
}

BulletBase::~BulletBase(void)
{
}

void BulletBase::Init(void)
{
}

void BulletBase::Update(void)
{
}

void BulletBase::Reset(const Transform& transform)
{
	//初期状態へ
	ChangeState(STATE::NONE);
	transform_.pos = transform.pos;
	transform_.quaRot = transform.quaRot;
}

void BulletBase::ChangeState(const STATE& state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void BulletBase::InitSound(void)
{
	//サウンドの登録
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::FIRE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::FIRE_SE).handleId_);
}

void BulletBase::CollisionSphere(void)
{
	//球体との衝突判定
	for (const std::weak_ptr<Collider> c : colliders_)
	{
		MV1_COLL_RESULT_POLY_DIM hits = MV1CollCheck_Sphere(
			c.lock()->modelId_, -1,
			GetSphere().GetPos(), GetSphere().GetRadius());
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
				int pHit = HitCheck_Sphere_Triangle(
					GetSphere().GetPos(), GetSphere().GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					//弾を破棄状態へ遷移
					ChangeState(STATE::DESTROY);
					continue;
				}
				break;
			}
		}
		//検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void BulletBase::SyncParentRotate(void)
{
	//敵の頭あたりをイメージした座標
	VECTOR basePos = VAdd(parentTran_.pos, offsetPos_);
	//予め決めておいた敵の頭からの相対座標を敵の向きに応じて回転させる
	transform_.pos = VAdd(basePos, parentTran_.quaRot.PosAxis(localPos_));
	transform_.quaRot = parentTran_.quaRot;
}

void BulletBase::EffectFire(void)
{
	//生存していなければ再生しない
	if (!isAlive_)return;

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

void BulletBase::EffectFirePositionSync(void)
{
	//エフェクトの位置を同期
	SetPosPlayingEffekseer3DEffect(
		effectFirePlayId_,
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z);
}
