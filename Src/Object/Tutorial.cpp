#include <DxLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Utility/StringUtility.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "Tutorial.h"

Tutorial::Tutorial(const TutorialStep& firstStep)
{
	//最初のチュートリアルを追加
	tutorialStep_.push_back(firstStep);
	state_ = STATE::NONE;
	viewGuide_ = "";
	fontHandle_ = -1;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Tutorial::ChangeStateNone, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Tutorial::ChangeStateMove, this));
	stateChanges_.emplace(STATE::CAMERA, std::bind(&Tutorial::ChangeStateCamera, this));
	stateChanges_.emplace(STATE::DASH, std::bind(&Tutorial::ChangeStateDash, this));
	stateChanges_.emplace(STATE::JUMP, std::bind(&Tutorial::ChangeStateJump, this));
	stateChanges_.emplace(STATE::DODGE, std::bind(&Tutorial::ChangeStateDodge, this));
	stateChanges_.emplace(STATE::PARRY, std::bind(&Tutorial::ChangeStateParry, this));
	stateChanges_.emplace(STATE::STAGE, std::bind(&Tutorial::ChangeStateStage, this));
}

Tutorial::~Tutorial(void)
{
	DeleteFontToHandle(fontHandle_);
}

void Tutorial::Init(void)
{
	//最初の表示ガイドを設定
	viewGuide_ = tutorialStep_.front().keyGuide_;
	//フォント作成
	float screenAspect = SceneManager::GetInstance().GetScreenAspectRatio();
	const int fontSize = 32 * screenAspect;	//フォントサイズ
	const int fontThick = 3;				//フォントの太さ
	fontHandle_ = CreateFontToHandle(L"しねきゃぷしょん", fontSize, fontThick, DX_FONTTYPE_ANTIALIASING);
	//最初の状態へ変更
	ChangeState(tutorialStep_.front().state_);
}

void Tutorial::Update(void)
{
	//更新ステップ
	stateUpdate_();
}

void Tutorial::Draw(void)
{
	//表示するチュートリアルが無ければ終了
	if (tutorialStep_.front().state_ == STATE::NONE)return;

	if (CheckHitKeyAll() > 0)viewGuide_ = tutorialStep_.front().keyGuide_;
	else if (GetJoypadInputState(DX_INPUT_PAD1) > 0)viewGuide_ = tutorialStep_.front().controllerGuide_;

	DrawStringToHandle(
		tutorialStep_.front().pos_.x,
		tutorialStep_.front().pos_.y,
		StringUtility::StringToWstring(viewGuide_).c_str(),
		0xFFFFFF,
		fontHandle_);
}

void Tutorial::AddTutorialStep(const TutorialStep& tutorialStep)
{
	tutorialStep_.push_back(tutorialStep);
}

void Tutorial::NextStep(void)
{
	//時間と回数が0以下になったらクリア
	if (tutorialStep_.front().requiredTime_ <= 0.0f &&
		tutorialStep_.front().requiredNum_ <= 0)
	{
		tutorialStep_.erase(tutorialStep_.begin());
		if(tutorialStep_.empty())
		{
			//チュートリアル終了
			TutorialStep endStep;
			endStep.state_ = STATE::NONE;
			tutorialStep_.push_back(endStep);
		}
		//状態変更
		ChangeState(tutorialStep_.front().state_);
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

void Tutorial::ChangeStateStage(void)
{
	stateUpdate_ = std::bind(&Tutorial::UpdateStage, this);
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
		tutorialStep_.front().requiredTime_ -= SceneManager::GetInstance().GetDeltaTime();
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
		tutorialStep_.front().requiredTime_ -= SceneManager::GetInstance().GetDeltaTime();
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
		tutorialStep_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateDodge(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputTriggered("Dodge"))
	{
		tutorialStep_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateParry(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsInputTriggered("Parry"))
	{
		tutorialStep_.front().requiredNum_--;
	}
	//次のステップへ
	NextStep();
}

void Tutorial::UpdateStage(void)
{
}

void Tutorial::UpdateImGui(void)
{
	ImGui::Text("Step Time: %.2f", tutorialStep_.front().requiredTime_);
	ImGui::Text("Step Num: %d", tutorialStep_.front().requiredNum_);

	switch (state_)
	{
		case STATE::NONE:
		ImGui::Text("STATE: NONE");
		break;
		case STATE::MOVE:
		ImGui::Text("STATE: MOVE");
		break;
		case STATE::CAMERA:
		ImGui::Text("STATE: CAMERA");
		break;
		case STATE::PARRY:
		ImGui::Text("STATE: PARRY");
		break;
		case STATE::STAGE:
		ImGui::Text("STATE: STAGE");
		break;
		case STATE::DODGE:
		ImGui::Text("STATE: DODGE");
		break;
		case STATE::DASH:
		ImGui::Text("STATE: DASH");
		break;
		case STATE::JUMP:
		ImGui::Text("STATE: JUMP");
		break;
	}
}
