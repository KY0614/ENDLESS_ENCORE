#pragma once
#include "SceneBase.h"

class ClearScene : public SceneBase
{
public:
	//コンストラクタ
	ClearScene(void);
	//デストラクタ
	~ClearScene(void);

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

