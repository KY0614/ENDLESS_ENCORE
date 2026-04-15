#include "BarUI.h"

BarUI::BarUI(void)
{
	barUiInfo_ = {};
	barMaxWidth_ = 0;

	//バーUI背景画像のリソースID
	barUIFrameImg_ = -1;

	parryBarImg_ = -1;
	parryCDBarImg_ = -1;
}

BarUI::~BarUI(void)
{
}

void BarUI::Init(void)
{
	//UI画像のハンドル取得
	uiImg_ = ResourceManager::GetInstance().Load(uiSrc_).handleId_;
	//UI背景画像のハンドル取得
	uiBackImg_ = ResourceManager::GetInstance().Load(uiBackSrc_).handleId_;
	//UIフレーム画像のハンドル取得
	barUIFrameImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_HP_BAR_FRAME).handleId_;
	
	parryBarImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_PARYY_BAR).handleId_;
	parryCDBarImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_PARYY_CD_BAR).handleId_;
}

void BarUI::Update(void)
{
	if (!isActive_)return;
}

void BarUI::Draw(void)
{
	if (!isActive_)return;

	//バーの背景描画
	const int frameOffset = 2;
	DrawExtendGraph(
		barUiInfo_.pos_.x - frameOffset, barUiInfo_.pos_.y - frameOffset,
		barUiInfo_.pos_.x + barMaxWidth_ + frameOffset,
		barUiInfo_.pos_.y + barUiInfo_.size_.y + frameOffset,
		barUIFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barMaxWidth_,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		uiBackImg_,
		true
	);

	//バーの描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barUiInfo_.size_.x,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		uiImg_,
		true
	);
}

void BarUI::DrawParry(void)
{
	if (!isActive_)return;

	//バーの背景描画
	const int frameOffset = 2;
	DrawExtendGraph(
		barUiInfo_.pos_.x - frameOffset, barUiInfo_.pos_.y - frameOffset,
		barUiInfo_.pos_.x + barMaxWidth_ + frameOffset,
		barUiInfo_.pos_.y + barUiInfo_.size_.y + frameOffset,
		barUIFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barMaxWidth_,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		uiBackImg_,
		true
	);
	//バーの描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barUiInfo_.size_.x,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		parryBarImg_,
		true
	);
}

void BarUI::DrawParryCD(void)
{
	if (!isActive_)return;

	//バーの背景描画
	const int frameOffset = 2;
	DrawExtendGraph(
		barUiInfo_.pos_.x - frameOffset, barUiInfo_.pos_.y - frameOffset,
		barUiInfo_.pos_.x + barMaxWidth_ + frameOffset,
		barUiInfo_.pos_.y + barUiInfo_.size_.y + frameOffset,
		barUIFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barMaxWidth_,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		uiBackImg_,
		true
	);
	//バーの描画
	DrawExtendGraph(
		barUiInfo_.pos_.x, barUiInfo_.pos_.y,
		barUiInfo_.pos_.x + barUiInfo_.size_.x,
		barUiInfo_.pos_.y + barUiInfo_.size_.y,
		parryCDBarImg_,
		true
	);
}

void BarUI::SetBarUISrc(const ResourceManager::SRC uiSrc, const ResourceManager::SRC uiBackSrc)
{
	uiSrc_ = uiSrc;
	uiBackSrc_ = uiBackSrc;
}

void BarUI::SetBarPos(const Vector2 pos)
{
	barUiInfo_.pos_ = pos;
}

void BarUI::SetBarSize(const Vector2 size)
{
	barUiInfo_.size_ = size;
}
