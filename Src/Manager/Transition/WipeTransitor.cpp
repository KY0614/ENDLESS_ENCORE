#include <DxLib.h>
#include "WipeTransitor.h"

WipeTransitor::WipeTransitor(TransitDirection direction)
{
}

WipeTransitor::~WipeTransitor()
{
}

void WipeTransitor::Update(void)
{
	if (frame_ < interval_)
	{
		frame_++;
		SetDrawScreen(oldRT_);
	}
	else if (frame_ == interval_)
	{
		SetDrawScreen(DX_SCREEN_BACK);
	}
}

void WipeTransitor::Draw(void)
{
	if (IsEnd())return;

	SetDrawScreen(DX_SCREEN_BACK);
	DrawGraph(0, 0, newRT_, true);
	auto rate = (float)frame_ / (float)interval_;
	auto result = DrawBlendGraph(0, 0, oldRT_,true, gradationH_,
		255*rate,64);
}
