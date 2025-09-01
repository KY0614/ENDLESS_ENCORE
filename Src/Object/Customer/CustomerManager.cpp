#include "../../Utility/AsoUtility.h"
#include "../../Object/Order/Order.h"
#include "../../Object/Customer/HotCustomer.h"
#include "../../Object/Customer/IceCustomer.h"
#include "../../Object/UI/OrderUI.h"
#include "../../Object/UI/UIManager.h"
#include "CustomerManager.h"

namespace
{
	//生成する上限
	const int MAX_CREATE_SIZE = 6;

	//お客同士の間隔
	const float CUSTOMERS_SPACE = 90.0f;

	//注文UIの表示位置
	const float ORDER_UI_OFFSET_X = 130.0f;		//X座標のオフセット
	const float ORDER_UI_OFFSET_Y = 220.0f;		//Y座標のオフセット
}

CustomerManager::CustomerManager(void)
{
	isCustomersMove_ = false;
	cnt_ = 0;
}

CustomerManager::~CustomerManager(void)
{
	customers_.clear();
}

void CustomerManager::Init(void)
{
	//お客らの初期位置を設定
	InitCustomersPos();

	isCustomersMove_ = true;
	cnt_ = 0;
}

void CustomerManager::Update(float orderTime)
{
	//お客の更新処理
	for (auto& c : customers_)
	{
		c->Update();
	}
	//注文UIの更新処理
	for (auto& ui : orderUI_)
	{
		ui->Update();
	}
	//お客が動いていたら、全員の位置を更新＆歩行アニメーションを再生
	if (isCustomersMove_)
	{
		for (auto& c : customers_)
		{
			c->SetState(CustomerBase::STATE::WALK);
			c->Move();
		}
	}

	//UI座標をお客の座用と合わせる
	for (int i = 0; i < customers_.size(); ++i)
	{
		VECTOR pos = VAdd(customers_[i]->GetPos(),
			VGet(ORDER_UI_OFFSET_X, ORDER_UI_OFFSET_Y,0.0f));
		orderUI_[i]->SetPos(pos);
		orderUI_[i]->SetOrderTimer(orderTime);
	}

	//カウンターの前に来たら、回転させてカウンターの方を見るようにする
	if (customers_[cnt_]->CollisionCounter())
	{
		if (customers_[cnt_]->CheckCounterToCustomer())
		{
			customers_[cnt_]->SetGoalRotate(AsoUtility::Deg2RadF(90.0f));
			isCustomersMove_ = false;
			orderUI_[cnt_]->SetActive(true);
			for (auto& c : customers_)
			{
				c->SetState(CustomerBase::STATE::IDLE);
			}
		}
	}
}

void CustomerManager::Draw(void)
{
	for (auto& c : customers_)
	{
		c->Draw();
	}
}

void CustomerManager::CreateSingleCustomer(Order::OrderData data)
{
	//注文用UIを生成
	orderUI_.emplace_back(std::make_unique<OrderUI>(
		data.drink_, data.sweets_,data.time_));
	//注文に応じたお客を生成
	switch (data.drink_)
	{
	case Order::DRINK::NONE:
		break;

	case Order::DRINK::HOT:
		customers_.emplace_back(std::make_unique<HotCustomer>());
		customers_.back()->Init(GetLastCustomerPos());
		{
			VECTOR pos = VAdd(
				GetLastCustomerPos(),
				VGet(ORDER_UI_OFFSET_X, ORDER_UI_OFFSET_Y, 0.0f));
			orderUI_.back()->Init();
			orderUI_.back()->SetPos(pos);
		}
		break;

	case Order::DRINK::ICE:
		customers_.emplace_back(std::move(std::make_unique<IceCustomer>()));
		customers_.back()->Init(GetLastCustomerPos());
		{
			VECTOR pos = VAdd(
				GetLastCustomerPos(),
				VGet(ORDER_UI_OFFSET_X, ORDER_UI_OFFSET_Y, 0.0f));
			orderUI_.back()->Init();
			orderUI_.back()->SetPos(pos);
		}
		break;

	default:
		break;
	}
	//UIをManagerに登録
	UIManager::GetInstance().AddOrderUI(orderUI_.back().get());
}

void CustomerManager::ClearFirstCustomers(void)
{
	if (!customers_.empty())
	{
		//お客を削除
		customers_[cnt_]->IsNotVisible();
		orderUI_[cnt_]->SetActive(false);
		cnt_++;
	}
}

void CustomerManager::SetCustomerReacton(int score)
{
	if (score >= 80)
	{
		customers_[cnt_]->SetReaction(CustomerBase::REACTION::GOOD);
	}
	else if (score > 50)
	{
		customers_[cnt_]->SetReaction(CustomerBase::REACTION::SOSO);
	}
	else
	{
		customers_[cnt_]->SetReaction(CustomerBase::REACTION::BAD);
	}
}

VECTOR CustomerManager::GetLastCustomerPos(void) const
{
	//座標を返す
	VECTOR retPos;
	//最後のお客の位置を取得
	retPos = customers_[cnt_]->GetPos();
	//最後のお客の位置から間隔をあけて左にずらす
	retPos.x -= ((MAX_CREATE_SIZE - 1) * CUSTOMERS_SPACE);
	return retPos;
}

bool CustomerManager::CheckFirstCustomerCol(void)
{
	bool ret = false;
	//先頭のお客がカウンター前にいるかどうか
	if (customers_[cnt_]->CollisionCounter())
	{
		ret = true;
	}
	return ret;
}

bool CustomerManager::CheckSecondCustomerCol(void)
{
	bool ret = false;
	//2番目のお客がカウンター前にいるかどうか
	if (customers_[cnt_ + 1]->CollisionCounter())
	{
		ret = true;
	}
	return ret;
}

void CustomerManager::IsCheckUI(const int index, const bool isActive)
{
	orderUI_[cnt_]->SetCheckUI(index, isActive);
}

void CustomerManager::InitCustomersPos(void)
{
	//お客の初期位置設定
	VECTOR pos = CustomerBase::CUSTOMER_POS;
	//全員の位置をx軸で左にずらす
	for (int i = 0; i < MAX_CREATE_SIZE; i++)
	{
		auto& customer = customers_[i];
		//一人ずつずらす
		pos.x -= CUSTOMERS_SPACE;
		customer->Init(pos);
	}
}
