#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Common/Sphere.h"
#include "../Object/Player.h"
#include "../Object/Stage/StageManager.h"
#include "Table.h"

Table::Table(const std::string objId, const float width,
	const float height, const float depth, Player& player,
	std::vector<std::unique_ptr<StageObject>>& objects) :
	StageObject(objId, width, height, depth, player),objects_(objects)
{
}

void Table::Update(void)
{
	StageObject::Update();
	colTran_.Update();
	//テーブルの上にオブジェクトが置かれていないかどうか
	for (const auto& obj : objects_)
	{
		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			GetSpherePos(), GetSphereRad()) && obj->GetItemState() == StageObject::ITEM_STATE::PLACED)
		{
			//置かれている場合は、置けない状態にする
			param_.placeable_ = false;
			break;
		}
		else if(obj->GetItemState() != StageObject::ITEM_STATE::PLACED)
		{
			param_.placeable_ = true;
		}
	}
}