#include <DxLib.h>
#include "../Libs/ImGui/imgui.h"
#include "../Common/DebugDrawFormat.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "OrderManager.h"
#include "Order.h"

Order::Order(void):
	orderData_(),
	orderNum_(-1)
{
}

Order::~Order(void)
{
}

void Order::TimerUpdate(void)
{
	//制限時間から経過時間を引いていく
	orderData_.time_ -= SceneManager::GetInstance().GetDeltaTime();
}

void Order::CreateOrder(void)
{
	int drinkType = 0;
	int sweetsType = 0;

	//注文数を決める
	orderNum_ = GetRand(ORDER_MAX_NUM);
	while(orderNum_ <= 0)
	{
		//乱数が0以下だったらもう一度乱数を取得する
		orderNum_ = GetRand(ORDER_MAX_NUM);
	}

	//注文内容を決める
	switch (orderNum_)
	{
	case ORDER_MIN_NUM:
		//飲み物の種類がNONE(0)以下だったら乱数取得しなおす
		while (drinkType <= 0)	
		{
			drinkType = GetRand(DRINK_MAX_NUM);
		}
		sweetsType = 0;		//注文数が１の時はスイーツは頼まないようにする
		//注文数を設定
		SetOrderNum(ORDER_MIN_NUM);
		//飲み物の種類を設定
		SetDrink(static_cast<DRINK>(drinkType));	
		//食べ物の種類を設定
		SetSweets(static_cast<SWEETS>(sweetsType));	
		//制限時間を設定
		SetOrderTime(ONE_ORDER_TIME);				
		break;

	case ORDER_MAX_NUM:
		//飲み物の種類がNONE(0)以下だったら乱数取得しなおす
		while (drinkType <= 0)	
		{
			drinkType = GetRand(DRINK_MAX_NUM);
		}

		while (sweetsType <= 0)	//スイーツの種類がNONE(0)以下だったら乱数取得しなおす
		{
			sweetsType = GetRand(FOOD_MAX_NUM);
		} 
		//注文数を設定
		SetOrderNum(ORDER_MAX_NUM);
		//飲み物の種類を設定
		SetDrink(static_cast<DRINK>(drinkType));
		//食べ物の種類を設定
		SetSweets(static_cast<SWEETS>(sweetsType));	
		//制限時間を設定
		SetOrderTime(TWO_ORDER_TIME);				
		break;

	default:
		break;
	}
}