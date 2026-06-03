#include "../Utility/CommonUtility.h"
#include "ShooterEnemy.h"

ShooterEnemy::ShooterEnemy(Player& player) :
	SummonEnemyBase(player)
{
	state_ = STATE::NONE;
	stepRotTime_ = 0.0f;
	isSummoned_ = false;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&ShooterEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&ShooterEnemy::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&ShooterEnemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&ShooterEnemy::ChangeStateAttack, this));
}

ShooterEnemy::~ShooterEnemy(void)
{
}

void ShooterEnemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();
}

void ShooterEnemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	transform_.Update();
}

void ShooterEnemy::Draw(void)
{
	//球体を仮で描画
	const float rad = 30.0f;
	const int div = 16;
	//青：遠距離攻撃タイプ
	int col = GetColor(0, 0, 255);
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

void ShooterEnemy::Init3DModel(void)
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

void ShooterEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateNone, this);
}

void ShooterEnemy::ChangeStateSummon(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateSummon, this);
}

void ShooterEnemy::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateMove, this);
}

void ShooterEnemy::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&ShooterEnemy::UpdateAttack, this);
}

void ShooterEnemy::UpdateNone(void)
{//何もしない
}

void ShooterEnemy::UpdateSummon(void)
{
	transform_.pos.y++;
	if (transform_.pos.y >= -100.0f)
	{
		transform_.pos.y = -100.0f;
		IsSummoned();
		ChangeState(STATE::MOVE);
	}
}

void ShooterEnemy::UpdateMove(void)
{
	//プレイヤーを見続ける
	Rotate2Player();

	//回転処理
	Rotate();
}

void ShooterEnemy::UpdateAttack(void)
{
	//攻撃処理
}