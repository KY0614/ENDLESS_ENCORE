#include "UIBase.h"

UIBase::UIBase(void):
	uiSrc_(ResourceManager::SRC::NONE),
	uiBackSrc_(ResourceManager::SRC::NONE),
	uiImg_(-1),
	isActive_(false)
{
}