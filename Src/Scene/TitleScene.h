#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class Stage;

class TitleScene : public SceneBase
{

public:
	static constexpr float HIGH_LIGHT_INTERVAL = 1.2f;

	//UVスケール
	static constexpr float TILLING_SIZE = 35.0f;

	static constexpr float AMBIENT_COLOR = 0.2f;

	static constexpr int LOGO_OFFSET_Y = 80;
	static constexpr int LOGO_HEIGHT = 1024;
	static constexpr int PUSHIMG_OFFSET_Y = 100;

	//コンストラクタ
	TitleScene(void);

	//デストラクタ
	~TitleScene(void);

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

	//ステージ
	std::shared_ptr<Stage> stage_;

	int logoImg_;

	//宣伝シーンへ遷移する用のタイマー
	int toAdvertiseLoopTimer_;

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);
};
