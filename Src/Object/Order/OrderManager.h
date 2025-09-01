#pragma once
#include<vector>
#include<memory>
#include"../../Object/Order/Order.h"

class Order;

class OrderManager
{
public:
	//注文関連
	static constexpr int MAX_CREATE_NUM = 6;	//最大注文生成数

	//コンストラクタ
	OrderManager(void);
	//デストラクタ
	~OrderManager(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void);

	/// <summary>
	/// 先頭の注文を更新する
	/// </summary>
	/// <param name=""></param>
	void FirstOrderUpdate(void);
	
	//生成関数-----------------------------------------------------

	/// <summary>
	/// 注文を追加生成
	/// </summary>
	/// <param name="">注文数が最大数未満になったら追加生成</param>
	void AddCreateOrder(void);

	//--------------------------------------------------------------
	//削除関数------------------------------------------------------

	/// <summary>
	/// 先頭の注文を削除する
	/// </summary>
	/// <param name=""></param>
	void ClearFirstOrder(void);
	//---------------------------------------------------------------
	//取得関数-------------------------------------------------------

	/// <summary>
	/// 最初の注文の制限時間が過ぎたかどうかを返す
	/// </summary>
	/// <returns>true:過ぎた　false:過ぎてない </returns>
	bool IsFirstOrderTimeOut(void);

	/// <summary>
	/// 先頭の注文内容を取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Order::OrderData GetFirstOrder(void) { return orders_.front()->GetOrder(); }

	/// <summary>
	/// すべての注文データを取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns>注文内容データ</returns>
	std::vector<Order::OrderData> GetAllOrder(void) const;

	/// <summary>
	/// 最後尾の注文データを取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns>最後尾の注文データ</returns>
	Order::OrderData GetLastOrderData(void) const;
	//----------------------------------------------------------------
private:

	//注文管理用
	std::vector<std::unique_ptr<Order>> orders_;

	//生成関数-----------------------------------------------------

	/// <summary>
	/// 注文を最大数まで生成する
	/// </summary>
	/// <param name="">ゲームの始まりに生成する用</param>
	void CreateOrderMaxNum(void);

	/// <summary>
	/// １つだけ注文を生成する
	/// </summary>
	/// <param name="">Orderクラスを１つ生成し、配列に格納</param>
	void CreateSingleOrder(void);

};

