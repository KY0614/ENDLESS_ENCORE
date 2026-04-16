#include "SkipBar.h"

SkipBar::SkipBar(const SkipBarUIInfo skipBarInfo,
	const float& skipTime,
	const float& skipTimeMax):
	skipBarInfo_(skipBarInfo),
	skipTime_(skipTime),
	skipTimeMax_(skipTimeMax)
{
	barUIFrameImg_ = -1;
	uiBackImg_ = -1;
}

SkipBar::~SkipBar(void)
{
}

void SkipBar::Init(void)
{
	//初期状態は表示
	isActive_ = true;

	//画像の初期化
	InitImage();
}

void SkipBar::Update(void)
{
	if (!isActive_)return;
}

void SkipBar::Draw(void)
{
	if (!isActive_)return;

	//バーのフレーム描画
	const int frameOffset = 2;
	DrawExtendGraph(
		skipBarInfo_.pos_.x - frameOffset, skipBarInfo_.pos_.y - frameOffset,
		skipBarInfo_.pos_.x + skipBarInfo_.size_.x + frameOffset,
		skipBarInfo_.pos_.y + skipBarInfo_.size_.y + frameOffset,
		barUIFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		skipBarInfo_.pos_.x, skipBarInfo_.pos_.y,
		skipBarInfo_.pos_.x + skipBarInfo_.size_.x,
		skipBarInfo_.pos_.y + skipBarInfo_.size_.y,
		uiBackImg_,
		true
	);
	//スキップの進行度を0.0f～1.0fで表す
	float skipRatio = skipTime_ / skipTimeMax_;
	//スキップの進行度に応じたバーの幅を計算
	int skipWidth = static_cast<int>(skipBarInfo_.size_.x * skipRatio);
	//バー本体の描画
	DrawExtendGraph(
		skipBarInfo_.pos_.x, skipBarInfo_.pos_.y,
		skipBarInfo_.pos_.x + skipWidth,
		skipBarInfo_.pos_.y + skipBarInfo_.size_.y,
		uiImg_,
		true
	);
}

void SkipBar::InitImage(void)
{
	//UI画像のハンドル取得
	uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_PARYY_BAR).handleId_;
	//UI背景画像のハンドル取得
	uiBackImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_BACK).handleId_;
	//UIフレーム画像のハンドル取得
	barUIFrameImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_FRAME).handleId_;
}