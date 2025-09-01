#include "../Common/DebugDrawFormat.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Object/Player.h"
#include "../../UI/GaugeUI.h"
#include "../../UI/UIManager.h"
#include "CupLidRack.h"

namespace {
	const std::string HOT_COFFEE = "Hot_Coffee";	//ホットコーヒー
	const std::string ICE_COFFEE = "Ice_Coffee";	//アイスコーヒー
}

CupLidRack::CupLidRack(const std::string objId, const float width,
	const float height, const float depth, Player& player,
	std::vector<std::unique_ptr<StageObject>>& object) :
	StageObject(objId, width, height, depth, player), objects_(object)
{
}

void CupLidRack::Interact(const std::string& objId)
{
	auto& items = param_.acceptedItems_;
	//objIdがインタラクト対象物に存在するかどうか
	bool isAccepted = std::find(items.begin(), items.end(), objId) != items.end();
	if (!isAccepted)return;	//存在しなかったら処理しない
	if (param_.interactTime_ <= 0.0f)return;

	auto& ins = InputManager::GetInstance();
	for (const auto& obj : objects_)
	{
		//コーヒー以外のオブジェクトは判定しない
		if (obj->GetParam().id_ != HOT_COFFEE && 
			obj->GetParam().id_ != ICE_COFFEE) continue;
		//既に蓋されているコーヒーは判定しない
		if (obj->IsLidOn())continue;
		
		bool isHoldingHotCoffee = obj->GetItemState()==ITEM_STATE::HOLD;

		if (AsoUtility::IsHitSpheres(GetSpherePos(), GetSphereRad(),
			obj->GetSpherePos(), obj->GetSphereRad()))
		{
			//スペースキーを押下し続けるとゲージがたまっていく
			if (isHoldingHotCoffee &&
				ins.IsInputPressed("Interact"))
			{
				param_.interactTime_ -= SceneManager::GetInstance().GetDeltaTime();
				isActioned_ = true;
				player_.ChangeState(Player::STATE::STOP);
 				gaugeUI_->SetActive(true);
			}
			else
			{
				SetInteractTime(LID_PRODUCES_TIME);
				isActioned_ = false;
				player_.ChangeState(Player::STATE::PLAY);
				gaugeUI_->Reset();
			}
		}
	}
}

void CupLidRack::Init(VECTOR pos, float rotY, VECTOR scale)
{
	StageObject::Init(pos,rotY, scale);

	gaugeUI_ = std::make_unique<GaugeUI>(false, LID_PRODUCES_TIME);
	gaugeUI_->Init();
	VECTOR uiPos = transform_.pos;
	uiPos.y += 50.0f;	//UIの位置を調整
	gaugeUI_->SetPos(uiPos); // UIの位置を設定
	UIManager::GetInstance().AddGaugeUI(gaugeUI_.get());
}

void CupLidRack::Update(void)
{
	if (!isActioned_)
	{
		SetInteractTime(LID_PRODUCES_TIME);
		player_.ChangeState(Player::STATE::PLAY);
	}
		gaugeUI_->Update();
	
	if (param_.interactTime_ <= 0.0f)
	{
		isActioned_ = false;
		gaugeUI_->Reset();
	}

	transform_.Update();
}

void CupLidRack::Draw(void)
{

	StageObject::Draw();

}
