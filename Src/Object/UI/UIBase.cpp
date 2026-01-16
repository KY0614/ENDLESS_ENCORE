#include "UIBase.h"

UIBase::UIBase(void):
	uiSrc_(ResourceManager::SRC::NONE),
	uiBackSrc_(ResourceManager::SRC::NONE),
	uiImg_(-1),
	uiBackImg_(-1),
	alpha_(0.0f),
	isActive_(false)
{
}