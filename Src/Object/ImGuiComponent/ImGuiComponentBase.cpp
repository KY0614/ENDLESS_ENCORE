#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/StringUtility.h"
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
	//スライドの値とJsonデータの値が異なる場合、元に戻すボタンを表示
	float subValue = jsonData.value(jsonKey, 0.0f) - *variable;
	if (subValue > 0.0f || subValue < 0.0f)
	{
		//元に戻すボタン(保存ボタンの横に配置)
		ImGui::SameLine();
		std::string resetLabel = StringUtility::Wstring2UTF8(L"元に戻す##") + label;
		if (ImGui::Button(resetLabel.c_str()))
		{
			*variable = jsonData.value(jsonKey, 0.0f);
		}
	}
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
		const float buttonWidth = 120.0f; //ボタンの横幅
		const float windowWidth = ImGui::GetWindowSize().x;				//現在のウィンドウの横幅
		const float posX = (windowWidth - (buttonWidth * 2)) / 2.0f;	//中央位置を計算
		ImGui::SetCursorPosX(posX);//ボタンを中央に配置
		if (ImGui::Button("SAVE", ImVec2(buttonWidth, 0)))
		{
			//保存（データを上書き）
			SaveJsonData(variable,jsonKey);

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

void ImGuiComponentBase::SetTargetHierarchy(const std::vector<const char*>& hierarchyKeys)
{
	//中身が入っていても上書き
	hierarchyKeys_ = hierarchyKeys;
}
