#include <vector>
#include <algorithm>
#include "../../Utility/AsoUtility.h"
#include "../../Common/DebugDrawFormat.h"
#include "../../Manager/Generic/Camera.h"
#include "../../Object/Customer/CustomerManager.h"
#include "../../Object/Order/OrderManager.h"
#include "../../Object/Stage/StageObject.h"
#include "../Generic/SceneManager.h"
#include "OrderCustomerManager.h"

OrderCustomerManager::OrderCustomerManager(void):
	orderMng_(nullptr),
	customerMng_(nullptr),
	isServe_(false)
{
}

OrderCustomerManager::~OrderCustomerManager(void)
{
}

void OrderCustomerManager::Init(void)
{
	//注文
	orderMng_ = std::make_unique<OrderManager>();
	orderMng_->Init();

	//客
	customerMng_ = std::make_shared<CustomerManager>();

	//生成されている注文内容を参照して種類を決めて生成
	CreateCustomersByOrders();
	customerMng_->Init();

	isServe_ = false;
}

void OrderCustomerManager::Update(void)
{
	//注文の更新
	customerMng_->Update(orderMng_->GetFirstOrder().time_);

	//入口からカウンターまで動かす用
	if (!customerMng_->CheckFirstCustomerCol())
	{
		customerMng_->StartMove();
		//着いたら動かないように&注文を表示
		if (customerMng_->CheckFirstCustomerCol())
		{
			customerMng_->StopMove();
		}
	}
	
	//注文の制限時間が過ぎたら追加生成を行う
	if (orderMng_->IsFirstOrderTimeOut() || isServe_)
	{
		if(!isServe_)customerMng_->SetCustomerReacton(0);

		//お客を移動させる
		customerMng_->StartMove();

		//お客の移動が終わったら削除と生成を行う
		if (customerMng_->CheckSecondCustomerCol())
		{
			//注文とお客を追加生成
			ClearOrderAndCustomer();

			//追加生成
			AddOrdersAndCustomers();
			isServe_ = false;
		}
	}
	else if(customerMng_->CheckFirstCustomerCol())
	{
		//制限時間を過ぎてなかったら先頭のオーダーだけ更新
		orderMng_->FirstOrderUpdate();
	}
}

void OrderCustomerManager::Draw(void)
{
	customerMng_->Draw();
}

void OrderCustomerManager::CreateCustomersByOrders(void)
{
	std::vector<Order::OrderData> data = orderMng_->GetAllOrder();
	for(int i = 0; i < data.size();++i)
	{
		customerMng_->CreateSingleCustomer(data[i]);
	}
}

void OrderCustomerManager::AddOrdersAndCustomers(void)
{
	//注文とお客を追加生成、位置調整
	orderMng_->AddCreateOrder();
	AddCustomerByOrder();
}

void OrderCustomerManager::ClearOrderAndCustomer(void)
{
	//先頭の注文とお客を削除
	orderMng_->ClearFirstOrder();
	customerMng_->ClearFirstCustomers();
}

void OrderCustomerManager::AddCustomerByOrder(void)
{
	//お客を追加生成
	customerMng_->CreateSingleCustomer(orderMng_->GetLastOrderData());
}

int OrderCustomerManager::GetOrderScore(const Order::OrderData serve)
{
	Order::DRINK serveDrink = serve.drink_;
	Order::SWEETS serveSweets = serve.sweets_;
	bool serveDrinkLid = serve.lid_;
	const auto& order = orderMng_->GetFirstOrder();
	int score = 0;

	// ドリンク判定
	if (order.drink_ != Order::DRINK::NONE) 
	{
		if (serve.drink_ == order.drink_) 
		{
			score += DRINK_SCORE;
			if (serve.lid_) score += LID_SCORE;
		}
		else	//ドリンクが違う場合はマイナス
		{
			score -= DRINK_SCORE;
		}
	}
	// スイーツ判定
	if (order.sweets_ != Order::SWEETS::NONE) 
	{
		if (serve.sweets_ == order.sweets_) 
		{
			score += SWEETS_SCORE;
		}
		else 
		{
			score -= SWEETS_SCORE;
		}
	}
	// 時間ボーナス
	if (order.num_ == 1) 
	{
		if (order.time_ > TIME_EARLY_LIMIT) score += TIME_EARLY_BONUS;
		else if (order.time_ > TIME_LIMIT) score += TIME_BONUS;
	}
	else if (order.num_ == 2) 
	{
		if (order.time_ > (TIME_EARLY_LIMIT * 2.0f)) score += TIME_EARLY_BONUS;
		else if (order.time_ > (TIME_LIMIT * 2.0f)) score += TIME_BONUS;
	}

	customerMng_->SetCustomerReacton(score);
	return score;
}

bool OrderCustomerManager::GetIsMoving(void)
{
	return customerMng_->GetIsMove();
}

Order::OrderData OrderCustomerManager::GetOrderData(void) const
{
	return orderMng_->GetFirstOrder();
}

void OrderCustomerManager::CheckServeAndOrder(const Order::OrderData serve)
{
	Order::DRINK serveDrink = serve.drink_;
	Order::SWEETS serveSweets = serve.sweets_;
	bool serveDrinkLid = serve.lid_;
	const auto& order = orderMng_->GetFirstOrder();

	// ドリンク判定
	if (order.drink_ != Order::DRINK::NONE)
	{
		if (serve.drink_ == order.drink_)
		{
			customerMng_->IsCheckUI(0,true);
		}
		else
		{
			customerMng_->IsCheckUI(0, false);
		}
	}
	// スイーツ判定
	if (order.sweets_ != Order::SWEETS::NONE)
	{
		if (serve.sweets_ == order.sweets_)
		{
			customerMng_->IsCheckUI(1,true);
		}
		else {
			customerMng_->IsCheckUI(1, false);
		}
	}
}
