#include "../Manager/Generic/SceneManager.h"
#include "../Utility/CommonUtility.h"
#include "FighterEnemy.h"

namespace
{
	const float FOLLOW_SPEED = 7.0f;	//追従速度

	const float TIME_ROT = 0.1f;		//回転にかける時間
}

FighterEnemy::FighterEnemy(Player& player):
	SummonEnemyBase(player)
{
	state_ = STATE::NONE;
	stepRotTime_ = 0.0f;
	isSummoned_ = false;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&FighterEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&FighterEnemy::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&FighterEnemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&FighterEnemy::ChangeStateAttack, this));
}

FighterEnemy::~FighterEnemy(void)
{
}

void FighterEnemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();
}

void FighterEnemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	transform_.Update();
}

void FighterEnemy::Draw(void)
{
	//球体を仮で描画
	const float rad = 30.0f;
	const int div = 16;
	//赤：近距離攻撃タイプ
	int col = GetColor(255, 0, 0);
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

void FighterEnemy::Init3DModel(void)
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

void FighterEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateNone, this);
}

void FighterEnemy::ChangeStateSummon(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateSummon, this);
}

void FighterEnemy::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateMove, this);
}

void FighterEnemy::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateAttack, this);
}

void FighterEnemy::UpdateNone(void)
{//何もしない
}

void FighterEnemy::UpdateSummon(void)
{
	transform_.pos.y++;
	if (transform_.pos.y >= -100.0f)
	{
		transform_.pos.y = -100.0f;
		IsSummoned();
		ChangeState(STATE::MOVE);
	}
}

void FighterEnemy::UpdateMove(void)
{
	//追従処理
	FollowMove();

	//回転処理
	Rotate();
}

void FighterEnemy::UpdateAttack(void)
{
	//攻撃処理
}

void FighterEnemy::FollowMove(void)
{
	// プレイヤーの位置
	VECTOR playerPos = player_.GetTransform().pos;

	//敵とプレイヤーの位置ベクトルを作成
	//プレイヤーの座標から敵の座標を引く
	VECTOR lookAt;
	lookAt = VSub(playerPos, transform_.pos);

	//位置ベクトルを正規化して方向ベクトルを作る
	// posE2P → direction
	//大きさ √をとる関数 sqrt    float用  sqrtf
	float size = sqrtf(lookAt.x * lookAt.x + lookAt.z * lookAt.z);

	//敵の移動処理
	if (size < FOLLOW_SPEED)
	{
		//ぶるぶるしないように
		//移動量よりも位置差が短い場合はプレイヤーに重なる
		transform_.pos = { static_cast<float>(playerPos.x) ,
			static_cast<float>(playerPos.y),
			static_cast<float>(playerPos.z) };
	}
	else
	{
		//正規化　位置ベクトルを大きさで割る
		VECTOR dirNorm = { lookAt.x / size, lookAt.y / size,lookAt.z / size };

		//位置ベクトルを使って敵を移動
		transform_.pos.x += static_cast<float>(dirNorm.x * FOLLOW_SPEED);
		transform_.pos.z += static_cast<float>(dirNorm.z * FOLLOW_SPEED);

		//向き画像を決める
		//水平か鉛直を選択する
		VECTOR dir = CommonUtility::VECTOR_ZERO;

		if (abs(dirNorm.x) < abs(dirNorm.y))
		{
			//鉛直の向き(UP or DOWN)
			if (dirNorm.y < 0.0F)
			{
				dir = CommonUtility::DIR_F;
			}
			else
			{
				dir = CommonUtility::DIR_B;
			}
		}
		else
		{
			//水平の向き(RIHGT or LEFT)
			if (dirNorm.x < 0.0F)
			{
				dir = CommonUtility::DIR_L;
			}
			else
			{
				dir = CommonUtility::DIR_R;
			}
		}

		//敵からプレイヤーへの位置ベクトルを作成
		float angle = atan2(lookAt.x, lookAt.z);
		SetGoalRotate(angle);
	}
}
