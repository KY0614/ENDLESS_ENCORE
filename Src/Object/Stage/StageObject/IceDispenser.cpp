#include "../Utility/AsoUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Object/Common/Sphere.h"
#include "../Object/Player.h"
#include "../Object/Stage/StageManager.h"
#include "../../UI/GaugeUI.h"
#include "../../UI/IconUI.h"
#include "../../UI/UIManager.h"
#include "IceDispenser.h"

namespace {
	const std::string ICE_CUP = "Ice_Cup";	//アイス用カップ
	const std::string CUP_WITH_ICE = "Cup_With_Ice";		//アイス用カップ
}

IceDispenser::IceDispenser(const std::string objId, const float width,
	const float height, const float depth, Player& player,
	std::vector<std::unique_ptr<StageObject>>& object) :
	StageObject(objId, width, height, depth, player), objects_(object)
{
}

void IceDispenser::Init(VECTOR pos, float rotY, VECTOR scale)
{
	StageObject::Init(pos, rotY,scale);

	iconUI_ = std::make_unique<IconUI>(VGet(0.0f, 160.0f, 0.0f),
		transform_.pos, ResourceManager::SRC::ICE_IN);
	iconUI_->Init();
	iconUI_->SetActive(false);
	UIManager::GetInstance().AddIconUI(iconUI_.get());

	gaugeUI_ = std::make_unique<GaugeUI>(false, ICE_PRODUCES_TIME);
	gaugeUI_->Init();
	VECTOR uiPos = transform_.pos;
	uiPos.y += 160.0f;	//UIの位置を調整
	gaugeUI_->SetPos(uiPos); // UIの位置を設定
	UIManager::GetInstance().AddGaugeUI(gaugeUI_.get());
}

void IceDispenser::Draw(void)
{
	StageObject::Draw();
}

void IceDispenser::Interact(const std::string& objId)
{
	if (GetMachineState() == MACHINE_STATE::ACTIVE)return;	//アクティブ状態だったら処理しない

	auto& items = param_.acceptedItems_;
	//objIdがインタラクト対象物に存在するかどうか
	bool isAccepted = std::find(items.begin(), items.end(), objId) != items.end();
	if (!isAccepted)return;	//存在しなかったら処理しない

	//既にマシン内にカップまたはコーヒーがある場合は何もしない
	for (const auto& obj : objects_)
	{
		//アイスカップまたはアイスコーヒー
		if ((obj->GetParam().id_ == ICE_CUP || obj->GetParam().id_ == CUP_WITH_ICE) &&
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
		//アイス用カップ以外のオブジェクトは判定しない
		if (obj->GetParam().id_ != ICE_CUP) continue;

		if (AsoUtility::IsHitSpheres(GetSpherePos(), GetSphereRad(),
			obj->GetSpherePos(), obj->GetSphereRad()))
		{
			iconUI_->SetActive(true);
			//スペースキー押下でマシンの場所にカップを置く(とりあえず)
			if (player_.GetHoldItem() == items.front().c_str() &&
				ins.IsInputTriggered("Interact"))
			{
				//マシンの上に乗るようにカップを配置する
				VECTOR cupPos = GetTransform().pos;
				cupPos = VAdd(cupPos, { 0.0f,MACHINE_OFSET_Y ,DISPENSER_OFSET_Z });

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

void IceDispenser::UpdateInActive(void)
{
	SetInteractTime(ICE_PRODUCES_TIME);
	iconUI_->SetActive(false);
	//マシンの当たり判定内にPLACED状態のカップが存在するかチェック
	bool hasPlacedCup = false;
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != CUP_WITH_ICE)continue;

		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			GetSpherePos(), GetSphereRad()) &&
			obj->GetItemState() == ITEM_STATE::PLACED)
		{
			hasPlacedCup = true;
			break;
		}
	}

	if (!hasPlacedCup)
	{
		gaugeUI_->Reset();
	}
}

void IceDispenser::UpdateActive(void)
{
	param_.interactTime_ -= SceneManager::GetInstance().GetDeltaTime();
	iconUI_->SetActive(false);
	gaugeUI_->Update();

	//マシンの当たり判定内にPLACED状態のカップが存在するかチェック
	bool hasPlacedCup = false;
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != ICE_CUP)continue;

		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			GetSpherePos(), GetSphereRad()) &&
			obj->GetItemState() == ITEM_STATE::PLACED &&
			param_.interactTime_ >= 0.0f)
		{
			hasPlacedCup = true;
			break;
		}
	}

	//PLACED状態のカップがなければ非アクティブにする
	if (!hasPlacedCup)
	{
		ChangeMachineState(MACHINE_STATE::INACTIVE);
		return;
	}
}
