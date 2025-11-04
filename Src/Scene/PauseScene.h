#pragma once
#include "SceneBase.h"

class PauseScene : public SceneBase
{
public:

	static constexpr int APPEAR_INTERVAL = 15;

	static constexpr int MARGINE_SIZE = 30;

	PauseScene(void);
	~PauseScene(void);

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

	//メニューの文字列リスト
	std::vector<std::wstring> menuList_;
	using MenuFunc_t = std::function<void()>;
	std::map<std::wstring, MenuFunc_t> menuFuncTable_;

	//メニューの描画関数リスト
	using MenuDraw_t = std::function<void()>;
	std::map<std::wstring, MenuDraw_t> menuDrawTable_;

	//ポーズ画面のシーン
	std::list<std::unique_ptr<SceneBase>> scenes_;

	//関数ポインタ
	using UpdateFunc_t = void(PauseScene::*)();
	using DrawFunc_t = void(PauseScene::*)();

	UpdateFunc_t update_;
	DrawFunc_t draw_;

	int backImg_;
	int* menuListImg_;
	int menuCursorImg_;

	int frame_;

	int cursorIdx_;

	void UpdateAppear(void);
	void UpdateDisappear(void);
	void DrawProcess(void);

	void UpdateNormal(void);
	void DrawNormal(void);

	void DrawMenuList(void);
};

