#pragma once
#include <memory>
#include "SceneBase.h"
class Player;
class Enemy;

class DebugScene : public SceneBase
{
public:
	//コンストラクタ
	DebugScene(void);
	//デストラクタ
	~DebugScene(void);

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

	//床
	Transform floor_;

	std::unique_ptr<Player> player_;
	std::unique_ptr<Enemy> enemy_;
};

