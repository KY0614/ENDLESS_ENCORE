#pragma once
#include <memory>
#include"../../Object/Customer/CustomerBase.h"
#include "../../Object/Order/Order.h"

class OrderManager;
class CustomerManager;
class StageObject;

class OrderCustomerManager
{
public:
	//注文のスコア
	static constexpr int DRINK_SCORE = 50;	//飲み物のスコア
	static constexpr int SWEETS_SCORE = 30;	//スイーツのスコア
	static constexpr int LID_SCORE = 20;	//スイーツのスコア

	static constexpr int TIME_EARLY_BONUS = 50;	//早い提供の時間ボーナス
	static constexpr int TIME_BONUS = 10;		//普通の提供の時間ボーナス

	static constexpr float TIME_EARLY_LIMIT = 6.0f;	//早い提供の時間制限
	static constexpr float TIME_LIMIT = 3.0f;	//普通提供の時間制限

	//コンストラクタ
	OrderCustomerManager(void);

	//デストラクタ
	~OrderCustomerManager(void);

	void Init(void);	//初期化処理
	void Update(void);	//更新処理
	void Draw(void);	//描画処理

	/// <summary>
	/// 生成された注文の内容に合わせてお客を生成する
	/// </summary>
	/// <param name=""></param>
	void CreateCustomersByOrders(void);

	/// <summary>
	/// お客と注文を追加で生成
	/// </summary>
	void AddOrdersAndCustomers(void);
	
	/// <summary>
	/// 先頭のお客と注文を削除する
	/// </summary>
	void ClearOrderAndCustomer(void);

	/// <summary>
	/// 追加で生成された注文の内容に合わせてお客を生成する
	/// </summary>
	void AddCustomerByOrder(void); 

	/// <summary>
	/// 提供されたものと注文を比較してスコアを返す
	/// </summary>
	/// <param name="serve">提供されたもの</param>
	/// <returns>スコア</returns>
	int GetOrderScore(const Order::OrderData serve);

	/// <summary>
	/// 
	/// </summary>
	/// <param name=""></param>
	void IsServe(void) { isServe_ = true; }

	/// <summary>
	/// 客が動いているかどうかを取得する
	/// </summary>
	/// <returns>動いている場合はtrue、そうでない場合はfalse</returns>
	bool GetIsMoving(void);

	Order::OrderData GetOrderData(void) const;

	void CheckServeAndOrder(const Order::OrderData serve);

private:

	//OrderManagerのポインタ
	std::unique_ptr<OrderManager> orderMng_;

	//CustomerMangerのポインタ
	std::shared_ptr<CustomerManager> customerMng_;

	bool isServe_;

	void DebugDraw(void);
};