#include "HPBar.h"

HPBar::HPBar(const HPBarInfo hpBarInfo, const float& hp):
	hpBarInfo_(hpBarInfo),
	hp_(hp)
{
	barUIFrameImg_ = -1;	
	uiBackImg_ = -1;
}

HPBar::~HPBar(void)
{
}

void HPBar::Init(void)
{
	//初期状態は表示
	isActive_ = true;

	//HPバーの画像の初期化
	InitImage();
}

void HPBar::Update(void)
{
	if (!isActive_)return;
}

void HPBar::Draw(void)
{
	if (!isActive_)return;
	//バーのフレーム描画（ふちのようなもの）
	const int frameOffset = 2;
	DrawExtendGraph(
		hpBarInfo_.pos_.x - frameOffset, hpBarInfo_.pos_.y - frameOffset,
		hpBarInfo_.pos_.x + hpBarInfo_.size_.x + frameOffset,
		hpBarInfo_.pos_.y + hpBarInfo_.size_.y + frameOffset,
		barUIFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		hpBarInfo_.pos_.x, hpBarInfo_.pos_.y,
		hpBarInfo_.pos_.x + hpBarInfo_.size_.x,
		hpBarInfo_.pos_.y + hpBarInfo_.size_.y,
		uiBackImg_,
		true
	);
	//HPの割合を計算
	float hp = hp_ / hpBarInfo_.size_.x;	
	int barWidth = static_cast<int>(hpBarInfo_.size_.x * hp);
	//バー本体の描画
	DrawExtendGraph(
		hpBarInfo_.pos_.x, hpBarInfo_.pos_.y,
		hpBarInfo_.pos_.x + barWidth,
		hpBarInfo_.pos_.y + hpBarInfo_.size_.y,
		uiImg_,
		true
	);
}

void HPBar::InitImage(void)
{
	//UI画像のハンドル取得
	uiImg_ = ResourceManager::GetInstance().Load(uiSrc_).handleId_;
	//UI背景画像のハンドル取得
	uiBackImg_ = ResourceManager::GetInstance().Load(uiBackSrc_).handleId_;
	//UIフレーム画像のハンドル取得
	barUIFrameImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_HP_BAR_FRAME).handleId_;

	//種類に応じた画像のハンドル取得
	if(hpBarInfo_.type_ == TYPE::PLAYER)
	{
		//プレイヤーのHPバー画像のハンドル取得
		uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_HP_BAR).handleId_;
	}
	else
	{
		//敵のHPバー画像のハンドル取得
		uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::ENEMY_HP_BAR).handleId_;
	}
}
