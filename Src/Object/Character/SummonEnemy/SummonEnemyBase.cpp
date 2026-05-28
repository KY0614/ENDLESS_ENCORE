#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Player.h"
#include "SummonEnemyBase.h"

namespace
{
	//‰ñ“]‚É‚©‚¯‚éŠÔ
	const float TIME_ROT = 0.1f;		
}

SummonEnemyBase::SummonEnemyBase(Player& player):
	player_(player)
{
	state_ = STATE::NONE;
	isSummoned_ = false;
	enemyRotY_ = Quaternion::Quaternion();
	goalQuaRot_ = Quaternion::Quaternion();
	stepRotTime_ = 0.0f;
	followSpeed_ = 0.0f;
}

SummonEnemyBase::~SummonEnemyBase(void)
{
}

void SummonEnemyBase::ChangeState(const STATE& state)
{
	state_ = state;

	//Šeó‘Ô‘JˆÚ‚Ì‰Šúˆ—
	stateChanges_[state_]();
}

void SummonEnemyBase::SetGoalRotate(double rotRad)
{
	Quaternion axis =
		Quaternion::AngleAxis(
			rotRad, CommonUtility::AXIS_Y);

	//Œ»İİ’è‚³‚ê‚Ä‚¢‚é‰ñ“]‚Æ‚ÌŠp“x·‚ğæ‚é
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	//‚µ‚«‚¢’l
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}
	//–Ú•W‰ñ“]‚ğİ’è
	goalQuaRot_ = axis;
}

void SummonEnemyBase::Rotate(void)
{
	//‰ñ“]ŠÔ‚ÌŒ¸­
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();

	//‰ñ“]‚Ì‹…–Ê•âŠÔ
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

	//d—Í•ûŒü‚É‰ˆ‚Á‚Ä‰ñ“]‚³‚¹‚é
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(enemyRotY_);
}

void SummonEnemyBase::Rotate2Player(void)
{
	//ƒvƒŒƒCƒ„[‚ÌÀ•W‚©‚ç“G‚ÌÀ•W‚ğˆø‚­
	VECTOR lookAt;
	lookAt = VSub(player_.GetTransform().pos, transform_.pos);
	//atan2 ‚ÅŠp“x‚ğŒvZ
	float angle = atan2(lookAt.x, lookAt.z);
	SetGoalRotate(angle);

	//‰ñ“]ˆ—
	Rotate();
}