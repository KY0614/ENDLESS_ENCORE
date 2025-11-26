#include "../Libs/ImGui/imgui.h"
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "Common/Geometry/Cube.h"
#include "Common/Geometry/Box.h"
#include "Stage.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_STAGE = "Stage";
	static const std::string KEY_THEATER = "Theater";
	static const std::string KEY_MIST_WALL = "MistWall";

	const float ALPHA_LINE_MAX = 1.1f;
	const float ALPHA_LINE_MIN = -0.1f;
	const float ALPHA_RANGE = 0.01f;
}

Stage::Stage(void)
{
	noiseTextureId_ = -1;
	mistScrollSpeed_ = 0.0f;
	dissolveAlphaLine_ = ALPHA_LINE_MAX;
	isBattle_ = false;
}

Stage::~Stage(void)
{
}

void Stage::Init(VECTOR pos, VECTOR sPos)
{
	//3Dモデル初期化
	Init3DModel();

	InitMaterial(pos, sPos);

	cube_ = std::make_unique<Box>(transform_);
	//cube_->SetLocalCenter(VGet(0.0f, -150.0f, 3000.0f));
	cube_->SetLocalCenter(VGet(0.0f, 0.0f, 0.0f));
	cube_->SetSize(VGet(2700.0f/2.0f, 500.0f / 2.0f, 500.0f));
}

void Stage::Update(void)
{
	//霧のスクロール速度更新
	mistScrollSpeed_ += SceneManager::GetInstance().GetDeltaTime();
	//マテリアルの定数バッファ更新
	UpdateStageMaterialConstBuf();	//ステージ
	UpdateMistWallMaterialConstBuf();	//霧の壁

	InputManager& ins = InputManager::GetInstance();
	static bool isDissolve_ = true;
	if (ins.IsTrgDown(KEY_INPUT_Q))
	{
		isDissolve_ = !isDissolve_;
	}
	if (isDissolve_)
	{
		dissolveAlphaLine_ += SceneManager::GetInstance().GetDeltaTime() * 0.5f;
		if (dissolveAlphaLine_ >= ALPHA_LINE_MAX)
		{
			dissolveAlphaLine_ = ALPHA_LINE_MAX;
		}
	}
	else {
		dissolveAlphaLine_ -= SceneManager::GetInstance().GetDeltaTime() * 0.5f;
		if (dissolveAlphaLine_ <= ALPHA_LINE_MIN)
		{
			dissolveAlphaLine_ = ALPHA_LINE_MIN;
		}
	}

	transform_.Update();
	mistWallTransform_.Update();

	UpdateDebugImGui();
}

void Stage::Draw(void)
{
	stageRenderer_->Draw();

	if (!isBattle_)return;
	mistWallRenderer_->Draw();
	//cube_->Draw();
}

void Stage::UpdateDebugImGui(void)
{

	//ウィンドウタイトル&開始処理
	ImGui::Begin("Stage");

	//霧の壁の位置調整
	ImGui::InputFloat3("Pos", &mistWallTransform_.pos.x);
	ImGui::SliderFloat("PosX", &mistWallTransform_.pos.x, -5000.0f, 5000.0f);
	ImGui::SliderFloat("PosY", &mistWallTransform_.pos.y, -5000.0f, 5000.0f);
	ImGui::SliderFloat("PosZ", &mistWallTransform_.pos.z, -5000.0f, 5000.0f);

	//霧の壁の位置調整
	ImGui::InputFloat3("Scale", &mistWallTransform_.scl.x);
	ImGui::SliderFloat("ScaleX", &mistWallTransform_.scl.x, 0.0f, 1000.0f);
	ImGui::SliderFloat("ScaleY", &mistWallTransform_.scl.y, 0.0f, 1000.0f);
	ImGui::SliderFloat("ScaleZ", &mistWallTransform_.scl.z, 0.0f, 1000.0f);

	//終了処理
	ImGui::End();
}

void Stage::Init3DModel(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json& data = jsonM.GetJsonData(JsonManager::JSON_DATA::STAGE);

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(KEY_STAGE))assert(0 && "データが存在しないか不正なデータです");
	const json& stageData = data[KEY_STAGE];
	//データが含まれていない場合はエラーメッセージを出す
	if (!stageData.contains(KEY_THEATER))assert(0 && "データが存在しないか不正なデータです");
	const json& theaterData = stageData[KEY_THEATER];

	//データが含まれていない場合はエラーメッセージを出す
	if (!theaterData.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const auto& theaterTransformData = theaterData[JsonManager::KEY_TRANSFORM];

	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::THEATER));
	//const float stageScale = theaterTransformData.value(JsonManager::KEY_SCALE, 1.0f);
	//transform_.scl = { stageScale ,stageScale ,stageScale };
	transform_.scl = JsonManager::GetParseVector(theaterTransformData, JsonManager::KEY_SCALE);
	transform_.pos = JsonManager::GetParseVector(theaterTransformData, JsonManager::KEY_POSITION);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.MakeCollider(Collider::TYPE::STAGE);
	transform_.Update();
	//データが含まれていない場合はエラーメッセージを出す
	if (!stageData.contains(KEY_MIST_WALL))assert(0 && "データが存在しないか不正なデータです");
	const json& mistWallData = stageData[KEY_MIST_WALL];

	//データが含まれていない場合はエラーメッセージを出す
	if (!mistWallData.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const auto& mistWallTransformData = mistWallData[JsonManager::KEY_TRANSFORM];

	mistWallTransform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::MIST_WALL));
	mistWallTransform_.scl = JsonManager::GetParseVector(mistWallTransformData, JsonManager::KEY_SCALE);
	mistWallTransform_.pos = JsonManager::GetParseVector(mistWallTransformData, JsonManager::KEY_POSITION);
	mistWallTransform_.quaRot = Quaternion();
	mistWallTransform_.quaRotLocal = Quaternion();
	mistWallTransform_.MakeCollider(Collider::TYPE::STAGE);
	mistWallTransform_.Update();

	noiseTextureId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::NOISE_TEXTURE).handleId_;
}

void Stage::InitMaterial(const VECTOR pos, VECTOR sPos)
{
	//モデル描画用
	stageMaterial_ = std::make_unique<ModelMaterial>(
		"StdModelVS.cso", 2,
		"StdModelPS.cso", 7
	);
	//カメラ座標
	VECTOR CameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	stageMaterial_->AddConstBufVS({ CameraPos.x,CameraPos.y,CameraPos.z,0.0f });
	//フォグ座標
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	stageMaterial_->AddConstBufVS({ fogStart,fogEnd,0.0f,0.0f });

	//ピクセルシェーダーの定数バッファ設定
	stageMaterial_->AddConstBufPS({ 1.0f,1.0f,1.0f,1.0f });

	VECTOR lightDir = GetLightDirection();
	stageMaterial_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,0.0f });

	float ambient = 0.0f;
	stageMaterial_->AddConstBufPS({ ambient,ambient,ambient,1.0f });

	int fogColorR, fogColorG, fogColorB;
	GetFogColor(&fogColorR, &fogColorG, &fogColorB);
	stageMaterial_->AddConstBufPS({ 0.1f,0.1f,0.1f,1.0f });

	//ポイントライト
	stageMaterial_->AddConstBufPS({ pos.x,pos.y,pos.z,500.0f });

	//スポットライト
	stageMaterial_->AddConstBufPS({ sPos.x,sPos.y,sPos.z,600.0f });
	VECTOR spotDir = CommonUtility::DIR_D;
	stageMaterial_->AddConstBufPS({ spotDir.x,spotDir.y,spotDir.z,120.0f });

	stageRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *stageMaterial_);

	//霧の壁のマテリアル
	//モデル描画用
	mistWallMaterial_ = std::make_unique<ModelMaterial>(
		"MistWallVS.cso", 2,
		"MistWallPS.cso", 4
	);
	mistWallMaterial_->SetTextureAddress(ModelMaterial::TEXADDRESS::WRAP);
	VECTOR uvScale = { 4.0f,4.0f,4.0f };
	mistWallMaterial_->AddConstBufVS({ uvScale.x,uvScale.y,uvScale.z,uvScale.z });
	mistWallMaterial_->AddConstBufVS({ mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_ });

	//ピクセルシェーダーの定数バッファ設定
	mistWallMaterial_->AddConstBufPS({ 1.0f,1.0f,1.0f,1.0f });
	//ライトの方向とスクロール時間
	mistWallMaterial_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,mistScrollSpeed_ });
	//環境光
	//ambient = 0.5f;
	//mistWallMaterial_->AddConstBufPS({ ambient,ambient,ambient,ambient });
	//ディゾルブの閾値と範囲
	mistWallMaterial_->AddConstBufPS({ dissolveAlphaLine_,ALPHA_RANGE,0.0f,0.0f });
	//dissolveの輪郭線の色
	mistWallMaterial_->AddConstBufPS({ 0.0f,0.0f,0.5f,1.0f });
	
	//ディゾルブ
	//mistWallMaterial_->AddConstBufPS({ dissolveSpeed_,dissolveSpeed_,dissolveSpeed_,dissolveSpeed_ });

	mistWallMaterial_->SetTextureBuf(1, noiseTextureId_);

	mistWallRenderer_ = std::make_unique<ModelRenderer>(mistWallTransform_.modelId, *mistWallMaterial_);

}

void Stage::UpdateStageMaterialConstBuf(void)
{
	//ステージマテリアルの定数バッファ更新
	//カメラ座標更新
	VECTOR CameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	stageMaterial_->SetConstBufVS(0, { CameraPos.x,CameraPos.y,CameraPos.z,0.0f });
	//フォグ座標更新
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	stageMaterial_->SetConstBufVS(1, { fogStart,fogEnd,0.0f,0.0f });
	//フォグの色
	int fogColorR, fogColorG, fogColorB;
	GetFogColor(&fogColorR, &fogColorG, &fogColorB);
	stageMaterial_->SetConstBufPS(3, { 0.0f,0.0f,0.0f,0.0f });
}

void Stage::UpdateMistWallMaterialConstBuf(void)
{
	//霧の壁の定数バッファ更新
	//スクロール速度更新
	mistWallMaterial_->SetConstBufVS(1,
		{ mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_ });
	//ライトの方向とスクロール時間
	VECTOR dir = GetLightDirection();
	mistWallMaterial_->SetConstBufPS(1, { dir.x,dir.y,dir.z,mistScrollSpeed_ });
	mistWallMaterial_->SetConstBufPS(2, { dissolveAlphaLine_,ALPHA_RANGE,0.0f,0.0f });
}
