#include <DxLib.h>
#include "../../Application.h"
#include "Transitor.h"

void Transitor::Start(void)
{
	const auto& size = Application::GetInstance().GetWindowSize();

	oldRT_ = MakeScreen(size.width_, size.height_);
	newRT_ = MakeScreen(size.width_, size.height_);
}
