#pragma once
#include <memory>
#include "SceneBase.h"

class PlayerTest;
class Stage;

class DebugScene : public SceneBase
{
public:
	DebugScene(void);
	~DebugScene(void);

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

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name=""></param>
	void UpdateImGui(void) override;
private:
	//ステージ
	std::unique_ptr<PlayerTest> playerTest_;
	std::unique_ptr<Stage> stage_;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void ObjectUpdateImGui(void);
};

