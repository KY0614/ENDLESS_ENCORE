#include "../Manager/Generic/SceneManager.h"
#include "HPBar.h"

HPBar::HPBar(const HPBarInfo& hpBarInfo,
	const float& hp):
	hpBarInfo_(hpBarInfo),
	hp_(hp),
	maxHp_(0.0f)
{
	barFrameImg_ = -1;
	barBackImg_ = -1;
}

HPBar::HPBar(const BillboardInfo& billboardInfo,
	const float& hp,
	const float& maxHp) :
	billboardInfo_(billboardInfo),
	hp_(hp),
	maxHp_(maxHp)
{
	barFrameImg_ = -1;
	barBackImg_ = -1;
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
		barFrameImg_,
		true
	);

	//バーの背景描画
	DrawExtendGraph(
		hpBarInfo_.pos_.x, hpBarInfo_.pos_.y,
		hpBarInfo_.pos_.x + hpBarInfo_.size_.x,
		hpBarInfo_.pos_.y + hpBarInfo_.size_.y,
		barBackImg_,
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

void HPBar::DrawWorld2Screen(void)
{
	if (!isActive_ || hp_ <= 0.0f)return;
	
	//描画座標を計算（ワールド座標→スクリーン座標）
	VECTOR drawPos = VAdd(*billboardInfo_.pos_,billboardInfo_.ofsset_);
	VECTOR pos = ConvWorldPosToScreenPos(drawPos);

	//画像拡大率
	float scale = 0.5f;
	scale *= SceneManager::GetInstance().GetScreenAspectRatio();	//画面比率に合わせる
	//額縁
	DrawRotaGraph(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		scale, 
		0.0f,
		barFrameImg_, 
		true,
		false
	);
	//背景
	DrawRotaGraph(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		scale,
		0.0f,
		barBackImg_,
		true,
		false
	);

	//画像の本来のサイズを取得
	int width, height;
	GetGraphSize(uiImg_, &width, &height);
	//描画サイズ
	float scaleX = width * billboardInfo_.scaleX_ * scale;
	float scaleY = height * billboardInfo_.scaleY_ * scale;

	//サイズを計算
	float barSizeX = scaleX / 2.0f;
	float barSizeY = scaleY / 2.0f;
	//HPの割合を計算
	float hp = hp_ / maxHp_;
	int barWidth = static_cast<int>(scaleX * hp);
	//バー本体
	DrawExtendGraph(
		static_cast<int>(pos.x - barSizeX),
		static_cast<int>(pos.y - barSizeY),
		static_cast<int>(pos.x - barSizeX + barWidth),
		static_cast<int>(pos.y + barSizeY),
		uiImg_,
		true
	);
}

void HPBar::InitImage(void)
{
	//UI背景画像のハンドル取得
	barBackImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_BACK).handleId_;
	//UIフレーム画像のハンドル取得
	barFrameImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BAR_FRAME).handleId_;

	//種類に応じた画像のハンドル取得
	if(hpBarInfo_.type_ == TYPE::PLAYER ||
		billboardInfo_.type_ == TYPE::PLAYER)
	{
		//プレイヤーのHPバー画像のハンドル取得
		uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_HP_BAR).handleId_;
	}
	if(hpBarInfo_.type_ == TYPE::ENEMY ||
		billboardInfo_.type_ == TYPE::ENEMY)
	{
		//敵のHPバー画像のハンドル取得
		uiImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::ENEMY_HP_BAR).handleId_;
	}
}
