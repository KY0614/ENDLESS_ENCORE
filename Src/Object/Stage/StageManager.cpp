#include <DxLib.h>
#include "../../Application.h"
#include "../Utility/AsoUtility.h"
#include "../../Manager/GameSystem/SoundManager.h"
#include "../../Manager/Generic/ResourceManager.h"
#include "../../Manager/Generic/InputManager.h"
#include "../Player.h"
#include "../Common/Sphere.h"
#include "../Common/AnimationController.h"
#include "StageObject/Furnitures.h"
#include "StageObject.h"
#include "StageObject/ItemObject.h"
#include "StageObject/RackObject.h"
#include "StageObject/FollowingObject.h"
#include "StageObject/Table.h"
#include "StageObject/Machine.h"
#include "StageObject/CupLidRack.h"
#include "StageObject/DustBox.h"
#include "StageObject/IceDispenser.h"
#include "StageManager.h"

namespace {
	const std::string TABLE = "Table";		//テーブル
	const std::string COUNTER = "Counter";	//カウンター
	const std::string HOT_CUP = "Hot_Cup";	//ホット用カップ
	const std::string ICE_CUP = "Ice_Cup";	//アイス用カップ
	const std::string HOT_CUP_RACK = "Cup_Hot_Rack";		//ホット用ラック
	const std::string ICE_CUP_RACK = "Cup_Ice_Rack";		//アイス用ラック
	const std::string CHOCO_SWEETSRACK = "Sweets_Choco_Rack";		//チョコスイーツ用ラック
	const std::string BERRY_SWEETSRACK = "Sweets_Strawberry_Rack";	//ベリースイーツ用ラック
	const std::string CHOCO_SWEETS = "Sweets_Choco";		//チョコスイーツ
	const std::string BERRY_SWEETS = "Sweets_Strawberry";	//ベリースイーツ
	const std::string CUP_WITH_ICE = "Cup_With_Ice";		//アイス用カップ
	const std::string HOT_COFFEE = "Hot_Coffee";			//ホットコーヒー
	const std::string ICE_COFFEE = "Ice_Coffee";			//アイスコーヒー
	const std::string COFFEE_MACHINE = "Coffee_Machine";	//コーヒーマシン
	const std::string CUP_LID_RACK = "Cup_Lid_Rack";		//蓋のラック
	const std::string HOTCUP_LID = "Hot_Cup_Lid";					//蓋（ホット）
	const std::string ICECUP_LID = "Ice_Cup_Lid";					//蓋
	const std::string ICE_DISPENSER = "Ice_Dispenser";		//製氷機
	const std::string DUST_BOX = "Dust_Box";				//ゴミ箱
}


StageManager::StageManager(Player& player):player_(player)
{
	servedItems_ = {};
	currentOrder_ = {};

	objects_.clear();
	tables_.clear();
	counter_ = nullptr;

	isServed_ = false;
	isServedItems_.clear();

	animationController_ = nullptr;
}

StageManager::~StageManager(void)
{
	isServedItems_.clear();
}

void StageManager::Init(void)
{
	auto& sound = SoundManager::GetInstance();

	//se追加
	//アイテムを取り出す時のSE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PICK_UP,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PICK_UP).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::PICK_UP, 256);	
	//アイテムを置く時のSE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PUT_ON,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PUT_ON).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::PUT_ON, 256);
	//ストック追加時SE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::ADD_STOCK,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::ADD_STOCK).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::ADD_STOCK, 256);
	//提供追加時SE
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::PAYING,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::PAYING).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::ADD_STOCK, 256 / 2);

	//3Dモデルの初期化
	Init3DModel();
}

void StageManager::Update(void)
{
	animationController_->Update();

	if (animationController_->IsEnd())
	{
		//アニメーションが終わったらマシンモードに切り替え
		animationController_->Play((int)ANIM_TYPE::IDLE);
	}

	auto& pSphere = player_.GetSphere();

	for (const auto& obj : objects_)
	{
		obj->Update();
	}

	for (const auto& obj : tables_)
	{
		obj->Update();
	}

	counter_->Update();
	furnitures_->Update();

	//ラックからカップを取り出す処理
	for (const auto& obj : objects_)
	{
		//ラックに在庫がないときの処理
		if (!player_.GetIsHolding() && obj->GetParam().interactable_ &&
			!obj->GetHasStock() &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				obj->GetSpherePos(), obj->GetSphereRad()))
		{
			if (obj->GetParam().id_ == HOT_CUP_RACK ||
				obj->GetParam().id_ == ICE_CUP_RACK)
			{
				obj->AddStock(StageObject::CUP_STOCK_MAX);
			}
			else if (obj->GetParam().id_ == BERRY_SWEETSRACK ||
				obj->GetParam().id_ == CHOCO_SWEETSRACK)
			{
				obj->AddStock(StageObject::SWEETS_STOCK_MAX);
			}
			break;
		}

		//プレイヤーが何も持っていないときの処理
		if (!player_.GetIsHolding() && obj->GetParam().interactable_ &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				obj->GetSpherePos(), obj->GetSphereRad()))
		{
			obj->PickUp(obj->GetParam().id_, objects_);
			break;
		}
	}

	//持ち運び可能なオブジェクトのインタラクト処理
	CarryableObjInteract();

	//マシンとのインタラクト処理
	MachineInteract();

	//蓋ラックとのインタラクト処理
	LidRackInteract();

	DustBoxInteract();

	transform_.Update();
	caseTran_.Update();
}

void StageManager::Draw(void)
{
	MV1DrawModel(transform_.modelId);
	MV1DrawModel(caseTran_.modelId);

	furnitures_->Draw();

	for (const auto& table : tables_)
	{
		table->Draw();
	}

	counter_->Draw();

	for (const auto& obj : objects_)
	{
		obj->Draw();
	}
}

void StageManager::SetCurrentOrder(const Order::OrderData& order)
{
	//注文数文の配列を確保し、初期化＆お客の注文内容を受け取る
	size_t size = static_cast<size_t>(order.num_);
	isServedItems_.resize(size);
	for(bool isSuved : isServedItems_)
	{
		isSuved = false; // 初期化
	}
	currentOrder_ = order;
}

void StageManager::ResetServeData(void)
{
	//提供データをリセット
	isServed_ = false;
	servedItems_.drink_ = Order::DRINK::NONE;
	servedItems_.sweets_ = Order::SWEETS::NONE;
	servedItems_.lid_ = false;
	isServedItems_.clear();
	isServedItems_.resize(0);
}

Transform StageManager::GetCounterTran(void) const
{
	return counter_->GetTransform(); 
}

Transform StageManager::GetTableTran(int index) const
{
	if (index > tables_.size())return Transform(); // 範囲外のインデックスは無視
	return tables_[index]->GetTalbeColTran();
}

Transform StageManager::GetShowCase(void) const
{
	return caseTran_;
}

Transform StageManager::GetDustBox(void) const
{
	return dustBoxTran_;
}

Transform StageManager::GetFloorTran(void) const
{
	return furnitures_->GetFloorTran();
}

void StageManager::Init3DModel(void)
{
	//モデル制御の基本情報
	caseTran_.SetModel(
		ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWEETS_CASE));
	caseTran_.scl = AsoUtility::VECTOR_ONE;
	caseTran_.pos = CASE_POS;
	caseTran_.quaRot = Quaternion::Euler(
		0.0f, 0.0f, 0.0f);

	caseTran_.quaRotLocal = Quaternion();
	caseTran_.MakeCollider(Collider::TYPE::STAGE);
	caseTran_.Update();

	//モデル制御の基本情報
	transform_.SetModel(
		ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::REGISTER));
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.pos = AsoUtility::VECTOR_ZERO;
	transform_.quaRot = Quaternion::Euler(AsoUtility::VECTOR_ZERO);
	transform_.quaRotLocal = Quaternion();
	transform_.MakeCollider(Collider::TYPE::STAGE);
	transform_.Update();

	InitAnimation(); // アニメーションの初期化

	//家具の初期化
	furnitures_ = std::make_unique<Furnitures>();
	furnitures_->Init();

	VECTOR firstPos = {};
	//横のテーブル群(手前)
	for (int x = 0; x < TABLE_ROW_BACK_NUM; x++)
	{
		firstPos = TABLE_POS_BACK;
		firstPos.x += (x * TABLE_WIDTH);
		tables_.emplace_back(std::make_unique<Table>(TABLE, TABLE_WIDTH, 76.0f, 60.0f, player_, objects_));
		tables_.back()->Init(firstPos, 180.0f);
	}

	//縦のテーブル群(左側）
	for (int z = TABLE_ROW_BACK_NUM; z < TABLE_ROW_BACK_NUM + TABLE_COLUMN_NUM; z++)
	{
		firstPos = COLUMN_TABLE_LEFT_POS;
		firstPos.z += ((z - TABLE_ROW_BACK_NUM) * TABLE_WIDTH);
		tables_.emplace_back(std::make_unique<Table>(TABLE, 60.0f, 76.0f, TABLE_WIDTH, player_, objects_));
		tables_.back()->Init(firstPos, -90.0f);
	}

	//縦のテーブル群(右側）
	for (int z = TABLE_ROW_BACK_NUM; z < TABLE_ROW_BACK_NUM + TABLE_COLUMN_NUM; z++)
	{
		firstPos = COLUMN_TABLE_RIGHT_POS;
		firstPos.z += ((z - TABLE_ROW_BACK_NUM) * TABLE_WIDTH);
		tables_.emplace_back(std::make_unique<Table>(TABLE, 60.0f, 76.0f, TABLE_WIDTH, player_, objects_));
		tables_.back()->Init(firstPos, 90.0f);
	}

	//真ん中のテーブル群
	for (int i = 0; i < TABLE_CENTER_NUM / 2; ++i)
	{
		firstPos = CENTER_TABLE_POS;
		firstPos.x += i * 90.0f;
		//奥側のテーブル２つ
		tables_.emplace_back(std::make_unique<Table>(TABLE, TABLE_WIDTH, 76.0f, 60.0f, player_, objects_));
		tables_.back()->Init(firstPos, 0.0f);
	}
	for (int i = 0; i < TABLE_CENTER_NUM / 2; ++i)
	{
		firstPos = CENTER_TABLE_POS;
		firstPos.x += i * 90.0f;
		firstPos.z += 60.0f;
		//手前側のテーブル２つ
		tables_.emplace_back(std::make_unique<Table>(TABLE, TABLE_WIDTH, 76.0f, 60.0f, player_, objects_));
		tables_.back()->Init(firstPos, 180.0f);
	}

	//ケースのテーブル群(奥側)
	for (int x = 0; x < TABLE_ROW_FRONT_NUM; x++)
	{
		firstPos = TABLE_POS_FRONT;
		firstPos.x += (x * (TABLE_WIDTH + 20.0f));
		tables_.emplace_back(std::make_unique<Table>(TABLE, TABLE_WIDTH + 20.0f, 76.0f, 60.0f, player_, objects_));
		tables_.back()->Init(firstPos);
	}

	//カウンター用テーブル
	counter_ = std::make_unique<Table>(COUNTER, TABLE_WIDTH, 76.0f, 60.0f, player_, objects_);
	counter_->Init(COUNTER_POS);

	VECTOR pos = tables_[TABLE_ROW_BACK_NUM + TABLE_COLUMN_NUM - 1]->GetTopCenter();
	//ホット用カップのラック
	objects_.emplace_back(std::make_unique<RackObject>(HOT_CUP_RACK, 60.0f, 20.0f, 60.0f, player_));
	objects_.back()->Init(pos, -90.0f);

	//アイス用カップのラック
	pos = tables_[TABLE_ROW_BACK_NUM + TABLE_COLUMN_NUM + TABLE_COLUMN_NUM - 1]->GetTopCenter();
	objects_.emplace_back(std::make_unique<RackObject>(ICE_CUP_RACK, 60.0f, 20.0f, 60.0f, player_));
	objects_.back()->Init(pos, 90.0f);

	//チョコスイーツ用のラック
	pos = tables_[tables_.size() - 2]->GetTopCenter();
	objects_.emplace_back(std::make_unique<RackObject>(CHOCO_SWEETSRACK, 60.0f, 20.0f, 60.0f, player_));
	objects_.back()->Init(pos);

	//ベリースイーツ用のラック
	pos = tables_.back()->GetTopCenter();
	objects_.emplace_back(std::make_unique<RackObject>(BERRY_SWEETSRACK, 60.0f, 20.0f, 60.0f, player_));
	objects_.back()->Init(pos);

	//カップ用の蓋
	pos = tables_[2]->GetTopCenter();
	objects_.emplace_back(std::make_unique<CupLidRack>(CUP_LID_RACK, 60.0f, 20.0f, 60.0f, player_, objects_));
	objects_.back()->Init(pos);

	//コーヒーマシン
	pos = tables_[MAX_TABLE_NUM - 2]->GetTopCenter();
	objects_.emplace_back(std::make_unique<Machine>(COFFEE_MACHINE, 50.0f, 60.0f, 75.0f,
		player_, objects_));
	objects_.back()->Init(pos, 90.0f);
	//コーヒーマシン２個目
	pos = tables_[MAX_TABLE_NUM - 4]->GetTopCenter();
	objects_.emplace_back(std::make_unique<Machine>(COFFEE_MACHINE, 50.0f, 60.0f, 75.0f,
		player_, objects_));
	objects_.back()->Init(pos, 90.0f);

	//アイスディスペンサー
	pos = tables_[MAX_TABLE_NUM - 1]->GetTopCenter();
	objects_.emplace_back(std::make_unique<IceDispenser>(ICE_DISPENSER, 50.0f, 75.0f, 60.0f,
		player_, objects_));
	objects_.back()->Init(pos, -90.0f);
	//アイスディスペンサー２個目
	pos = tables_[MAX_TABLE_NUM - 3]->GetTopCenter();
	objects_.emplace_back(std::make_unique<IceDispenser>(ICE_DISPENSER, 50.0f, 75.0f, 60.0f,
		player_, objects_));
	objects_.back()->Init(pos, -90.0f);

	//ゴミ箱
	objects_.emplace_back(std::make_unique<DustBox>(DUST_BOX, 50.0f, 75.0f, 60.0f,
		player_, objects_));
	objects_.back()->Init(DUSTBOX_POS, -180.0f, { 1.0f,0.8f,1.0f });
	dustBoxTran_ = objects_.back()->GetTransform(); // ゴミ箱のTransformを保存

}

void StageManager::InitAnimation(void)
{
	std::string path = Application::PATH_MODEL + "Stage/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::CREATE, path + "create_register.mv1", 30.0f);

	animationController_->Add((int)ANIM_TYPE::PAYING, path + "paying.mv1", 30.0f);
	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void StageManager::SurveItem(StageObject& obj)
{
	// 商品情報をOrderDataに変換
	Order::OrderData data;
	// 例: objのIDや状態からOrderDataをセット
	if (obj.GetParam().id_ == HOT_COFFEE)
	{
		servedItems_.drink_ = Order::DRINK::HOT;
		servedItems_.lid_ = obj.IsLidOn();
		isServedItems_.front() = true;
	}
	else if (obj.GetParam().id_ == ICE_COFFEE)
	{
		servedItems_.drink_ = Order::DRINK::ICE;
		servedItems_.lid_ = obj.IsLidOn();
		isServedItems_.front() = true;
	}

	if (obj.GetParam().id_ == CHOCO_SWEETS)
	{
		servedItems_.sweets_ = Order::SWEETS::CHOCO;
		isServedItems_.back() = true;
	}
	else if (obj.GetParam().id_ == BERRY_SWEETS)
	{
		servedItems_.sweets_ = Order::SWEETS::STRAWBERRY;
		isServedItems_.back() = true;
	}
	//提供後はオブジェクトを削除
	DeleteSurvedItem(); 

	auto& sound = SoundManager::GetInstance();
	// 注文が揃ったか判定
	if (IsOrderCompleted())
	{
		isServed_ = true;
		animationController_->Play((int)ANIM_TYPE::PAYING, false);
		sound.Play(SoundManager::SOUND::PAYING);
	}
}

void StageManager::DeleteSurvedItem(void)
{
	//カウンターの球体と当たっているオブジェクトを削除
	for (auto it = objects_.begin(); it != objects_.end(); )
	{
		if (AsoUtility::IsHitSpheres(
			(*it)->GetSpherePos(), (*it)->GetSphereRad(),
			counter_->GetSpherePos(), counter_->GetSphereRad()))
		{
			StageObject* target = it->get(); //今から削除する親オブジェクトを記録
			if (target->IsLidOn())
			{
				//追従している蓋をすべて削除
				for (int i = 0; i < objects_.size(); )
				{
					FollowingObject* follower = dynamic_cast<FollowingObject*>(objects_[i].get());
					if (follower && &(follower->GetFollowedObj()) == target)
					{
						objects_.erase(objects_.begin() + i);
						continue;
					}
					++i;
				}
			}
			it = objects_.erase(it);

		}
		else
		{
			++it;
		}
	}
}

void StageManager::CarryableObjInteract(void)
{
	auto& pSphere = player_.GetSphere();
	for (const auto& obj : objects_)
	{
		//カウンターで商品を提供する処理
		if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
			counter_->GetSpherePos(), counter_->GetSphereRad()
		))
		{
			//設置されているアイテムを提供する処理
			if (obj->GetItemState() == StageObject::ITEM_STATE::PLACED)
			{
				SurveItem(*obj);
				break;
			}
		}

		//プレイヤーが何も持っていないときの処理
		if (!player_.GetIsHolding() && obj->GetParam().carryable_ &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				obj->GetSpherePos(), obj->GetSphereRad()))
		{
			obj->ItemCarry();
			break;
		}

		//プレイヤーがアイテムを持っているときの処理
		if (player_.GetIsHolding())
		{
			//カウンターに設置
			if (AsoUtility::IsHitSpheres(obj->GetSpherePos(), obj->GetSphereRad(),
				counter_->GetSpherePos(), counter_->GetSphereRad()
			))
			{
				auto items = counter_->GetParam().acceptedItems_;
				//objIdがインタラクト対象物に存在するかどうか
				bool isAccepted = std::find(items.begin(), items.end(), obj->GetParam().id_) != items.end();
				if (!isAccepted)continue;	//存在しなかったら処理しない

				obj->ItemPlaced(counter_->GetTopCenter());
			}

			for (const auto& table : tables_)
			{
				//設置可能なテーブルの上にアイテムを設置する処理
				if (table->GetParam().placeable_ &&
					AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
						table->GetSpherePos(), table->GetSphereRad()
					))
				{
					obj->ItemPlaced(table->GetTopCenter());
				}

				if (obj->IsActioned())
				{
					break;
				}
			}
		}
		if (obj->IsActioned())
		{
			break;
		}
	}
}

void StageManager::MachineInteract(void)
{
	auto& pSphere = player_.GetSphere();

	//マシンとカップの処理
	for (size_t i = 0; i < objects_.size(); ++i)
	{
		//コーヒーマシンの判定だけさせたい
		if (objects_[i]->GetParam().id_ != COFFEE_MACHINE)continue;
		//持っているアイテムをマシンに設置する処理
		if (player_.GetIsHolding() &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				objects_[i]->GetSpherePos(), objects_[i]->GetSphereRad()))
		{
			objects_[i]->Interact(player_.GetHoldItem());
		}

		//設置して一定時間経ったら氷入りカップを出力する
		if (objects_[i]->GetParam().interactTime_ <= 0.0f)
		{
			ProduceCoffee(static_cast<int>(i));
			break;
		}
	}
	//for (const auto& obj : objects_)
	//{
	//	if (obj->GetParam().id_ != COFFEE_MACHINE)continue;

	//	//持っているアイテムをマシンに設置する処理
	//	if (player_.GetIsHolding() &&
	//		AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
	//			obj->GetSpherePos(), obj->GetSphereRad()))
	//	{
 //			obj->Interact(player_.GetHoldItem());
	//	}

	//	//設置して一定時間経ったらコーヒーを出力する
	//	if (obj->GetParam().id_ == COFFEE_MACHINE &&
	//		obj->GetParam().interactTime_ <= 0.0f)
	//	{
	//		ProduceCoffee();
	//		break;
	//	}
	//}

	//ディスペンサーとカップの処理
	for (size_t i = 0; i < objects_.size(); ++i)
	{
		//ディスペンサーの判定だけさせたい
		if (objects_[i]->GetParam().id_ != ICE_DISPENSER)continue;
		//持っているアイテムをマシンに設置する処理
		if (player_.GetIsHolding() &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				objects_[i]->GetSpherePos(), objects_[i]->GetSphereRad()))
		{
			objects_[i]->Interact(player_.GetHoldItem());
		}

		//設置して一定時間経ったら氷入りカップを出力する
		if (objects_[i]->GetParam().interactTime_ <= 0.0f)
		{
			DispenseIce2Cup(i);
			break;
		}
	}
}

void StageManager::LidRackInteract(void)
{
	auto& pSphere = player_.GetSphere();
	//コーヒーと蓋の処理
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != CUP_LID_RACK)continue;

		//持っているコーヒーに蓋をつける処理
		if (player_.GetIsHolding() &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				obj->GetSpherePos(), obj->GetSphereRad()))
		{
			obj->Interact(player_.GetHoldItem());
		}
		else
		{
			//判定外の場合は初期値に戻す
			obj->IsNotActioned();
			obj->SetInteractTime(3.0f);
		}

		//インタラクトし続けて一定時間経ったら蓋をする
		if (obj->GetParam().interactTime_ <= 0.0f)
		{
			LidFollowCup();
			break;
		}
	}
}

void StageManager::ProduceCoffee(int index)
{
	//コーヒーマシン以外のオブジェクトは判定しない
	if (objects_[index]->GetParam().id_ != COFFEE_MACHINE) return;
	const auto& machine = objects_[index];
	for (size_t i = 0; i < objects_.size(); ++i)
	{
		//ホット用カップ以外のオブジェクトは判定しない
		if (objects_[i]->GetParam().id_ != HOT_CUP &&
			objects_[i]->GetParam().id_ != CUP_WITH_ICE) continue;

		//マシンの判定だけさせたい
		if (machine->GetParam().id_ != COFFEE_MACHINE)continue;

		//マシンの球体と設置されているカップだけ処理する
		if (objects_[i]->GetItemState() == StageObject::ITEM_STATE::PLACED &&
			AsoUtility::IsHitSpheres(machine->GetPos(), machine->GetSphereRad(),
				objects_[i]->GetSpherePos(), objects_[i]->GetSphereRad()))
		{
			if (objects_[i]->GetParam().id_ == HOT_CUP)
			{
				MakeCoffee(i, *machine, HOT_COFFEE);
				return; // ホットコーヒーを作ったら処理を終了
			}
			else {
				MakeCoffee(i, *machine, ICE_COFFEE);
				return; // アイスコーヒーを作ったら処理を終了
			}
		}

	}
}

void StageManager::MakeCoffee(int index, StageObject& obj, std::string objName)
{
	//アイスコーヒーを生成する場合は氷を先に削除しておく
	if (objName == ICE_COFFEE)
	{
		//氷入りカップの場合は氷も削除
		ItemObject* cupWithIce = dynamic_cast<ItemObject*>(objects_[index].get());
		if (cupWithIce->IsIce())
		{
			//蓋のインデックスを探す
			for (int i = 0; i < objects_.size(); ++i)
			{
				//dynamic_castでFollowingObject型に変換し、親参照を比較
				//蓋を削除する
				FollowingObject* follower = dynamic_cast<FollowingObject*>(objects_[i].get());
				if (follower && &(follower->GetFollowedObj()) == objects_[index].get())
				{
					objects_.erase(objects_.begin() + i);
					continue; // eraseしたのでiは進めず次のループへ
				}
			}
		}
	}

	//設置されているカップをコーヒーに上書きする
	objects_[index] = std::make_unique<ItemObject>(objName, 20.0f, 30.0f, 20.0f, player_);
	//マシンの上に乗るようにカップを配置する
	VECTOR cupPos = obj.GetTransform().pos;
	cupPos = VAdd(cupPos,
		{ 0.0f,StageObject::MACHINE_OFSET_Y ,StageObject::MACHINE_OFSET_Z });

	//マシンの回転に合わせてカップの位置を調整
	VECTOR rotPos = AsoUtility::RotXZPos(obj.GetTransform().pos, cupPos,
		Quaternion::ToEuler(obj.GetTransform().quaRotLocal).y);

	objects_[index]->Init(rotPos);
	objects_[index]->ChangeItemState(StageObject::ITEM_STATE::PLACED);
}

void StageManager::DispenseIce2Cup(int index)
{
	//ディスペンサー以外のオブジェクトは判定しない
	if (objects_[index]->GetParam().id_ != ICE_DISPENSER) return;
	const auto& dispenser = objects_[index];
	for (size_t i = 0; i < objects_.size(); ++i)
	{
		//ディスペンサーの判定だけさせたい
		if (objects_[index]->GetParam().id_ != ICE_DISPENSER)continue;

		//マシンの球体と設置されているカップだけ処理する
		if (objects_[i]->GetItemState() == StageObject::ITEM_STATE::PLACED &&
			AsoUtility::IsHitSpheres(
				dispenser->GetPos(), dispenser->GetSphereRad(),
				objects_[i]->GetSpherePos(), objects_[i]->GetSphereRad()))
		{
			//設置されているカップに氷を入れる
			if (auto iceCup = dynamic_cast<ItemObject*>(objects_[i].get()))
			{
				iceCup->PouredIce();

				//氷を生成＆追従させる
				objects_.emplace_back(std::make_unique<FollowingObject>("Ice", 23.0f, 5.0f, 23.0f, player_, *objects_[i]));
				objects_.back()->Init(AsoUtility::VECTOR_ZERO);
				objects_.back()->Update();
				break;
			}
		}
	}
}

void StageManager::LidFollowCup(void)
{
	bool isCreate = false;
	for (size_t i = 0; i < objects_.size(); ++i)
	{
		//コーヒー以外のオブジェクトは判定しない
		if (objects_[i]->GetParam().id_ != HOT_COFFEE &&
			objects_[i]->GetParam().id_ != ICE_COFFEE) continue;

		//既に蓋が乗っているカップは判定しない
		if (objects_[i]->IsLidOn())continue;

		for (const auto& lid : objects_)
		{
			//蓋の判定だけさせたい
			if (lid->GetParam().id_ != CUP_LID_RACK)continue;

			//蓋の球体と当たっているカップだけ処理する
			if (AsoUtility::IsHitSpheres(lid->GetPos(), lid->GetSphereRad(),
				objects_[i]->GetSpherePos(), objects_[i]->GetSphereRad()))
			{
				//コーヒーオブジェクトに蓋をする
				objects_[i]->PutOnTheLid();
				std::string lidType = "";
				if(objects_[i]->GetParam().id_ == HOT_COFFEE)lidType = HOTCUP_LID;
				else if (objects_[i]->GetParam().id_ == ICE_COFFEE)lidType = ICECUP_LID;
				//蓋を生成＆追従させる
				objects_.emplace_back(std::make_unique<FollowingObject>(lidType, 23.0f, 5.0f, 23.0f, player_, *objects_[i]));
				objects_.back()->Init(AsoUtility::VECTOR_ZERO);
				objects_.back()->Update();
				isCreate = true;
				break;
			}
		}
		if (isCreate)break;
	}
}

void StageManager::DustBoxInteract(void)
{
	auto& ins = InputManager::GetInstance();
	auto& pSphere = player_.GetSphere();

	// ゴミ箱の処理
	for (const auto& obj : objects_)
	{
		if (obj->GetParam().id_ != DUST_BOX) continue;

		// プレイヤーが持っているアイテムをゴミ箱に近づけている場合
		if (player_.GetIsHolding() &&
			AsoUtility::IsHitSpheres(pSphere.GetPos(), pSphere.GetRadius(),
				obj->GetSpherePos(), obj->GetSphereRad()))
		{
			if(ins.IsInputTriggered("Interact"))
			{
				//ゴミ箱にアイテムを捨てる処理
				obj->Interact(player_.GetHoldItem());
			}
			break;
		}
	}
}

bool StageManager::IsOrderCompleted(void)
{
	for(bool isSurved : isServedItems_)
	{
		if (!isSurved)
		{
			return false; // 1つでも未提供のアイテムがあればfalse
			break;
		}
	}
	return true;
}