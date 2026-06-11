#include "../Application.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../../Common/AnimationController.h"
#include "../Utility/CommonUtility.h"
#include "../../Common/Geometry/Capsule.h"
#include "../../Common/Geometry/Sphere.h"
#include "FighterEnemy.h"

namespace
{
	const float FOLLOW_SPEED = 7.0f;	//追従速度

	const float TIME_ROT = 0.1f;		//回転にかける時間
}

FighterEnemy::FighterEnemy(Player& player):
	SummonEnemyBase(player)
{
	state_ = STATE::NONE;
	stepRotTime_ = 0.0f;
	isSummoned_ = false;
	stateTimer_ = 0.0f;
	isAttack_ = false;
	isHitAttack_ = false;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&FighterEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&FighterEnemy::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&FighterEnemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK, std::bind(&FighterEnemy::ChangeStateAttack, this));
}

FighterEnemy::~FighterEnemy(void)
{
}

void FighterEnemy::Init(void)
{
	//3Dモデルの初期化
	Init3DModel();

	//アニメーションの初期化
	InitAnimation();

	//マテリアルの初期化
	InitMaterial();

	//当たり判定の初期化
	InitCollider();
}

void FighterEnemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	VECTOR cameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
	material_->SetConstBufVS(0, { cameraPos.x,cameraPos.y,cameraPos.z,0.0f });
	//フォグ座標更新
	float fogStart, fogEnd = 0.0f;
	GetFogStartEnd(&fogStart, &fogEnd);
	material_->SetConstBufVS(1, { fogStart,fogEnd,fogStart,fogEnd });
	//ライトの方向
	VECTOR lightDir = GetLightDirection();
	material_->SetConstBufPS(1, { lightDir.x,lightDir.y,lightDir.z,0.0f });
	material_->SetConstBufPS(4, { cameraPos.x,cameraPos.y,cameraPos.z,
		SceneManager::GetInstance().GetTotalTime()});

	transform_.Update();
	animationController_->Update();
}

void FighterEnemy::Draw(void)
{
	//モデルの描画
	renderer_->Draw();

	int col = Sphere::COLOR;
	if (isAttack_)col = 0xFF0000;
	sphere_->Draw(col,true);
}

void FighterEnemy::Init3DModel(void)
{
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::FIGHTER_GHOST));
	//モデルの大きさ(Jsonデータから取得できなかったら1.0f)
	const float scale = 0.7f;
	//const float scale = 50.0f;
	transform_.scl = { scale ,scale ,scale };
	//モデルの初期位置
	transform_.pos = CommonUtility::VECTOR_ZERO;
	//モデルの初期回転(度数法で保存されているのでラジアンに変換)
	const float rotY = 180.0f;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();
}

void FighterEnemy::InitAnimation(void)
{
	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/Fighter_Ghost/";
	const float animSpeed = 15.0f;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Ghost_Idle.mv1",
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Fighter_Attack.mv1",
		animSpeed);
}

void FighterEnemy::InitMaterial(void)
{
	//シェーダー毎の定数バッファ数
	const int VS_CONST_BUF_NUM = 2;
	const int PS_CONST_BUF_NUM = 5;
	material_ = std::make_unique<ModelMaterial>(
		"GhostVS.cso", VS_CONST_BUF_NUM,
		"GhostPS.cso", PS_CONST_BUF_NUM
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
	float ambient = 0.05f;
	material_->AddConstBufPS({ ambient,ambient,ambient,ambient });

	//フォグの色
	const FLOAT4 fogColor = { 0.1f,0.1f,0.1f,1.0f };
	material_->AddConstBufPS(fogColor);

	//カメラの位置
	material_->AddConstBufPS({ cameraPos.x,cameraPos.y,cameraPos.z,0.0f });

	renderer_ = std::make_unique<ModelRenderer>(transform_.modelId, *material_);
}

void FighterEnemy::InitCollider(void)
{
	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	const VECTOR localPosTop = { 0.0f, 70.0f, 0.0f };
	const VECTOR localPosDown = { 0.0f, 20.0f, 0.0f };
	const float capsuleRadius = 20.0f;
	capsule_->SetLocalPosTop(localPosTop);
	capsule_->SetLocalPosDown(localPosDown);
	capsule_->SetRadius(capsuleRadius);

	//球コライダ
	const VECTOR localPos = { 0.0f, 40.0f, 30.0f };
	const float sphereRadius = 25.0f;
	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos(localPos);
	sphere_->SetRadius(sphereRadius);
}

void FighterEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateNone, this);
}

void FighterEnemy::ChangeStateSummon(void)
{
	stateUpdate_ = std::bind(&FighterEnemy::UpdateSummon, this);
}

void FighterEnemy::ChangeStateMove(void)
{
	animationController_->Play((int)ANIM_TYPE::IDLE);
	stateUpdate_ = std::bind(&FighterEnemy::UpdateMove, this);
}

void FighterEnemy::ChangeStateAttack(void)
{
	//攻撃アニメーション再生(ループなし)
	animationController_->Play((int)ANIM_TYPE::ATTACK,false);
	isAttack_ = false;
	isHitAttack_ = false;
	stateUpdate_ = std::bind(&FighterEnemy::UpdateAttack, this);
}

void FighterEnemy::UpdateNone(void)
{//何もしない
}

void FighterEnemy::UpdateSummon(void)
{
	transform_.pos.y++;
	if (transform_.pos.y >= -200.0f)
	{
		transform_.pos.y = -200.0f;
		IsSummoned();
		ChangeState(STATE::MOVE);
	}
}

void FighterEnemy::UpdateMove(void)
{
	//追従処理
	//FollowMove();

	stateTimer_ += SceneManager::GetInstance().GetDeltaTime();
	if(stateTimer_ >= 2.0f)
	{
		stateTimer_ = 0.0f;
		ChangeState(STATE::ATTACK);
	}

	Rotate2Player();

	//回転処理
	Rotate();
}

void FighterEnemy::UpdateAttack(void)
{
	isAttack_ = false;
	//攻撃処理
	const float currentStep = animationController_->GetPlayAnimStep();
	const float attackStartStep = 25.0f;
	const float attackEndStep = 35.0f;
	//攻撃の当たり判定を有効にするタイミング（斧を振りかぶるとき）
	if (currentStep > attackStartStep && currentStep < attackEndStep)
	{
		isAttack_ = true;
	}

	//アニメーションが終わったら移動状態に遷移
	if(IsEndAttack())
	{
		ChangeState(STATE::MOVE);
	}

	//攻撃の当たり判定
	if (!isHitAttack_ && isAttack_)
	{
		if(CommonUtility::IsHitSphereCapsule(
			sphere_->GetPos(), sphere_->GetRadius(),
			player_.GetCapsule().GetPosTop(),
			player_.GetCapsule().GetPosDown(),
			player_.GetCapsule().GetRadius()
		))
		{
			player_.Damage(10.0f);
			isHitAttack_ = true;
			//画面揺らし
			SceneManager::GetInstance().StartShakeScreen();
		}
	}

	if (animationController_->GetPlayAnimStep() > attackStartStep)return;
	//追従（プレイヤーを見続ける）
	Rotate2Player();

	//回転処理
	Rotate();
}

void FighterEnemy::FollowMove(void)
{
	// プレイヤーの位置
	VECTOR playerPos = player_.GetTransform().pos;

	//敵とプレイヤーの位置ベクトルを作成
	//プレイヤーの座標から敵の座標を引く
	VECTOR lookAt;
	lookAt = VSub(playerPos, transform_.pos);

	//位置ベクトルを正規化して方向ベクトルを作る
	// posE2P → direction
	//大きさ √をとる関数 sqrt    float用  sqrtf
	float size = sqrtf(lookAt.x * lookAt.x + lookAt.z * lookAt.z);

	//敵の移動処理
	if (size < FOLLOW_SPEED)
	{
		//ぶるぶるしないように
		//移動量よりも位置差が短い場合はプレイヤーに重なる
		transform_.pos = { static_cast<float>(playerPos.x) ,
			static_cast<float>(playerPos.y),
			static_cast<float>(playerPos.z) };
	}
	else
	{
		//正規化　位置ベクトルを大きさで割る
		VECTOR dirNorm = { lookAt.x / size, lookAt.y / size,lookAt.z / size };

		//位置ベクトルを使って敵を移動
		transform_.pos.x += static_cast<float>(dirNorm.x * FOLLOW_SPEED);
		transform_.pos.z += static_cast<float>(dirNorm.z * FOLLOW_SPEED);

		//向き画像を決める
		//水平か鉛直を選択する
		VECTOR dir = CommonUtility::VECTOR_ZERO;

		if (abs(dirNorm.x) < abs(dirNorm.y))
		{
			//鉛直の向き(UP or DOWN)
			if (dirNorm.y < 0.0F)
			{
				dir = CommonUtility::DIR_F;
			}
			else
			{
				dir = CommonUtility::DIR_B;
			}
		}
		else
		{
			//水平の向き(RIHGT or LEFT)
			if (dirNorm.x < 0.0F)
			{
				dir = CommonUtility::DIR_L;
			}
			else
			{
				dir = CommonUtility::DIR_R;
			}
		}

		//敵からプレイヤーへの位置ベクトルを作成
		float angle = atan2(lookAt.x, lookAt.z);
		SetGoalRotate(angle);
	}
}

bool FighterEnemy::IsEndAttack(void)
{
	// アニメーションが攻撃ではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::ATTACK)
	{
		return true;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return true;	//終了している
	}

	return false;
}
