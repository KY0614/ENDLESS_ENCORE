#include "OrderUI.h"
#include "GaugeUI.h"
#include "IconUi.h"
#include "PopUpUI.h"
#include "UIManager.h"

UIManager* UIManager::instance_ = nullptr;

void UIManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new UIManager();
	}
	instance_->Init();
}

UIManager& UIManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		UIManager::CreateInstance();
	}
	return *instance_;
}

UIManager::UIManager(void)
{
}

UIManager::~UIManager(void)
{
}

void UIManager::Init(void)
{
}

void UIManager::Update(void)
{
	for (auto& ui : gaugeUIs_) 
	{
		ui->Update();
	}	
	for (auto& ui : orderUIs_) 
	{
		ui->Update();
	}

}

void UIManager::PopUpUIUpdate(void)
{
	for (auto& ui : popUpUIs_)
	{
		ui->Update();
	}
}

void UIManager::Draw(void)
{
	if (gaugeUIs_.empty() && orderUIs_.empty())return;

	for (auto& ui : gaugeUIs_) 
	{
		ui->Draw();
	}	
	for (auto& ui : orderUIs_) 
	{
		ui->Draw();
	}
	for (auto& ui : iconUIs_)
	{
		ui->Draw();
	}
	for (auto& ui : popUpUIs_)
	{
		ui->Draw();
	}
}

void UIManager::Release(void)
{
	gaugeUIs_.clear();
	orderUIs_.clear();
	iconUIs_.clear();
	popUpUIs_.clear();
}

void UIManager::Destroy(void)
{
	gaugeUIs_.clear();
	orderUIs_.clear();
	iconUIs_.clear();
	popUpUIs_.clear();
	delete instance_;
}

void UIManager::AddGaugeUI(GaugeUI* ui)
{
	gaugeUIs_.emplace_back(ui);
}

void UIManager::AddOrderUI(OrderUI* ui)
{
	orderUIs_.emplace_back(ui);
}

void UIManager::AddIconUI(IconUI* ui)
{
	iconUIs_.emplace_back(ui);
}

void UIManager::AddPopUpUI(int score, const VECTOR& pos)
{
	popUpUIs_.emplace_back(std::make_unique<PopUpUI>(score, pos));
	popUpUIs_.back()->Init();
}
