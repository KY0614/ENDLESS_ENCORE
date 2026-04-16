#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "ParryBar.h"

namespace
{
	//フォントサイズ
	const int FONT_SIZE = 32;
}

ParryBar::ParryBar(const ParryBarInfo parryBarInfo,
	const float& parryCD,
	const float& parryCDMax):
	parryBarInfo_(parryBarInfo),
	parryCD_(parryCD),
	parryCDMax_(parryCDMax)
{
	parryCDBarImg_ = -1;
	barFrameImg_ = -1;
	barBackImg_ = -1;
	fontHandle_ = -1;
}

ParryBar::~ParryBar(void)
{
	DeleteFontToHandle(fontHandle_);
}

void ParryBar::Init(void)
{
	//初期状態は表示
	isActive_ = true;

	//パリィバーの画像の初期化
	InitImage();
	
	//フォントハンドルの取得
	fontHandle_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TUTORIAL_FONT).handleId_;
}

void ParryBar::Update(void)
{
	if (!isActive_)return;
}

void ParryBar::Draw(void)
{
	if (!isActive_)return;

	//バーの背景描画
	const int frameOffset = 2;
	DrawExtendGraph(
		parryBarInfo_.pos_.x - frameOffset, parryBarInfo_.pos_.y - frameOffset,
		parryBarInfo_.pos_.x + parryBarInfo_.size_.x + frameOffset,
		parryBarInfo_.pos_.y + parryBarInfo_.size_.y + frameOffset,
		barFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		parryBarInfo_.pos_.x, parryBarInfo_.pos_.y,
		parryBarInfo_.pos_.x + parryBarInfo_.size_.x,
		parryBarInfo_.pos_.y + parryBarInfo_.size_.y,
		barBackImg_,
		true
	);

	//パリィのクールダウンの進行度を0.0f～1.0fで表す
	float parryCDRatio = parryCD_ / parryCDMax_;
	//クールダウンの進行度に応じたバーの幅を計算
	int parryCD = static_cast<int>(parryBarInfo_.size_.x * parryCDRatio);
	//テキストを少しずらす用の幅
	const int textOffset = 10;
	// ゲージの色
	const int parryColor = 0x00FFFF; // 前景色（水色：パリィ可能）
	const int parryCDColor = 0xAA6600; // クールダウン中の色（オレンジ）
	if(parryCD_ > 0.0f)
	{
		//クールダウン中のバーの描画
		DrawExtendGraph(
			parryBarInfo_.pos_.x, parryBarInfo_.pos_.y,
			parryBarInfo_.pos_.x + parryCD,
			parryBarInfo_.pos_.y + parryBarInfo_.size_.y,
			parryCDBarImg_,
			true
		);
		//クールダウン中のテキストを描画
		DrawFormatStringToHandle(
			parryBarInfo_.pos_.x + parryBarInfo_.size_.x + textOffset,
			parryBarInfo_.pos_.y,
			parryCDColor, fontHandle_,	//色とフォント
			L"PARRY CD: %.1f",
			parryCDMax_ - parryCD_);
	}
	else
	{
		//バーの描画
		DrawExtendGraph(
			parryBarInfo_.pos_.x, parryBarInfo_.pos_.y,
			parryBarInfo_.pos_.x + parryBarInfo_.size_.x,
			parryBarInfo_.pos_.y + parryBarInfo_.size_.y,
			uiImg_,
			true
		);
		//クールダウンが完了している場合のテキストを描画
		DrawStringToHandle(
			parryBarInfo_.pos_.x + parryBarInfo_.size_.x + textOffset,
			parryBarInfo_.pos_.y,
			L"PARRY READY",
			parryColor,
			fontHandle_);
	}
}

void ParryBar::InitImage(void)
{
	//UI画像のハンドル取得
	uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_PARYY_BAR).handleId_;
	parryCDBarImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_PARYY_CD_BAR).handleId_;
	//UI背景画像のハンドル取得
	barBackImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_BACK).handleId_;
	//UIフレーム画像のハンドル取得
	barFrameImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_FRAME).handleId_;
}