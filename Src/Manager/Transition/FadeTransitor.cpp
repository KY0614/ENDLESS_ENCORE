#include <DxLib.h>
#include "FadeTransitor.h"


void FadeTransitor::Update(void)
{
	if (frame_ < interval_)
	{
		++frame_;
		SetDrawScreen(true);	
	}
}

void FadeTransitor::Draw(void)
{
}

