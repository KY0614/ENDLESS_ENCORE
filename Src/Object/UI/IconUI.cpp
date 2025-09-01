#include "IconUI.h"

IconUI::IconUI(VECTOR localPos,VECTOR parentPos,ResourceManager::SRC src) :
	localPos_(localPos), parentPos_(parentPos), imgSrc_(src)
{
}

void IconUI::Init(void)
{
	uiImg_ = ResourceManager::GetInstance().Load(
		imgSrc_).handleId_;

	//追従先の座標にベクトル加算
	pos_ = VAdd(parentPos_, localPos_);
}

void IconUI::Update(void)
{
}

void IconUI::Draw(void)
{
	if (!isActive_) return;
	DrawBillboard3D(pos_,
		0.5f, 0.5f, 70.0f,
		0.0f, uiImg_, true);
}
