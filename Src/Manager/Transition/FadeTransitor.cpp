#include <DxLib.h>
#include "FadeTransitor.h"

FadeTransitor* FadeTransitor::instance_ = nullptr;

void FadeTransitor::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new FadeTransitor();
	}
}

FadeTransitor& FadeTransitor::GetInstance(void)
{
	return *instance_;
}

FadeTransitor::FadeTransitor(int interval) :
	Transitor(interval)
{
}

void FadeTransitor::Update(void)
{
	if (frame_ < interval_) {
		++frame_;
		SetDrawScreen(newRT_);
	}
	else if (frame_ == interval_) {
		SetDrawScreen(DX_SCREEN_BACK);
	}
}

void FadeTransitor::Draw(void)
{
	if (IsEnd()) {
		return;
	}
	SetDrawScreen(DX_SCREEN_BACK);
	auto rate = (float)frame_ / (float)interval_;
	DrawGraph(0, 0, oldRT_, true);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, rate * 255);
	DrawGraph(0, 0, newRT_, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

