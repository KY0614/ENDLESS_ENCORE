#include "../../Application.h"
#include "../../Manager/Generic/ResourceManager.h"
#include "../Common/AnimationController.h"
#include "IceCustomer.h"

IceCustomer::IceCustomer(void): CustomerBase()
{
}

void IceCustomer::Draw(void)
{
	//モデルが非表示なら描画しない
	if (!isVisible_) return;
	MV1DrawModel(transform_.modelId);
}

void IceCustomer::SetParam(void)
{
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ICE_CUSTOMER));
	//モデルの情報更新
	transform_.Update();

	//お客のタイプを設定
	SetType(TYPE::ICE);

	//表示中に設定
	isVisible_ = true;
}

void IceCustomer::InitAnimation(void)
{
	//アニメーションの初期化
	std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)STATE::IDLE, path + "Idle.mv1", IDLE_ANIM_SPEED);
	animationController_->Add((int)STATE::WALK, path + "Walk.mv1", WALK_ANIM_SPEED);
	//アニメーションの初期状態を設定
	animationController_->Play((int)STATE::IDLE);
}