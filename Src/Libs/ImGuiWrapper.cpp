#include <DxLib.h>
#include "../Manager/Generic/InputManager.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGuiWrapper.h"

ImGuiWrapper* ImGuiWrapper::instance_ = nullptr;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ImGuiWrapper::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ImGuiWrapper();
	}
	instance_->Init();
}

ImGuiWrapper& ImGuiWrapper::GetInstance(void)
{
	return *instance_;
}

void ImGuiWrapper::Init(void)
{

	// ウインドウへのメッセージ(イベント)を
	// フックするウインドウプロージャを登録する
	DxLib::SetHookWinProc(WndProc);
	
	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
	ImGui_ImplDX11_Init(
		(ID3D11Device*)DxLib::GetUseDirect3D11Device(),
		(ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext());

}

void ImGuiWrapper::Update(void)
{
	// マウス入力制御
	UpdateInputMouse();
	// ImGui更新準備
	UpdateNewFrame();
}

void ImGuiWrapper::Draw(void)
{
	// ImGui描画
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// 他の描画処理から、DxLib描画に戻る時に必要な処理
	DxLib::RefreshDxLibDirect3DSetting();
}

void ImGuiWrapper::Destroy(void)
{
	// ImGuiの解放
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete instance_;
}

LRESULT ImGuiWrapper::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	// このウインドウプロージャの戻り値を使用しない
	DxLib::SetUseHookWinProcReturnValue(false);

	// ImGuiにウィンドウイベントを渡す
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		//g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		//g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);

}

ImGuiWrapper::~ImGuiWrapper(void)
{
}

void ImGuiWrapper::UpdateInputMouse(void)
{

	// マウス情報をImGuiに渡す(InputManager使用)
	ImGuiIO& io = ImGui::GetIO();
	auto& input = InputManager::GetInstance();
	auto mousePos = input.GetMousePos();
	io.AddMousePosEvent(mousePos.x, mousePos.y);
	io.AddMouseButtonEvent(ImGuiMouseButton_Left, input.IsClickMouseLeft());
	io.AddMouseButtonEvent(ImGuiMouseButton_Right, input.IsClickMouseRight());

	// マウス情報をImGuiに渡す(InputManager未使用、DxLib使用)
	//ImGuiIO& io = ImGui::GetIO();
	//auto mouseInput = DxLib::GetMouseInput();
	//int mousePosX = 0;
	//int mousePosY = 0;
	//DxLib::GetMousePoint(&mousePosX, &mousePosY);
	//io.AddMousePosEvent(mousePosX, mousePosY);
	//io.AddMouseButtonEvent(ImGuiMouseButton_Left, mouseInput & MOUSE_INPUT_LEFT);
	//io.AddMouseButtonEvent(ImGuiMouseButton_Right, mouseInput & MOUSE_INPUT_RIGHT);

}

void ImGuiWrapper::UpdateNewFrame(void)
{

	// ImGui操作前準備
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

ImGuiWrapper::ImGuiWrapper(void)
{
}

