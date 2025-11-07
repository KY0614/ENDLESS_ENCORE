#pragma once
#include "../SceneBase.h"

class ExplainScene : public SceneBase
{
public:

	// コンストラクタ
	ExplainScene(void);

	// デストラクタ
	~ExplainScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

	// 初期化処理
	void Init(void)override;

	// 更新ステップ
	void Update(void)override;

	// 描画処理
	void Draw(void)override;

private:

	int backImg_;
	int explainImg_;
	int menuBackImg_;
};

