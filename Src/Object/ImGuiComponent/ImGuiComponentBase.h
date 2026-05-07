#pragma once
#include "../Character/Player.h"

class ImGuiComponentBase
{
public:
	//コンストラクタ
	ImGuiComponentBase(void);

	//デストラクタ
	virtual ~ImGuiComponentBase(void);

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update(void);

	virtual void SaveData();

protected:


};

