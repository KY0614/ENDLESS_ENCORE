#include <DxLib.h>
#include "../Utility/StringUtility.h"
#include "Tutorial.h"

Tutorial::Tutorial(const TutorialStep& firstStep)
{
	//最初のチュートリアルを追加
	step_.push_back(firstStep);
	state_ = STATE::NONE;
	currentStepIndex_ = 0;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Tutorial::ChangeStateNone, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::CAMERA, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::DASH, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::JUMP, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::DODGE, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::PARRY, std::bind(&Tutorial::ChangeStateMove, this));
}

Tutorial::~Tutorial(void)
{
}

void Tutorial::Init(void)
{
	ChangeState(step_.front().state_);
}

void Tutorial::Update(void)
{
	//更新ステップ
	stateUpdate_();
}

void Tutorial::Draw(void)
{
	DrawFormatString(
		step_.front().pos_.x,
		step_.front().pos_.y,
		0xFFFFFF,
		StringUtility::StringToWstring(step_.front().keyGuide_).c_str());
}

void Tutorial::AddTutorialStep(const TutorialStep& tutorialStep)
{
	step_.push_back(tutorialStep);
}

void Tutorial::ChangeState(STATE type)
{
}

void Tutorial::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateNone, this);
}

void Tutorial::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateMove, this);
}

void Tutorial::ChangeStateCamera(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateCamera, this);
}

void Tutorial::ChangeStateJump(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateJump, this);
}

void Tutorial::ChangeStateDodge(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateDodge, this);
}

void Tutorial::ChangeStateParry(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateParry, this);
}

void Tutorial::UpdateNone(void)
{//何もしない
}

void Tutorial::UpdateMove(void)
{

}

void Tutorial::UpdateCamera(void)
{
}

void Tutorial::UpdateJump(void)
{
}

void Tutorial::UpdateDodge(void)
{
}

void Tutorial::UpdateParry(void)
{
}
