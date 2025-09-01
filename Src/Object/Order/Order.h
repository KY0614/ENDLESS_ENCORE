#pragma once

class Order
{
public:
	//注文関連
	static constexpr int ORDER_MAX_NUM = 2;			//注文の最大数
	static constexpr int ORDER_MIN_NUM = 1;			//注文の最小数

	static constexpr float ONE_ORDER_TIME = 25.0f;	//注文の制限時間(1つ)
	static constexpr float TWO_ORDER_TIME = 33.0f;	//注文の制限時間(２つ)

	//商品関連
	static constexpr int DRINK_MAX_NUM = 2;			//飲み物類の最大数
	static constexpr int FOOD_MAX_NUM = 2;			//食べ物類の最大数

	//ドリンクの種類
	enum class DRINK
	{
		NONE,	//無し
		HOT,	//ホット
		ICE,	//アイス
	};

	//スイーツの種類
	enum class SWEETS
	{
		NONE,		//無し
		CHOCO,		//チョコ
		STRAWBERRY,	//ストロベリー
	};

	//注文の内容
	struct OrderData
	{
		int num_;			//注文数
		DRINK drink_;		//飲み物
		bool lid_;			//飲み物に蓋があるかどうか
		SWEETS sweets_;		//スイーツ
		float time_;		//注文の制限時間
	};

	//コンストラクタ
	Order(void);

	//デストラクタ
	~Order(void);

	/// <summary>
	/// 制限時間の更新処理
	/// </summary>
	void TimerUpdate(void);

	/// <summary>
	/// 注文を生成
	/// </summary>
	/// <param name="">１つ生成</param>
	void CreateOrder(void);

	//取得関数--------------------------------------------------------------------

	/// <summary>
	/// 注文数を取得
	/// </summary>
	/// <param name="">注文している商品の数</param>
	/// <returns>注文数</returns>
	int GetOrderNum(void) const { return orderNum_; }	

	/// <summary>
	/// 注文内容を取得
	/// </summary>
	/// <param name="">注文内容を返す(OrderDataを返す)</param>
	/// <returns>注文内容</returns>
	OrderData GetOrder(void) const { return orderData_; }

	/// <summary>
	/// 注文の制限時間を取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	float GetOrderTime(void) const { return orderData_.time_; }	

private:
	//注文内容
	OrderData orderData_;

	//注文数
	int orderNum_;

	//設定関数-------------------------------------------------

	/// <summary>
	/// 注文数を設定する
	/// </summary>
	/// <param name="num"></param>
	void SetOrderNum(const int num) { orderData_.num_ = num; }

	/// <summary>
	/// 注文内容の飲み物を設定する
	/// </summary>
	/// <param name="type">設定するもの</param>
	void SetDrink(const DRINK type) { orderData_.drink_ = type; }

	/// <summary>
	/// 注文内容の食べ物を設定する
	/// </summary>
	/// <param name="type">設定するもの</param>
	void SetSweets(const SWEETS type) { orderData_.sweets_ = type; };

	/// <summary>
	/// 注文の制限時間を設定する
	/// </summary>
	/// <param name="time">設定する時間</param>
	void SetOrderTime(const float time) { orderData_.time_ = time; }
	//--------------------------------------------------------------
};

