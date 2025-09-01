#pragma once
#include <Windows.h>

class ImGuiWrapper
{

public:

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static ImGuiWrapper& GetInstance(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Destroy(void);

private:

	// 静的インスタンス
	static ImGuiWrapper* instance_;

	// ウインドウプロージャ
    static LRESULT CALLBACK WndProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ImGuiWrapper(void);
	// コピーコンストラクタも同様
	ImGuiWrapper(const ImGuiWrapper&);
	// デストラクタも同様
	~ImGuiWrapper(void);

	void UpdateInputMouse(void);
	void UpdateNewFrame(void);

};

