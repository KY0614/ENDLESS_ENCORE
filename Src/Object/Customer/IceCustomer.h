#pragma once
#include "../Customer/CustomerBase.h"


class IceCustomer : public CustomerBase
{
public:
	//コンストラクタ
	IceCustomer(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;
	
protected:
	/// <summary>
	/// パラメーターを設定する
	/// </summary>
	void SetParam(void) override;

	/// <summary>
	/// アニメーションの初期化処理
	/// </summary>
	void InitAnimation(void) override;
};

