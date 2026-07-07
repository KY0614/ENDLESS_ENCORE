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
	if(instance_ == nullptr)
	{
		CreateInstance();
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
}

void UIManager::Draw(void)
{
}

void UIManager::Release(void)
{
}

void UIManager::Destroy(void)
{
	//ƒVƒ“ƒOƒ‹ƒgƒ“‚Ì‰ð•ú
	delete instance_;
}