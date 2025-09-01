#include "../../Application.h"
#include "../Common/Easing.h"
#include "../../Manager/Generic/ResourceManager.h"
#include "../../Manager/Generic/SceneManager.h"
#include "OrderUI.h"

OrderUI::OrderUI(Order::DRINK drink, Order::SWEETS sweets, float maxTime)
{
	orderUIData_.drinkType_ = drink;
	orderUIData_.sweetsType_ = sweets;
	orderMaxTime_ = maxTime;

	orderUIData_.drinkUIImg_ = -1;
	orderUIData_.sweetsUIImg_ = -1;
	orderUIData_.backUIImg_ = -1;
	orderUIData_.timerBackImg_ = -1;
	orderUIData_.timerImg_ = -1;
	orderUIData_.checkImg_ = -1;
	isOrderCheck_.clear();
	orderTimer_ = 0.0f;
	orderUIData_.currentRate_ = 0.0f;
	orderUIData_.displayedRate_ = 0.0f;
	gaugeTime_ = 0.0f;
	isActive_ = false;
	alpha_ = 1.0f;
	size_ = 0.0f;
}

void OrderUI::Init(void)
{
	orderUIData_.backUIImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::UI_ORDER_BACK).handleId_;

	//ドリンク用UI
	if (orderUIData_.drinkType_ == Order::DRINK::HOT)
	{
		orderUIData_.drinkUIImg_ = ResourceManager::GetInstance().Load(
			ResourceManager::SRC::UI_HOT).handleId_;
	}
	else if (orderUIData_.drinkType_ == Order::DRINK::ICE)
	{
		orderUIData_.drinkUIImg_ = ResourceManager::GetInstance().Load(
			ResourceManager::SRC::UI_ICE).handleId_;
	}
	//スイーツ用UI
	if (orderUIData_.sweetsType_ == Order::SWEETS::CHOCO)
	{
		orderUIData_.sweetsUIImg_ = ResourceManager::GetInstance().Load(
			ResourceManager::SRC::UI_CHOCO).handleId_;
		isOrderCheck_.resize(2);
	}
	else if (orderUIData_.sweetsType_ == Order::SWEETS::STRAWBERRY)
	{
		orderUIData_.sweetsUIImg_ = ResourceManager::GetInstance().Load(
			ResourceManager::SRC::UI_BERRY).handleId_;
		isOrderCheck_.resize(2);
	}
	else
	{
		orderUIData_.sweetsUIImg_ = -1;
		isOrderCheck_.resize(1);
	}
	//タイマー背景画像
	orderUIData_.timerBackImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::UI_CIRCLESHADOW).handleId_;
	//タイマー画像
	orderUIData_.timerImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::UI_CIRCLE).handleId_;
	//チェック画像
	orderUIData_.checkImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::UI_CHECK).handleId_;

	//マークは最初は非表示
	for(auto check : isOrderCheck_)
	{
		check = false;
	}
	//画像サイズ
	size_ = IMG_SIZE;
}

void OrderUI::Update(void)
{
	if (!isActive_)return;
	orderUIData_.displayedRate_ = Easing::Linear(
		gaugeTime_, 1.0f,
		0.0f, orderUIData_.currentRate_);

	//スコアを現在のランクの範囲を比例計算する（後で100をかけてパーセントにする）
	orderUIData_.currentRate_ = orderTimer_ / orderMaxTime_;
	if (orderUIData_.currentRate_ <= 0.0f)orderUIData_.currentRate_ = 0.0f;
}

void OrderUI::Draw(void)
{
	if (isActive_ != true)return;
	//注文内容の背景を描画
	DrawBillboard3D(pos_, 0.5f, 0.5f, BACK_IMG_SIZE,
		0.0f, orderUIData_.backUIImg_, true);
	//スイーツとドリンクのUIを描画
	if (orderUIData_.sweetsType_ != Order::SWEETS::NONE)
	{
		VECTOR drinkPos = VAdd(pos_, VGet(-(size_ / 2.0f), 0.0f, 0.0f));
		VECTOR sweetsPos = VAdd(pos_, VGet((size_ / 2.0f),0.0f,0.0f));
		DrawBillboard3D(drinkPos, 0.5f, 0.5f, size_,
			0.0f, orderUIData_.drinkUIImg_, true);

		DrawBillboard3D(sweetsPos, 0.5f, 0.5f, size_,
			0.0f, orderUIData_.sweetsUIImg_, true);

		if(isOrderCheck_.front())//ドリンクのチェックマーク描画
		{
			DrawBillboard3D(drinkPos, 0.5f, 0.5f, size_,
				0.0f, orderUIData_.checkImg_, true);
		}

		if(isOrderCheck_.back())//スイーツのチェックマーク描画
		{
			DrawBillboard3D(sweetsPos, 0.5f, 0.5f, size_,
				0.0f, orderUIData_.checkImg_, true);
		}
	}
	else//スイーツがない場合はドリンクだけ描画
	{
		DrawBillboard3D(pos_, 0.5f, 0.5f, size_,
			0.0f, orderUIData_.drinkUIImg_, true);
		//チェックマーク描画
		if (isOrderCheck_.front())
		{
			DrawBillboard3D(pos_, 0.5f, 0.5f, size_,
				0.0f, orderUIData_.checkImg_, true);
		}
	}

	VECTOR pos = VAdd(pos_, VGet(120.0f, -50.0f, 0.0f));
	VECTOR screenPos = ConvWorldPosToScreenPos(pos);
	//タイマーの背景を描画
	DrawBillboard3D(pos,
		0.5f, 0.5f, size_,
		0.0f, orderUIData_.timerBackImg_, true);

	//画面の大きさに合わせて拡大率を変える
	float scale = 
		static_cast<float>(Application::SCREEN_SIZE_Y) / 
		static_cast<float>(Application::SCREEN_MAX_SIZE_Y);
	//タイマーの円ゲージを描画
	DrawCircleGauge(
		(int)screenPos.x,
		(int)screenPos.y,
		orderUIData_.currentRate_ * 100.0f,
		orderUIData_.timerImg_,
		0.0f,
		scale,
		false, false
	);
}
