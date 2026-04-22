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

	animationController_->Update();
	transform_.Update();
}

void EncountEnemy::Draw(void)
{
	//エンカウントしていない場合は描画しない
	if (!isEncount_)return;

	//モデルの描画
	MV1DrawModel(transform_.modelId);
	//renderer_->Draw();

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


void EncountEnemy::ChangeStateCastSpell(void)
{
	//弾の生成
	const int bulletNum = 1;
	bullet_ = std::make_unique<EnemyBullet>(transform_);
	bullet_->Init();
	const VECTOR headPos = GetFramePos(L"mixamorig:Head");
	//弾のオフセット座標
	const VECTOR offsetPos = VSub(headPos, transform_.pos);
	//弾のローカル座標
	const VECTOR localPos = VGet(70.0f, 40.0f, 0.0f);
	//弾の相対座標にセットする
	bullet_->SetOffsetPos(offsetPos);
	bullet_->SetLocalPos(localPos);
	bullet_->SetPos(headPos);

	animationController_->Play((int)ANIM_TYPE::CAST_SPELL, false);
	stateUpdate_ = std::bind(&EncountEnemy::UpdateCastSpell, this);
}

void EncountEnemy::ChangeStateAttackPlayer(void)
{
	animationController_->Play((int)ANIM_TYPE::ATTACK_FAR_ONE, false);
	stateUpdate_ = std::bind(&EncountEnemy::UpdateAttackPlayer, this);
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

void EncountEnemy::UpdateCastSpell(void)
{
	//詠唱アニメーションが終わったら魔法待機へ
	if (IsCastSpell())
	{
		animationController_->Play((int)ANIM_TYPE::MAGIC_ILDE);
	}
	//弾の状態更新
	bullet_->Update();
	//
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();

	//弾を準備状態にする
	const float bulletInterval = 0.7f;
	if (bullet_->GetState() != EnemyBullet::STATE::NONE)return;
	if (stateStep_ > bulletInterval)
	{
		bullet_->SetStateReady();
		stateStep_ = 0.0f;
	}
}

void EncountEnemy::UpdateAttackPlayer(void)
{
	if (IsSpellAttack())
	{
		animationController_->Play((int)ANIM_TYPE::MAGIC_ILDE);
	}
	//弾の状態更新
	bullet_->Update();
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	const float bulletInterval = 0.5f;
	if (stateStep_ > bulletInterval)
	{
		bullet_->SetStateShot();
		//VECTOR targetPos = player_.GetTransform().pos;
		//targetPos.y = player_.GetFramePos(L"mixamorig:Spine").y;
		//bullets_.front()->SetTargetPos(player_.GetTransform().pos);
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

bool EncountEnemy::IsCastSpell(void)
{
	//アニメーションが終了しているか
	if (animationController_->IsEnd() &&
		animationController_->GetPlayType() == (int)ANIM_TYPE::CAST_SPELL)
	{
		return true;	//終了している
	}

	return false;
}

bool EncountEnemy::IsSpellAttack(void)
{
	//アニメーションが終了しているか
	if (animationController_->IsEnd() &&
		animationController_->GetPlayType() == (int)ANIM_TYPE::ATTACK_FAR_ONE)
	{
		return true;	//終了している
	}

	return false;
}
