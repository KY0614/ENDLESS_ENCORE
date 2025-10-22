#pragma once
#include <memory>
#include "SceneBase.h"

class Player;
class Enemy;

class GameScene : public SceneBase
{
public:
	//コンストラクタ
	GameScene(void);

	//デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	//関数ポインタ（カウントダウン、ゲーム中、タイムアップ）
	using UpdateFunc_t = void(GameScene::*)();
	using DrawFunc_t = void(GameScene::*)();

	UpdateFunc_t update_;
	DrawFunc_t draw_;

	std::unique_ptr<Player> player_;
	std::unique_ptr<Enemy> enemy_;

	int RT_;

	float shakeFrame_;
	float shakeRate_;

	/// <summary>
	/// ゲーム中の更新処理
	/// </summary>
	/// <param name="">ゲーム中の処理</param>
	void UpdateGame(void);

	/// <summary>
	/// ゲーム中の描画
	/// </summary>
	/// <param name="">カウントダウン、カウントアップ以外の描画</param>
	void DrawGame(void);

#ifdef _DEBUG

	//床
	Transform floor_;
	
	/// <summary>
	/// デバッグ用の描画処理
	/// </summary>
	void DrawDebug(void);

#endif // _DEBUG
};
