#include "../../../Libs/ImGui/imgui.h"
#include "../../../Manager/Generic/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Renderer/ModelMaterial.h"
#include "../../../Renderer/ModelRenderer.h"
#include "Furnitures.h"

Furnitures::Furnitures(void)
{
}

Furnitures::~Furnitures(void)
{
}

void Furnitures::Init(void)
{
	//モデルの基本設定
	SetupFurniture(floor_, ResourceManager::SRC::FLOOR, AsoUtility::VECTOR_ZERO);
	floor_.MakeCollider(Collider::TYPE::STAGE);

	//床用のマテリアル初期化
	InitMaterial();
	
	SetupFurniture(deskL_, ResourceManager::SRC::DESK, DESK_L_POS);
	SetupFurniture(deskR_, ResourceManager::SRC::DESK, DESK_R_POS);
	SetupFurniture(sidePlant_, ResourceManager::SRC::PLANT, SIDE_PLANT_POS);
	SetupFurniture(frontPlant_, ResourceManager::SRC::PLANT, FRONT_PLANT_POS);
	SetupFurniture(sideShelves_, ResourceManager::SRC::SHELVES, SIDE_SHELVES_POS);
	SetupFurniture(frontLShelves_, ResourceManager::SRC::SHELVES_FRONT,
		{ FRONT_SHELVES_R_POS_X,FRONT_SHELVES_POS.y,FRONT_SHELVES_POS.z });
	SetupFurniture(frontRShelves_, ResourceManager::SRC::SHELVES,
		FRONT_SHELVES_POS, Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f }));
}

void Furnitures::Update(void)
{
	//動かす予定特にないので何も処理しない
}

void Furnitures::Draw(void)
{
	renderer_->Draw();
	MV1DrawModel(deskL_.modelId);
	MV1DrawModel(deskR_.modelId);
	MV1DrawModel(sidePlant_.modelId);
	MV1DrawModel(frontPlant_.modelId);
	MV1DrawModel(sideShelves_.modelId);
	MV1DrawModel(frontLShelves_.modelId);
	MV1DrawModel(frontRShelves_.modelId);
}

void Furnitures::SetupFurniture(Transform& transform,
	ResourceManager::SRC modelSrc, const VECTOR& pos, const Quaternion& quaRotLocal)
{
	transform.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(modelSrc));
	transform.scl = AsoUtility::VECTOR_ONE;
	transform.pos = pos;
	transform.quaRot = Quaternion();
	transform.quaRotLocal = quaRotLocal;
	transform.Update();
}

void Furnitures::InitMaterial(void)
{
	//モデル描画用
	material_ = std::make_unique<ModelMaterial>(
		"StdModelVS.cso", 1,
		"StdModelPS.cso", 3
	);
	//タイリングするのでテクスチャアドレスをWRAPに
	material_->SetTextureAddress(ModelMaterial::TEXADDRESS::WRAP);

	//uvに渡すスケール値
	material_->AddConstBufVS({ TILLING_SIZE ,TILLING_SIZE,TILLING_SIZE,TILLING_SIZE });

	//色の影響度
	material_->AddConstBufPS({ AsoUtility::VECTOR_ONE.x,AsoUtility::VECTOR_ONE.y,AsoUtility::VECTOR_ONE.z,1.0f });

	//ライトの方向
	VECTOR light = GetLightDirection();
	material_->AddConstBufPS({ light.x,light.y,light.z,1.0f });

	//環境光
	material_->AddConstBufPS({ AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR,AMBIENT_COLOR });

	renderer_ = std::make_unique<ModelRenderer>(floor_.modelId, *material_);

}