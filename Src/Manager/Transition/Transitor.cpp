#include <DxLib.h>
#include "../../Application.h"
#include "Transitor.h"
#include "FadeTransitor.h"

Transitor::Transitor(int interval) :
	interval_(interval),
	oldRT_(0),
	newRT_(0),
	frame_(0),
	isStart_(false)
{
}

Transitor::~Transitor(void)
{
	DeleteGraph(oldRT_);
	DeleteGraph(newRT_);
}

void Transitor::Start(void)
{
	const auto& size = Application::GetInstance().GetWindowSize();
	//新しい画面（遷移先)と古い画面（遷移元)で、RenderTargetを作っておきます
	oldRT_ = MakeScreen(size.width_, size.height_);
	newRT_ = MakeScreen(size.width_, size.height_);

	//現在表示中の画面をoldRT_にコピー
	int result = GetDrawScreenGraph(0, 0, size.width_, size.height_,oldRT_);
	frame_ = 0;
	isStart_ = true;
}
