#include <DxLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Utility/StringUtility.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "Tutorial.h"

Tutorial::Tutorial(const TutorialStep& firstStep)
{
	//最初のチュートリアルを追加
	step_.push_back(firstStep);
	state_ = STATE::NONE;
	viewGuide_ = "";
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Tutorial::ChangeStateNone, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::CAMERA, std::bind(&Tutorial::ChangeStateCamera, this));
	stateChanges_.emplace(STATE::DASH, std::bind(&Tutorial::ChangeStateDash, this));
	stateChanges_.emplace(STATE::JUMP, std::bind(&Tutorial::ChangeStateJump, this));
	stateChanges_.emplace(STATE::DODGE, std::bind(&Tutorial::ChangeStateDodge, this));
	stateChanges_.emplace(STATE::PARRY, std::bind(&Tutorial::ChangeStateParry, this));
}

Tutorial::~Tutorial(void)
{
}

void Tutorial::Init(void)
{
	viewGuide_ = step_.front().keyGuide_;
	ChangeState(step_.front().state_);
}

void Tutorial::Update(void)
{
	//更新ステップ
	stateUpdate_();
}

void Tutorial::Draw(void)
{
	//表示するチュートリアルが無ければ終了
	if (step_.front().state_ == STATE::NONE)return;

	if (CheckHitKeyAll() > 0)viewGuide_ = step_.front().keyGuide_;
	else if (GetJoypadInputState(DX_INPUT_PAD1) > 0)viewGuide_ = step_.front().controllerGuide_;

	DrawFormatString(
		step_.front().pos_.x,
		step_.front().pos_.y,
		0xFFFFFF,
		StringUtility::StringToWstring(viewGuide_).c_str());
}

void Tutorial::AddTutorialStep(const TutorialStep& tutorialStep)
{
	step_.push_back(tutorialStep);
}

void Tutorial::NextStep(void)
{
	//時間と回数が0以下になったらクリア
	if (step_.front().requiredTime_ <= 0.0f &&
		step_.front().requiredNum_ <= 0)
	{
		step_.erase(step_.begin());
		if(step_.empty())
		{
			//チュートリアル終了
			TutorialStep endStep;
			endStep.state_ = STATE::NONE;
			step_.push_back(endStep);
		}
		//状態変更
		ChangeState(step_.front().state_);
		return;
	}
}

void Tutorial::ChangeState(STATE state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
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

void Tutorial::ChangeStateDash(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateDash, this);
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
	InputManager& ins = InputManager::GetInstance();
	
	if(ins.IsInputPressed("Up") ||
	   ins.IsInputPressed("Down") ||
	   ins.IsInputPressed("Left") ||
	   ins.IsInputPressed("Right"))
	{
		step_.front().requiredTime_ -= SceneManager::GetInstance().GetDeltaTime();
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateCamera(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputPressed("CameraUp") ||
		ins.IsInputPressed("CameraDown") ||
		ins.IsInputPressed("CameraRight") ||
		ins.IsInputPressed("CameraLeft"))
	{
		step_.front().requiredTime_ -= SceneManager::GetInstance().GetDeltaTime();
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateDash(void)
{
}

void Tutorial::UpdateJump(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputTriggered("Jump"))
	{
		step_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateDodge(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputTriggered("Dodge"))
	{
		step_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateParry(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputTriggered("Parry"))
	{
		step_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateImGui(void)
{
	ImGui::Text("Step STATE: %d", step_.front().state_);
	ImGui::Text("Step Time: %.2f", step_.front().requiredTime_);
	ImGui::Text("Step Num: %d", step_.front().requiredNum_);
}
