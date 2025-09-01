#pragma once
#include <vector>
#include "UIBase.h"
#include "../Order/Order.h"

class OrderUI :  public UIBase
{
public:
	static constexpr int ORDER_NUM = 2; //オーダーの数

	//背景UIのサイズ
    static constexpr float BACK_IMG_SIZE = 300.0f;
    static constexpr float IMG_SIZE = 100.0f;

    struct OrderUIData
    {
		Order::DRINK drinkType_;        //ドリンクの種類
		Order::SWEETS sweetsType_;  	//スイーツの種類

		int drinkUIImg_;    //ドリンクのUI画像
		int sweetsUIImg_;   //スイーツのUI画像
		int backUIImg_;     //背景のUI画像

		int timerBackImg_;  //タイマーの背景画像
		int timerImg_;      //タイマーの画像

		int checkImg_;     //チェックの画像

        float currentRate_ = 0.0f;	
        float displayedRate_ = 0.0f;
    };

    OrderUI(Order::DRINK drink,Order::SWEETS sweets,float maxTime);
    virtual ~OrderUI() = default;

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    /// <summary>
    /// 注文の残り時間を設定
    /// </summary>
    /// <param name="progressT">経過時間</param>
    void SetOrderTimer(const float progressT) { orderTimer_ = progressT; }

	/// <summary>
	/// チェックマークの状態を設定
	/// </summary>
	/// <param name="index">指定する配列</param>
	/// <param name="isCheck">true:表示　false:非表示</param>
	void SetCheckUI(const int index, const bool isActive) { isOrderCheck_[index] = isActive; }

private:

	//オーダーのUIデータ
    OrderUIData orderUIData_;

	//提供された商品がオーダーと一致しているかどうかのチェック
    std::vector<bool> isOrderCheck_;

    //ゲージを進ませる用の時間
    float gaugeTime_;

	//ゲージの最大時間
    float orderMaxTime_;
	//ゲージの現在の時間
    float orderTimer_;

	//注文内容画像のサイズ
    float size_;
};

