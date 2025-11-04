#pragma once
#include <memory>
#include "SceneBase.h"

class MovieScene : public SceneBase
{
public:

	// コンストラクタ
	MovieScene(void);

	// デストラクタ
	~MovieScene(void);

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

