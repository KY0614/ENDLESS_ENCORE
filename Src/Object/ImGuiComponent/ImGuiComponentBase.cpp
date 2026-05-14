#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/StringUtility.h"
#include "../Manager/Generic/JsonManager.h"
#include "ImGuiComponentBase.h"

ImGuiComponentBase::ImGuiComponentBase(void)
{
}

ImGuiComponentBase::~ImGuiComponentBase(void)
{
}

void ImGuiComponentBase::SliderFloatWithSave(
	const char* label,
	float* variable,
	const float min,
	const float max,
	const nlohmann::json& jsonData,
	const char* jsonKey)
{
	//ImGuiのfloat型のスライダー
	ImGui::SliderFloat(label, variable, min, max);

	//ポップアップのタイトル
	std::string popUpTitle = "Save " + std::string(label);
	//保存ボタンのラベル
	std::string saveLavel = StringUtility::Wstring2UTF8(L"保存##") + label;
	//保存ボタン(スライドの横に配置)
	ImGui::SameLine();
	if (ImGui::Button(saveLavel.c_str()))
	{
		ImGui::OpenPopup(popUpTitle.c_str());
	}
	//元に戻すボタン(保存ボタンの横に配置)
	ImGui::SameLine();
	std::string resetLabel = StringUtility::Wstring2UTF8(L"元に戻す##") + label;
	if (ImGui::Button(resetLabel.c_str()))
	{
		*variable = jsonData.value(jsonKey, 0.0f);
	}

	//JsonManagerのインスタンスを取得
	JsonManager& jsonM = JsonManager::GetInstance();
	//ポップアップの処理
	if (ImGui::BeginPopupModal(
		popUpTitle.c_str(),
		NULL,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(StringUtility::Wstring2UTF8(
			L"変更した内容を保存しますか？").c_str());
		ImGui::Text(StringUtility::Wstring2UTF8(L"変更内容：%.2ff →　%.2ff").c_str(),
			jsonData.value(jsonKey, 0.0f),
			*variable);
		const float buttonWidth = 120.0f; // ボタンの横幅
		const float windowWidth = ImGui::GetWindowSize().x; // 現在のウィンドウの横幅
		const float posX = (windowWidth - (buttonWidth * 2)) / 2.0f; // 中央位置を計算
		ImGui::SetCursorPosX(posX);// ボタンを中央に配置
		if (ImGui::Button("SAVE", ImVec2(buttonWidth, 0)))
		{
			//保存（データを上書き）
			jsonM.OverWriteJsonDatas(
				JsonManager::JSON_DATA::PLAYER, 
				*variable,
				"Player",
				"Parameter",
				jsonKey);

			//ポップアップを閉じる
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("CANCEL", ImVec2(buttonWidth, 0)))
		{
			//ポップアップを閉じる
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
