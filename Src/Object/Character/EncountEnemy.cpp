#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "../Common/AnimationController.h"
#include "../EnemyBullet.h"
#include "EncountEnemy.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONキー名を定義
	static const std::string KEY_ENEMY = "Enemy";
	//アニメーションキー名
	static const std::string KEY_IDLE = "Idle";		//待機
	static const std::string KEY_TURN = "Turn";		//振り向き
	static const std::string KEY_WALK = "Walk";		//歩き
}

EncountEnemy::EncountEnemy(void)
{
	isEncount_ = false;
	stateStep_ = 0.0f;
	state_ = STATE::NONE;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&EncountEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::ENCOUNT, std::bind(&EncountEnemy::ChangeStateEncount, this));
	stateChanges_.emplace(STATE::TURN, std::bind(&EncountEnemy::ChangeStateTurn, this));
	stateChanges_.emplace(STATE::ENCOUNT_FINISH, std::bind(&EncountEnemy::ChangeStateEncountFinish, this));
}

EncountEnemy::~EncountEnemy(void)
{
}

void EncountEnemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();

	//アニメーションの初期化
	InitAnimation();

	//マテリアルの初期化
	InitMaterial();

	//状態遷移の初期化
	ChangeState(STATE::NONE);
}

void EncountEnemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	//マテリアルの更新
	UpdateMaterial();

	animationController_->Update();
	transform_.Update();
}

void EncountEnemy::Draw(void)
{
	//エンカウントしていない場合は描画しない
	if (!isEncount_)return;

	//モデルの描画
	renderer_->Draw();

	//丸影描画
	DrawShadow();
}

void EncountEnemy::EncountStart(void)
{
	//エンカウント開始
	isEncount_ = true;
	ChangeState(STATE::ENCOUNT);
}

void EncountEnemy::Turn(void)
{
	ChangeState(STATE::TURN);
}

void EncountEnemy::ChangeState(const STATE& state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void EncountEnemy::Init3DModel(void)
{
	//Jsonデータ取得
	const json& data = GetJsonData();

	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(JsonManager::KEY_TRANSFORM))assert(0 && "データが存在しないか不正なデータです");
	const json& transformData = data[JsonManager::KEY_TRANSFORM];

	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ENEMY));
	//モデルの大きさ(Jsonデータから取得できなかったら1.0f)
	const float scale = transformData.value(JsonManager::KEY_SCALE, 1.0f);
	transform_.scl = { scale ,scale ,scale };
	//モデルの初期位置
	transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
	//モデルの初期回転(度数法で保存されているのでラジアンに変換)
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
}

void EncountEnemy::InitAnimation(void)
{
	//Jsonデータ取得
	const json& data = GetJsonData();
	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(JsonManager::KEY_ANIMATION))assert(0 && "データが存在しないか不正なデータです");
	const json& animPath = data[JsonManager::KEY_ANIMATION];

	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/Animation/";
	const char* KEY_EMPTY = "";
	const float animSpeed = animPath.value(JsonManager::KEY_ANIM_SPEED, 0.0f);
	const float animSpeedSlow = animSpeed / 2.0f;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + animPath.value(KEY_IDLE, KEY_EMPTY),
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::TURN, path + animPath.value(KEY_TURN, KEY_EMPTY),
		animSpeedSlow);
	animationController_->Add((int)ANIM_TYPE::WALK, path + animPath.value(KEY_WALK, KEY_EMPTY),
		animSpeed);
	//初期アニメーションはアイドルを再生
	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void EncountEnemy::InitMaterial(void)
{
	//シェーダー毎の定数バッファ数
	const int VS_CONST_BUF_NUM = 2;
	const int PS_CONST_BUF_NUM = 5;
	//モデル描画用
	material_ = std::make_unique<ModelMaterial>(
		"EnemyRimLightVS.cso", VS_CONST_BUF_NUM,
		"EnemyRimLightPS.cso", PS_CONST_BUF_NUM
	);
	//頂点シェーダーの定数バッファ設定
	//カメラ座標
	VECTOR cameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	material_->AddConstBufVS({ cameraPos.x,cameraPos.y,cameraPos.z,0.0f });

	//フォグの開始距離と終了距離
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	material_->AddConstBufVS({ fogStart,fogEnd,0.0f,0.0f });

	//ピクセルシェーダーの定数バッファ設定
	//モデルカラー
	const FLOAT4 modelColor = { 1.0f,1.0f,1.0f,1.0f };
	material_->AddConstBufPS(modelColor);
	//ライトの方向
	VECTOR lightDir = GetLightDirection();
	material_->AddConstBufPS({ lightDir.x,lightDir.y,lightDir.z,0.0f });

	//環境光
	float ambient = 0.0f;
	material_->AddConstBufPS({ ambient,ambient,ambient,ambient });

	//フォグの色
	const FLOAT4 fogColor = { 0.1f,0.1f,0.1f,1.0f };
	material_->AddConstBufPS(fogColor);

	//カメラの位置
	material_->AddConstBufPS({ cameraPos.x,cameraPos.y,cameraPos.z,0.0f });

	renderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *material_);
}

void EncountEnemy::UpdateMaterial(void)
{
	//マテリアルの定数バッファ更新
	//カメラ座標更新
	int constBufPSIdx = 0;	//定数バッファのインデックス
	VECTOR cameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	material_->SetConstBufVS(0, { cameraPos.x,cameraPos.y,cameraPos.z,0.0f });
	//フォグ座標更新
	float fogStart, fogEnd = 0.0f;
	constBufPSIdx = 1;		//定数バッファのインデックス
	GetFogStartEnd(&fogStart, &fogEnd);
	material_->SetConstBufVS(constBufPSIdx, { fogStart,fogEnd,0.0f,0.0f });
	//ピクセルシェーダー
	//フォグの色
	constBufPSIdx = 3;		//定数バッファのインデックス
	material_->SetConstBufPS(constBufPSIdx, { 0.0f,0.0f,0.0f,0.0f });
	//カメラの位置
	constBufPSIdx = 4;		//定数バッファのインデックス
	material_->SetConstBufPS(constBufPSIdx,
		{ cameraPos.x,cameraPos.y,cameraPos.z,
		SceneManager::GetInstance().GetTotalTime() });
}

void EncountEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EncountEnemy::UpdateNone, this);
}

void EncountEnemy::ChangeStateEncount(void)
{
	stateUpdate_ = std::bind(&EncountEnemy::UpdateEncount, this);
}

void EncountEnemy::ChangeStateTurn(void)
{
	animationController_->Play((int)ANIM_TYPE::TURN, false);
	stateUpdate_ = std::bind(&EncountEnemy::UpdateTurn, this);
}

void EncountEnemy::ChangeStateEncountFinish(void)
{
	//敵の向きを戦闘開始時の向きに設定
	const float battleRotY = 180.0f;
	transform_.quaRot =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(-battleRotY), 0.0f });
	stateUpdate_ = std::bind(&EncountEnemy::UpdateEncountFinish, this);
}

void EncountEnemy::UpdateNone(void)
{//何もしない
}

void EncountEnemy::UpdateEncount(void)
{
}

void EncountEnemy::UpdateTurn(void)
{
	//アニメーションが終了したら待機アニメーションに切り替える
	if (animationController_->IsEnd())
	{
		animationController_->Play((int)ANIM_TYPE::IDLE);
		ChangeState(STATE::ENCOUNT_FINISH);
		return;
	}
}

void EncountEnemy::UpdateEncountFinish(void)
{
}

const json EncountEnemy::GetJsonData(void)const
{
	JsonManager& jsonM = JsonManager::GetInstance();
	//Jsonデータ取得
	const json data = jsonM.GetJsonData(
		JsonManager::JSON_DATA::ENEMY, KEY_ENEMY);

	return data;
}