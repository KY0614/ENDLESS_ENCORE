#include "../Libs/ImGui/imgui.h"
#include "../Common/DebugDrawFormat.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Object/Common/Sphere.h"
#include "../Object/Player.h"
#include "../Object/Stage/StageManager.h"
#include "../../UI/IconUI.h"
#include "../../UI/GaugeUI.h"
#include "../../UI/UIManager.h"
#include "Machine.h"

namespace {
	const std::string HOT_CUP = "Hot_Cup";	//ホット用カップ
	const std::string CUP_WITH_ICE = "Cup_With_Ice";		//アイス用カップ
	const std::string HOT_COFFEE = "Hot_Coffee";		//ホットコーヒー
	const std::string ICE_COFFEE = "Ice_Coffee";		//アイスコーヒー
}

Machine::Machine(const std::string objId, const float width,
	const float height, const float depth,Player& player,
	std::vector<std::unique_ptr<StageObject>>& object) :
	StageObject(objId, width, height, depth,player),objects_(object)
{
}

void Machine::Interact(const std::string& objId)
{
	if (GetMachineState() == MACHINE_STATE::ACTIVE)return;	//アクティブ状態だったら処理しない

	auto& items = param_.acceptedItems_;
	//objIdがインタラクト対象物に存在するかどうか
	bool isAccepted = std::find(items.begin(), items.end(), objId) != items.end();
	if (!isAccepted)return;	//存在しなかったら処理しない

	//既にマシン内にカップまたはコーヒーがある場合は何もしない
	for (const auto& obj : objects_)
	{
		// ホットカップまたはホットコーヒー
		if ((obj->GetParam().id_ == HOT_CUP ||
			obj->GetParam().id_ == HOT_COFFEE ||
			obj->GetParam().id_ == CUP_WITH_ICE ||
			obj->GetParam().id_ == ICE_COFFEE) &&
			AsoUtility::IsHitSpheres(GetSpherePos(), GetSphereRad(),
				obj->GetSpherePos(), obj->GetSphereRad()) &&
			obj->GetItemState() == ITEM_STATE::PLACED)
		{
			// 既にセット済み
			return;
		}
	}

	auto& ins = InputManager::GetInstance();
	for (const auto& obj : objects_)
	{
		//カップ以外のオブジェクトは判定しない
		if (obj->GetParam().id_ != HOT_CUP && obj->GetParam().id_ != CUP_WITH_ICE) continue;

		if (AsoUtility::IsHitSpheres(GetSpherePos(), GetSphereRad(),
			obj->GetSpherePos(), obj->GetSphereRad()))
		{
			iconUI_->SetActive(true);
			//スペースキー押下でマシンの場所にカップを置く
			if ((player_.GetHoldItem() == items.front().c_str() ||
				player_.GetHoldItem() == items.back().c_str()) &&
				ins.IsInputTriggered("Interact"))
			{
				//マシンの上に乗るようにカップを配置する
				VECTOR cupPos = GetTransform().pos;	
				cupPos = VAdd(cupPos, { 0.0f,MACHINE_OFSET_Y ,MACHINE_OFSET_Z });

				//マシンの回転に合わせてカップの位置を調整
				VECTOR rotPos = AsoUtility::RotXZPos(GetTransform().pos, cupPos,
					Quaternion::ToEuler(GetTransform().quaRotLocal).y);

				obj->ItemPlaced(rotPos);
				gaugeUI_->SetActive(true);
				ChangeMachineState(MACHINE_STATE::ACTIVE);
			}
		}
	}
}

void Machine::Init(VECTOR pos, float rotY, VECTOR scale)
{
	StageObject::Init(pos, rotY, scale);

	iconUI_ = std::make_unique<IconUI>(VGet(0.0f, UI_OFFSET_Y, 0.0f),
		transform_.pos, ResourceManager::SRC::BREW_COFFEE);
	iconUI_->Init();
	iconUI_->SetActive(false);
	UIManager::GetInstance().AddIconUI(iconUI_.get());

	gaugeUI_ = std::make_unique<GaugeUI>(false, COFFEE_PRODUCES_TIME);
	gaugeUI_->Init();
	VECTOR uiPos = transform_.pos;
	uiPos.y += UI_OFFSET_Y;	//UIの位置を調整
	gaugeUI_->SetPos(uiPos); // UIの位置を設定1
	UIManager::GetInstance().AddGaugeUI(gaugeUI_.get());
}

void Machine::UpdateInActive(void)
{
	iconUI_->SetActive(false);
	SetInteractTime(COFFEE_PRODUCES_TIME);

	//マシンの当たり判定内にPLACED状態のカップが存在するかチェック
	bool hasPlacedCup = false;
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != HOT_COFFEE &&
			obj->GetParam().id_ != ICE_COFFEE)continue;

		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			GetSpherePos(), GetSphereRad()) &&
			obj->GetItemState() == ITEM_STATE::PLACED)
		{
			hasPlacedCup = true;
			break;
		}
	}

	if(!hasPlacedCup)
	{
		gaugeUI_->Reset();
	}
}

void Machine::UpdateActive(void)
{
	iconUI_->SetActive(false);
	param_.interactTime_ -= SceneManager::GetInstance().GetDeltaTime();

	gaugeUI_->Update();

	//マシンの当たり判定内にPLACED状態のカップが存在するかチェック
	bool hasPlacedCup = false;
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != HOT_CUP &&
			obj->GetParam().id_ != CUP_WITH_ICE )continue;

		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			GetSpherePos(), GetSphereRad()) &&
			obj->GetItemState() == ITEM_STATE::PLACED)
		{
			hasPlacedCup = true;
			break;
		}
	}

	//PLACED状態のカップがなければ非アクティブにする
	if (!hasPlacedCup || param_.interactTime_ <= 0.0f)
	{
		ChangeMachineState(MACHINE_STATE::INACTIVE);
		return;
	}
}

void Machine::Draw(void)
{
	StageObject::Draw();
}
