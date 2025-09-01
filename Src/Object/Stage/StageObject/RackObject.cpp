#include "../../../Manager/GameSystem/SoundManager.h"
#include "../../../Manager/Generic/SceneManager.h"
#include "../../../Manager/Generic/InputManager.h"
#include "../../../Manager/Generic/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Common/Sphere.h"
#include "../Object/Player.h"
#include "../../UI/GaugeUI.h"
#include "../../UI/IconUI.h"
#include "../../UI/UIManager.h"
#include "ItemObject.h"
#include "RackObject.h"

RackObject::RackObject(const std::string objId,
	const float width, const float height,
	const float depth, Player& player) :
	StageObject(objId, width, height, depth, player)
{
	sweetsStockCnt_ = SWEETS_STOCK_MAX;
	cupsStockCnt_ = CUP_STOCK_MAX;
	hasStock_ = true;
	addInterval_ = 0.0f;
}

void RackObject::PickUp(std::string rackName,std::vector<std::unique_ptr<StageObject>>& object)
{
	if (!hasStock_)return;
	isInteract_ = true;
	auto& ins = InputManager::GetInstance();
	auto& sound = SoundManager::GetInstance();
	//ホットカップ用ラックにインタラクトしたときの処理
	if (rackName == "Cup_Hot_Rack" && ins.IsInputTriggered("Interact"))
	{
		sound.Play(SoundManager::SOUND::PICK_UP);
		object.emplace_back(std::make_unique<ItemObject>("Hot_Cup", 40.0f, 30.0f, 40.0f, player_));
		object.back()->Init(player_.GetSphere().GetPos());
		player_.SetHoldItem(object.back()->GetParam().id_);
		object.back()->ChangeItemState(ITEM_STATE::HOLD);
		isActioned_ = true;
		cupsStockCnt_--;
	}

	//アイスカップ用ラックにインタラクトしたときの処理
	if (rackName == "Cup_Ice_Rack" && ins.IsInputTriggered("Interact"))
	{
		sound.Play(SoundManager::SOUND::PICK_UP);
		object.emplace_back(std::make_unique<ItemObject>("Ice_Cup", 40.0f, 30.0f, 40.0f, player_));
		object.back()->Init(player_.GetSphere().GetPos());
		player_.SetHoldItem(object.back()->GetParam().id_);
		object.back()->ChangeItemState(ITEM_STATE::HOLD);
		isActioned_ = true;
		cupsStockCnt_--;
	}

	//スイーツ（ベリー）用ラックにインタラクトしたときの処理
	if (rackName == "Sweets_Strawberry_Rack" && ins.IsInputTriggered("Interact"))
	{
		sound.Play(SoundManager::SOUND::PICK_UP);
		//スイーツを取り出す
		object.emplace_back(std::make_unique<ItemObject>("Sweets_Strawberry", 40.0f, 30.0f, 40.0f, player_));
		object.back()->Init(player_.GetSphere().GetPos());
		object.back()->SetScale({ 1.2f,1.2f,1.2f }); // スイーツのサイズを調整
		player_.SetHoldItem(object.back()->GetParam().id_);
		object.back()->ChangeItemState(ITEM_STATE::HOLD);
		isActioned_ = true;
		sweetsStockCnt_--;
	}

	//スイーツ（チョコ）用ラックにインタラクトしたときの処理
	if (rackName == "Sweets_Choco_Rack" && ins.IsInputTriggered("Interact"))
	{
		sound.Play(SoundManager::SOUND::PICK_UP);
		//スイーツを取り出す
		object.emplace_back(std::make_unique<ItemObject>("Sweets_Choco", 40.0f, 30.0f, 40.0f, player_));
		object.back()->Init(player_.GetSphere().GetPos());
		object.back()->SetScale({ 1.2f,1.2f,1.2f }); // スイーツのサイズを調整
		player_.SetHoldItem(object.back()->GetParam().id_);
		object.back()->ChangeItemState(ITEM_STATE::HOLD);
		isActioned_ = true;
		sweetsStockCnt_--;
	}
}

void RackObject::AddStock(int addStockNum)
{
	if (hasStock_) return;
	
	auto& ins = InputManager::GetInstance();
	auto& sound = SoundManager::GetInstance();
	if (ins.IsInputPressed("Interact"))
	{
		gaugeUI_->SetActive(true);
		addInterval_ += SceneManager::GetInstance().GetDeltaTime();
		gaugeUI_->Update();
	}

	if(GetParam().id_ == "Sweets_Strawberry_Rack" ||
		GetParam().id_ == "Sweets_Choco_Rack")
	{
		//スイーツの在庫を追加
		if (addInterval_ > ADD_INTERVAL)
		{
			addInterval_ = 0.0f;
			sweetsStockCnt_++;
			sound.Play(SoundManager::SOUND::ADD_STOCK);
		}
		//スイーツの在庫が最大に達したら、在庫ありに設定
		if (sweetsStockCnt_ >= SWEETS_STOCK_MAX)
		{
			hasStock_ = true;
			gaugeUI_->Reset();
			return;
		}
	}
	else if(GetParam().id_ == "Cup_Hot_Rack" ||
			GetParam().id_ == "Cup_Ice_Rack")
	{
		//カップの在庫を追加
		if (addInterval_ > ADD_INTERVAL)
		{
			addInterval_ = 0.0f;
			cupsStockCnt_++;
			sound.Play(SoundManager::SOUND::ADD_STOCK);
		}
		//カップの在庫が最大に達したら、在庫ありに設定
		if (cupsStockCnt_ >= CUP_STOCK_MAX)
		{
			hasStock_ = true;
			gaugeUI_->Reset();
			return;
		}
	}
}

void RackObject::Init(VECTOR pos, float rotY, VECTOR scale)
{
	StageObject::Init(pos, rotY, scale);

	//スイーツ用UIの初期化
	iconUI_ = std::make_unique<IconUI>(VGet(0.0f, height_ + 60.0f, 0.0f),
		transform_.pos, ResourceManager::SRC::INTERACT);
	iconUI_->SetActive(false);
	iconUI_->Init();
	iconUI_->SetUISize(70.0f);
	UIManager::GetInstance().AddIconUI(iconUI_.get());
	//文字列をSRCに変換してモデル設定
	ResourceManager::SRC srcType = ResourceManager::SRC::NONE;
	if (param_.id_ == "Sweets_Strawberry_Rack")
	{
		srcType = ResourceManager::SRC::SWEETS_BERRY; // デフォルトのラックIDを設定
		cupsStockCnt_ = 0;
	}
	else if (param_.id_ == "Sweets_Choco_Rack")
	{
		srcType = ResourceManager::SRC::SWEETS_CHOCO; // チョコレートラックIDを設定
		cupsStockCnt_ = 0;
	}

	if (srcType != ResourceManager::SRC::NONE)
	{
		//スイーツ用UIの初期化
		gaugeUI_ = std::make_unique<GaugeUI>(false, ADD_INTERVAL * SWEETS_STOCK_MAX);
		gaugeUI_->Init();
		gaugeUI_->SetUISize(50.0f, 5.0f);
		VECTOR uiPos = transform_.pos;
		uiPos.y += SWEETS_UI_OFFSET_Y;	//UIの位置を調整
		gaugeUI_->SetPos(uiPos); // UIの位置を設定
		UIManager::GetInstance().AddGaugeUI(gaugeUI_.get());
		// 各スイーツの基準座標からのオフセットを配列で定義
		const VECTOR sweetsOffsets[] = {
			{SWEETS_HALF_WIDTH,  SWEETS_HEIGHT_OFFSET, SWEETS_Z_BACK_OFFSET},
			{-SWEETS_HALF_WIDTH,  SWEETS_HEIGHT_OFFSET, SWEETS_Z_BACK_OFFSET},
			{-SWEETS_HALF_WIDTH,  SWEETS_HEIGHT_OFFSET,  SWEETS_Z_FRONT_OFFSET},
			{SWEETS_HALF_WIDTH,  SWEETS_HEIGHT_OFFSET,  SWEETS_Z_FRONT_OFFSET}
		};

		for (int i = 0; i < sweetsStockCnt_; ++i)
		{
			//モデルの基本設定
			sweetsOfRack_[i].SetModel(ResourceManager::GetInstance().LoadModelDuplicate(srcType));
			sweetsOfRack_[i].scl = AsoUtility::VECTOR_ONE;
			VECTOR sweetsPos = VAdd(pos, sweetsOffsets[i]);
			VECTOR rotPos = AsoUtility::RotXZPos(transform_.pos, sweetsPos, AsoUtility::Deg2RadF(rotY));
			sweetsOfRack_[i].pos = rotPos;
			sweetsOfRack_[i].quaRot = Quaternion();
			sweetsOfRack_[i].quaRotLocal =
				Quaternion::Euler({ AsoUtility::Deg2RadF(SWEETS_ROT_X),AsoUtility::Deg2RadF(rotY), 0.0f });
			sweetsOfRack_[i].Update();
		}
	}

	if (srcType != ResourceManager::SRC::NONE)return;

	//カップ用UIの初期化
	gaugeUI_ = std::make_unique<GaugeUI>(false, ADD_INTERVAL * CUP_STOCK_MAX);
	gaugeUI_->Init();
	gaugeUI_->SetUISize(70.0f, 10.0f);
	VECTOR uiPos = transform_.pos;
	uiPos.y -= CUP_UI_OFFSET_Y;	//UIの位置を調整
	gaugeUI_->SetPos(uiPos); // UIの位置を設定
	UIManager::GetInstance().AddGaugeUI(gaugeUI_.get());
	//設定されていなかったらカップモデルを設定する
	if (param_.id_ == "Cup_Hot_Rack")
	{
		srcType = ResourceManager::SRC::HOTCUP; // ホットカップラックIDを設定
		sweetsStockCnt_ = 0;
	}
	else if (param_.id_ == "Cup_Ice_Rack")
	{
		srcType = ResourceManager::SRC::ICECUP; // アイスカップラックIDを設定
		sweetsStockCnt_ = 0;
	}

	// 各カップの基準座標からのオフセットを配列で定義
	float cupHeightOffset = 8.0f;
	const VECTOR cupOffsets[] = {
		{CUPS_HALF_WIDTH,  CUPS_HEIGHT_OFFSET,  CUPS_Z_OFFSET},
		{-CUPS_HALF_WIDTH, CUPS_HEIGHT_OFFSET,  CUPS_Z_OFFSET},
		//2段目のカップ
		{CUPS_HALF_WIDTH,  CUPS_HEIGHT_OFFSET + cupHeightOffset,  CUPS_Z_OFFSET},
		{-CUPS_HALF_WIDTH, CUPS_HEIGHT_OFFSET + cupHeightOffset,  CUPS_Z_OFFSET},
		//3段目のカップ								倍ずらす
		{CUPS_HALF_WIDTH,  CUPS_HEIGHT_OFFSET + (cupHeightOffset * 2.0f),  CUPS_Z_OFFSET}
	};

	for (int i = 0; i < cupsStockCnt_; ++i)
	{
		//モデルの基本設定
		cupesOfRack_[i].SetModel(ResourceManager::GetInstance().LoadModelDuplicate(srcType));
		cupesOfRack_[i].scl = AsoUtility::VECTOR_ONE;
		VECTOR cupsPos = VAdd(pos, cupOffsets[i]);
		VECTOR rotPos = AsoUtility::RotXZPos(transform_.pos, cupsPos, AsoUtility::Deg2RadF(rotY));
		cupesOfRack_[i].pos = rotPos;
		cupesOfRack_[i].quaRot = Quaternion();
		cupesOfRack_[i].quaRotLocal =
			Quaternion::Euler({ AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(rotY), 0.0f });
		cupesOfRack_[i].Update();
	}
}

void RackObject::Update(void)
{
	if (isInteract_)iconUI_->SetActive(true);
	else iconUI_->SetActive(false);
	if(sweetsStockCnt_ <= 0 && cupsStockCnt_ <= 0)
	{
		hasStock_ = false;
	}

	for(auto& sweets : sweetsOfRack_)
	{
		sweets.Update();
	}

	for(auto& cups : cupesOfRack_)
	{
		cups.Update();
	}

	StageObject::Update();
}

void RackObject::Draw(void)
{
	StageObject::Draw();

	for (int i = 0; i < cupsStockCnt_; ++i)
	{
		//モデルの描画
		MV1DrawModel(cupesOfRack_[i].modelId);
	}

	for (int i = 0; i < sweetsStockCnt_; ++i)
	{
		//モデルの描画
		MV1DrawModel(sweetsOfRack_[i].modelId);
	}

}
