#pragma once
#include "../SceneBase.h"

class InventoryScene : public SceneBase
{
public:
	//コンストラクタ
	InventoryScene(void);
	//デストラクタ
	~InventoryScene(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:
};

