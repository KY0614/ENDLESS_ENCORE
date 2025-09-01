#pragma once
#include <vector>
#include <memory>
#include "./CustomerBase.h"
#include "../Order/Order.h"

class OrderUI;
class GaugeUI;

class CustomerManager
{
public:

	//コンストラクタ
	CustomerManager(void);
	//デストラクタ
	~CustomerManager(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="orderTime">注文の経過時間</param>
	void Update(float orderTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	/// <summary>
	/// お客を一人生成する
	/// </summary>
	/// <param name="order">生成するタイプ</param>
	void CreateSingleCustomer(Order::OrderData data);

	/// <summary>
	/// 配列の先頭のお客情報を削除
	/// </summary>
	void ClearFirstCustomers(void);

	/// <summary>
	/// 客を動かす
	/// </summary>
	void StartMove(void) { isCustomersMove_ = true; }

	/// <summary>
	/// 客を止める
	/// </summary>
	/// <param name="">止めるためにfalseにする</param>
	void StopMove(void) { isCustomersMove_ = false; }

	/// <summary>
	/// 客が動いているか取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:動いている　false:止まってる</returns>
	bool GetIsMove(void)const { return isCustomersMove_; }

	/// <summary>
	/// スコアによるお客の反応の設定
	/// </summary>
	/// <param name="score">スコア</param>
	void SetCustomerReacton(int score);

	/// <summary>
	/// 最後尾（６人目）の座標を取得
	/// </summary>
	/// <param name="">５人目から間隔をあけて左にずらした座標を返す</param>
	/// <returns>最後尾の座標</returns>
	VECTOR GetLastCustomerPos(void) const;

	/// <summary>
	/// 先頭のお客がカウンター前にいるかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:いる　false:いない</returns>
	bool CheckFirstCustomerCol(void);

	/// <summary>
	/// 2人目のお客がカウンター前にいるかどうか
	/// </summary>
	/// <param name="">先頭のお客の注文提供後に確認する用</param>
	/// <returns>true:いる　false:いない</returns>
	bool CheckSecondCustomerCol(void);

	/// <summary>
	/// 注文の確認UIを表示する
	/// </summary>
	/// <param name="index">指定する配列</param>
	/// <param name="isActive">表示させるかどうか</param>
	void IsCheckUI(const int index,const bool isActive);

private:
	// 静的インスタンス
	static CustomerManager* instance_;
	
	//お客生成用
	std::vector<std::unique_ptr<CustomerBase>> customers_;

	//注文UI用
	std::vector<std::unique_ptr<OrderUI>> orderUI_;

	bool isCustomersMove_;
	int cnt_;


	/// <summary>
	/// 最初の5人の座標を初期化
	/// </summary>
	void InitCustomersPos(void);
};

