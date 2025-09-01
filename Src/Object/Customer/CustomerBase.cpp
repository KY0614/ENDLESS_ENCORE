#include<DxLib.h>
#include "../../Manager/Generic/SceneManager.h"
#include "../Common/AnimationController.h"
#include "../../Utility/AsoUtility.h"
#include "CustomerBase.h"

namespace//‚±‚Ìcpp“à‚Å‚µ‚©Žg‚í‚È‚¢’è”
{
	const VECTOR CUSTOMER_SCALE = { 0.7f, 0.7f, 0.7f };	//‚¨‹q‚Ì‘å‚«‚³
	const VECTOR COUNTER_POS = { 221.0f, 0.0f, 271.0f };//ƒJƒEƒ“ƒ^[‚ÌˆÊ’u
	const float CUSTOMER_ROT_Y = -90.0f;	//‚¨‹q‚Ì‰ŠúYŽ²‰ñ“]Šp“x
	const float MOVE_SPEED = 1.5f;			//‚¨‹q‚ÌˆÚ“®‘¬“x
	const float COUNTER_RADIUS = 30.0f;		//ƒJƒEƒ“ƒ^[‚Ì‹…‘Ì“–‚½‚è”»’è‚Ì”¼Œa
	const float ROT_TIME = 1.0f;			//‰ñ“]Š®—¹‚Ü‚Å‚ÌŽžŠÔ
}

CustomerBase::CustomerBase(void)
{
	animationController_ = nullptr;
	type_ = TYPE::NONE;
	state_ = STATE::IDLE;
	reaction_ = REACTION::NONE;

	isVisible_ = false;

	stepRotTime_ = -1.0f;
}

void CustomerBase::Init(const VECTOR pos)
{
	//‚¨‹q‚Ìƒpƒ‰ƒ[ƒ^[Ý’è
	SetParam();
	//ƒAƒjƒ[ƒVƒ‡ƒ“‰Šú‰»
	InitAnimation();
	//ƒ‚ƒfƒ‹î•ñ‰Šú‰»
	transform_.scl = CUSTOMER_SCALE;	//‘å‚«‚³
	transform_.pos = pos;				//ˆÊ’u‚ðˆø”‚ÅŽw’è
	transform_.quaRot = Quaternion();	//‰ñ“]‚Í‰Šú‰»
	transform_.quaRotLocal =			//ƒ[ƒJƒ‹‰ñ“]Ý’è
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(CUSTOMER_ROT_Y), 0.0f });
	//ƒ‚ƒfƒ‹î•ñ‚ðXV
	transform_.Update();
}

void CustomerBase::Update(void)
{
	transform_.Update();
	animationController_->Update();

	//ó‘Ô‚²‚Æ‚ÌƒAƒjƒ[ƒVƒ‡ƒ“
	StateAnimation();
	//YŽ²‰ñ“]
	RotateY();

	//YŽ²‰ñ“]‚ð”½‰f
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(customerRotY_);
}

void CustomerBase::Move(void)
{
	//‰E‚ÉˆÚ“®‚³‚¹‚é
	transform_.pos.x += MOVE_SPEED;
}

void CustomerBase::SetGoalRotate(double rotRad)
{ 
	//‰ñ“]Ž²‚ðYŽ²‚ÉÝ’è‚µ‚Ä‰ñ“]Šp“x‚ðÝ’è
	Quaternion axis =
		Quaternion::AngleAxis(
			rotRad, AsoUtility::AXIS_Y);

	//Œ»ÝÝ’è‚³‚ê‚Ä‚¢‚é‰ñ“]‚Æ‚ÌŠp“x·‚ðŽæ‚é
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	//Šp“x·‚ª0.1ˆÈã‚È‚ç‰ñ“]ŽžŠÔ‚ðÝ’è
	if (angleDiff > 0.1)
	{
		stepRotTime_ = ROT_TIME;
	}
	//–Ú•W‚Ì‰ñ“]‚ðÝ’è
	goalQuaRot_ = axis;
}

bool CustomerBase::CollisionCounter(void)
{
	//•\Ž¦’†‚Ì‚¨‹q‚ªƒJƒEƒ“ƒ^[‚Ì‹…‘Ì“–‚½‚è”»’è‚É“–‚½‚Á‚Ä‚¢‚é‚©
	if (AsoUtility::IsHitSpherePoint(COUNTER_POS, COUNTER_RADIUS,
		GetTransform().pos) && isVisible_)
	{
		//“–‚½‚Á‚Ä‚¢‚½‚çtrue‚ð•Ô‚·
		return true;
	}
	return false;
}

bool CustomerBase::CheckCounterToCustomer(void)
{
	//‚¨‹q‚ªƒJƒEƒ“ƒ^[‚ÌˆÊ’u‚ð’´‚¦‚»‚¤‚¾‚Á‚½‚çtrue‚ð•Ô‚·
	if (GetTransform().pos.x > COUNTER_POS.x)
	{
		return true;
	}
	return false;
}

void CustomerBase::RotateY(void)
{
	//‰ñ“]‚É‚©‚©‚éŽžŠÔ‚ðŒo‰ßŽž‚ÅŒ¸‚ç‚·
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();

	//‰ñ“]‚Ì‹…–Ê•âŠÔ
	customerRotY_ = Quaternion::Slerp(
		customerRotY_, goalQuaRot_, (ROT_TIME - stepRotTime_) / ROT_TIME);
}

void CustomerBase::StateAnimation(void)
{
	//ó‘Ô‚²‚Æ‚É‚æ‚éƒAƒjƒ[ƒVƒ‡ƒ“‚ÌØ‚è‘Ö‚¦
	switch (state_)
	{
	case CustomerBase::STATE::IDLE:	//‘Ò‹@ó‘Ô
		animationController_->Play((int)STATE::IDLE);
		break;

	case CustomerBase::STATE::WALK:	//•àsó‘Ô
		animationController_->Play((int)STATE::WALK);
		break;
	default:
		break;
	}
}
