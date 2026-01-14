#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "Stage.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_STAGE = "Stage";
	static const std::string KEY_THEATER = "Theater";
	static const std::string KEY_MIST_WALL = "MistWall";
	//ディゾルブの閾値最大値・最小値・範囲
	const float ALPHA_LINE_MAX = 1.1f;	//閾値最大値
	const float ALPHA_LINE_MIN = -0.1f;	//閾値最小値
	const float ALPHA_RANGE = 0.01f;	//範囲
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

	//マテリアル初期化
	InitMaterial(pos, sPos);
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
}

void Stage::Draw(void)
{
	stageRenderer_->Draw();

	//cube_->Draw();

	if (!isBattle_)return;
	mistWallRenderer_->Draw();
}

void Stage::Init3DModel(void)
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json& data = jsonM.GetJsonData(
		JsonManager::JSON_DATA::STAGE,KEY_STAGE);

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(KEY_THEATER))assert(0 && "データが存在しないか不正なデータです");
	const json& theaterData = data[KEY_THEATER];

	//データが含まれていない場合はエラーメッセージを出す
	if (!theaterData.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const auto& theaterTransformData = theaterData[JsonManager::KEY_TRANSFORM];

	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::THEATER));
	transform_.scl = JsonManager::GetParseVector(theaterTransformData, JsonManager::KEY_SCALE);
	transform_.pos = JsonManager::GetParseVector(theaterTransformData, JsonManager::KEY_POSITION);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.MakeCollider(Collider::TYPE::STAGE);
	transform_.Update();
	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(KEY_MIST_WALL))assert(0 && "データが存在しないか不正なデータです");
	const json& mistWallData = data[KEY_MIST_WALL];

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
	//ノイズテクスチャ読み込み
	noiseTextureId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::NOISE_TEXTURE).handleId_;
}

void Stage::InitMaterial(const VECTOR pos, VECTOR sPos)
{
	//シェーダー毎の定数バッファ数
	int VS_CONST_BUF_NUM = 2;
	int PS_CONST_BUF_NUM = 7;
	//モデル描画用
	stageMaterial_ = std::make_unique<ModelMaterial>(
		"StdModelVS.cso", VS_CONST_BUF_NUM,
		"StdModelPS.cso", PS_CONST_BUF_NUM
	);
	//カメラ座標
	VECTOR CameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	stageMaterial_->AddConstBufVS({ CameraPos.x,CameraPos.y,CameraPos.z,0.0f });
	//フォグ座標
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	stageMaterial_->AddConstBufVS({ fogStart,fogEnd,0.0f,0.0f });

	//ピクセルシェーダーの定数バッファ設定
	//モデルカラー
	const FLOAT4 modelColor = { 1.0f,1.0f,1.0f,1.0f };
	stageMaterial_->AddConstBufPS(modelColor);

	//ライトの方向
	VECTOR lightDir = GetLightDirection();
	stageMaterial_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,0.0f });

	//環境光
	float ambient = 0.0f;
	stageMaterial_->AddConstBufPS({ ambient,ambient,ambient,ambient });

	//フォグの色
	const FLOAT4 fogColor = { 0.1f,0.1f,0.1f,1.0f };
	stageMaterial_->AddConstBufPS(fogColor);

	//ポイントライト
	const float pointLightRange = 500.0f;	//範囲
	stageMaterial_->AddConstBufPS({ pos.x,pos.y,pos.z,pointLightRange });

	//スポットライト
	const float spotLightRange = 600.0f;	//範囲
	stageMaterial_->AddConstBufPS({ sPos.x,sPos.y,sPos.z,spotLightRange });
	VECTOR spotDir = CommonUtility::DIR_D;
	const float spotLightAngle = 120.0f;	//角度
	stageMaterial_->AddConstBufPS({ spotDir.x,spotDir.y,spotDir.z,spotLightAngle });

	stageRenderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *stageMaterial_);

	//霧の壁のマテリアル
	//シェーダー毎の定数バッファ数
	VS_CONST_BUF_NUM = 2;
	PS_CONST_BUF_NUM = 4;
	//モデル描画用
	mistWallMaterial_ = std::make_unique<ModelMaterial>(
		"MistWallVS.cso", VS_CONST_BUF_NUM,
		"MistWallPS.cso", PS_CONST_BUF_NUM
	);
	mistWallMaterial_->SetTextureAddress(ModelMaterial::TEXADDRESS::WRAP);
	//UVスケール
	const VECTOR uvScale = { 4.0f,4.0f,4.0f };
	mistWallMaterial_->AddConstBufVS({ uvScale.x,uvScale.y,uvScale.z,uvScale.z });
	mistWallMaterial_->AddConstBufVS({ mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_,mistScrollSpeed_ });

	//ピクセルシェーダーの定数バッファ設定
	mistWallMaterial_->AddConstBufPS(modelColor);
	//ライトの方向とスクロール時間
	mistWallMaterial_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,mistScrollSpeed_ });

	//ディゾルブの閾値と範囲
	mistWallMaterial_->AddConstBufPS({ dissolveAlphaLine_,ALPHA_RANGE,0.0f,0.0f });
	//dissolveの輪郭線の色
	const FLOAT4 edgeColor = { 0.0f,0.0f,0.5f,1.0f };
	mistWallMaterial_->AddConstBufPS(edgeColor);

	//ノイズテクスチャ設定
	const int noiseTextureSlot = 1;
	mistWallMaterial_->SetTextureBuf(noiseTextureSlot, noiseTextureId_);
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
