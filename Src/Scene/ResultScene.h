#pragma once
#include <memory>
#include "SceneBase.h"

class Score;

class ResultScene : public SceneBase
{
public:

	// コンストラクタ
	ResultScene(void);

	// デストラクタ
	~ResultScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

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

