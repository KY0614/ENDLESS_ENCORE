#pragma once
#include "../Object/Common/Transform.h"

class ItemPrevew
{
public:
	//コンストラクタ
	ItemPrevew(void);
	//デストラクタ
	~ItemPrevew(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void);

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

private:

	Transform transform_;

	int prevewScreen_;
};

