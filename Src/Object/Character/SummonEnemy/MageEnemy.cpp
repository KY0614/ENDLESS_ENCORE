#include <random>
#include "../Application.h"
#include "../Renderer/ModelRenderer.h"
#include "../Renderer/ModelMaterial.h"
#include "../Manager/GameSystem/Camera.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/JsonManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../../Common/AnimationController.h"
#include "../Utility/CommonUtility.h"
#include "../../Common/Geometry/Capsule.h"
#include "../../Common/Geometry/Sphere.h"
#include "../../Bullet/EnemyBullet.h"
#include "../../Bullet/StraightBullet.h"
#include "../../UI/HPBar.h"
#include "MageEnemy.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

namespace
{
	//JSONのデータのオブジェクト指定キー
	static const std::string KEY_MAGE = "Mage";
	//アニメーションキー名
	static const std::string KEY_IDLE = "Idle";		//通常
	static const std::string KEY_ATTACK = "Attack";	//攻撃
}

MageEnemy::MageEnemy(Player& player) :
	SummonEnemyBase(player)
{
	state_ = STATE::NONE;
	stepRotTime_ = 0.0f;
	isSummoned_ = false;
	bulletInterval_ = 0.0f;
	changeDirStep_ = 0.0f;
	movedPos_ = CommonUtility::VECTOR_ZERO;		
	movePow_ = CommonUtility::VECTOR_ZERO;
	moveDir_ = CommonUtility::VECTOR_ZERO;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&MageEnemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::SUMMON, std::bind(&MageEnemy::ChangeStateSummon, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&MageEnemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK, std::bind(&MageEnemy::ChangeStateAttack, this));
	stateChanges_.emplace(STATE::DAMAGE, std::bind(&MageEnemy::ChangeStateDamage, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&MageEnemy::ChangeStateDead, this));
}

MageEnemy::~MageEnemy(void)
{
}

void MageEnemy::Init(void)
{
	isAlive_ = true;
	//3Dモデルの初期化
	Init3DModel();

	//アニメーションの初期化
	InitAnimation();

	//マテリアルの初期化
	InitMaterial();

	//当たり判定の初期化
	InitCollider();

	//UIの初期化
	InitUI();

	//サウンドの初期化
	InitSound();
	//弾の生成と初期化
	bullet_ = std::make_unique<StraightBullet>(transform_,player_.GetTransform().pos);
	bullet_->Init();
}

void MageEnemy::Update(void)
{
	if (!isAlive_)return;
	if(hp_ <= 0.0f && state_ != STATE::DEAD)
	{
		ChangeState(STATE::DEAD);
	}

	//更新ステップ
	stateUpdate_();

	transform_.Update();
	animationController_->Update();
}

void MageEnemy::Draw(void)
{
	if (!isAlive_)return;
	//モデルの描画
	renderer_->Draw();
}

void MageEnemy::DrawUI(void)
{
	//HPが最大値より減っていたら表示
	if (hp_ < maxHp_)hpBar_->SetActive(true);
	else hpBar_->SetActive(false);

	//HPバーの描画
	hpBar_->DrawBillboard();
}
 
void MageEnemy::Init3DModel(void)
{
	const JsonManager& jsonM = JsonManager::GetInstance();
	const json& data = jsonM.GetJsonData(
		JsonManager::JSON_DATA::FIGHTER_GHOST, KEY_MAGE);
	//データが含まれていない場合はエラーメッセージを出す
	if (!data.contains(JsonManager::KEY_TRANSFORM))
	{
		assert(0 && "データが存在しないか不正なデータです");
	}
	const json& transformData = data[JsonManager::KEY_TRANSFORM];
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::MAGE_GHOST));
	//モデルの大きさ(Jsonデータから取得できなかったら1.0f)
	const float scale = transformData.value(JsonManager::KEY_SCALE, 1.0f);
	//const float scale = 50.0f;
	transform_.scl = { scale ,scale ,scale };
	//モデルの初期位置
	transform_.pos = JsonManager::GetParseVector(transformData, JsonManager::KEY_POSITION);
	//モデルの初期回転(度数法で保存されているのでラジアンに変換)
	const float rotY = transformData.value(JsonManager::KEY_ROT_Y, 0.0f);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(rotY), 0.0f });
	transform_.Update();

	//HPを設定
	const json& paramData = data[JsonManager::KEY_PARAMETER];
	SetHP(paramData.value(JsonManager::KEY_HP, 0.0f));
	SetMaxHP(paramData.value(JsonManager::KEY_MAX_HP, 0.0f));
}

void MageEnemy::InitAnimation(void)
{
	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Enemy/Mage_Ghost/";
	const float animSpeed = 15.0f;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Ghost_Idle.mv1",
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Mage_Attack.mv1",
		animSpeed);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Damage.mv1",
		animSpeed * 2);
}

void MageEnemy::InitMaterial(void)
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

void MageEnemy::InitCollider(void)
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

void MageEnemy::InitUI(void)
{
	const VECTOR offset = { 0.0f, 100.0f, 0.0f };
	const float uiScale = 1.0f;
	hpBar_ = std::make_unique<HPBar>(
		HPBar::BillboardInfo{
			HPBar::TYPE::ENEMY,
			&transform_.pos,
			offset,
			uiScale,//UIの拡大率X
			uiScale	//UIの拡大率Y
		},
		hp_, maxHp_);
	hpBar_->Init();
}

void MageEnemy::InitSound(void)
{
	SoundManager& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::DAMAGE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::DAMAGE_SE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::DAMAGE, 70);
}

void MageEnemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&MageEnemy::UpdateNone, this);
}

void MageEnemy::ChangeStateSummon(void)
{
	stateUpdate_ = std::bind(&MageEnemy::UpdateSummon, this);
}

void MageEnemy::ChangeStateMove(void)
{
	animationController_->Play((int)ANIM_TYPE::IDLE);
	stateUpdate_ = std::bind(&MageEnemy::UpdateMove, this);
}

void MageEnemy::ChangeStateAttack(void)
{
	//コライダを弾に追加
	if (!colliders_.empty())
	{
		for (const std::weak_ptr<Collider> c : colliders_)
		{
			bullet_->AddCollider(c);
		}
	}
	if (bullet_->GetState() == StraightBullet::STATE::DESTROY)
	{
		bullet_->Reset(transform_);
	}
	VECTOR headPos = VAdd(transform_.pos, VScale(transform_.GetUp(), 100.0f));
	VECTOR ofssetPos = VSub(headPos, transform_.pos);
	bullet_->SetOffsetPos(ofssetPos);
	bullet_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	bullet_->SetPos(headPos);
	//弾を発射可能状態にする
	bullet_->SetStateReady();
	//攻撃アニメーション再生(ループなし)
	animationController_->Play((int)ANIM_TYPE::ATTACK, false);
	stateUpdate_ = std::bind(&MageEnemy::UpdateAttack, this);
}

void MageEnemy::ChangeStateDamage(void)
{
	//ダメージアニメーション再生(ループなし)
	animationController_->Play((int)ANIM_TYPE::DAMAGE, false);
	//ダメージ音再生
	SoundManager::GetInstance().Play(SoundManager::SOUND::DAMAGE);
	stateUpdate_ = std::bind(&MageEnemy::UpdateDamage, this);
}

void MageEnemy::ChangeStateDead(void)
{
	bullet_->SetStateDestroy();
	stateUpdate_ = std::bind(&MageEnemy::UpdateDead, this);
}

void MageEnemy::UpdateNone(void)
{//何もしない
}

void MageEnemy::UpdateSummon(void)
{
	transform_.pos.y++;
	if (transform_.pos.y >= -200.0f)
	{
		transform_.pos.y = -200.0f;
		IsSummoned();
		ChangeState(STATE::MOVE);
	}
}

void MageEnemy::UpdateMove(void)
{
	bulletInterval_ += SceneManager::GetInstance().GetDeltaTime();

	if(bulletInterval_ >= 2.0f)
	{
		bulletInterval_ = 0.0f;
		ChangeState(STATE::ATTACK);
		return;
	}

	//移動
	//Move();

	//プレイヤーを見続ける
	Rotate2Player();

	//回転処理
	Rotate();

	//ダメージ判定
	if (bullet_->GetState() == StraightBullet::STATE::REVERSE &&
		CommonUtility::IsHitSpheres(
			bullet_->GetSphere().GetPos(),
			bullet_->GetSphere().GetRadius(),
			sphere_->GetPos(),
			sphere_->GetRadius()))
	{
		hp_ -= 10.0f;
		bullet_->SetStateDestroy();
		ChangeState(STATE::DAMAGE);
	}

	bullet_->Update();
}

void MageEnemy::UpdateAttack(void)
{
	//攻撃
	Shoot();

	//攻撃アニメーションが終わったら移動状態へ遷移
	if (animationController_->IsEnd() &&
		(bullet_->GetState() == StraightBullet::STATE::DESTROY ||
		 bullet_->GetState() == StraightBullet::STATE::REVERSE))
	{
		bulletInterval_ = 0.0f;
		ChangeState(STATE::MOVE);
	}

	//プレイヤーを見続ける
	Rotate2Player();

	//回転処理
	Rotate();

	bullet_->Update();
}

void MageEnemy::UpdateDamage(void)
{
	//ダメージアニメーションが終わったら移動状態へ遷移
	if(animationController_->IsEnd())
	{
		ChangeState(STATE::MOVE);
	}
}

void MageEnemy::UpdateDead(void)
{
	isAlive_ = false;
}

void MageEnemy::Move(void)
{
	//移動方向変更の経過時間
	changeDirStep_ += SceneManager::GetInstance().GetDeltaTime();
	//一定時間経過したら移動方向をランダムで変更
	const float changeInterval = 1.0f;
	if (changeDirStep_ >= changeInterval)
	{
		changeDirStep_ = 0.0f;
		std::vector<VECTOR> moveDir =
		{ transform_.GetRight(), transform_.GetLeft() };
		// 乱数生成器の初期化
		std::random_device rd; //非決定的な乱数生成器
		std::mt19937 engine(rd()); //メルセンヌ・ツイスタ法による乱数生成器
		std::shuffle(moveDir.begin(), moveDir.end(), engine);
		moveDir_ = moveDir[0];
	}
	movePow_ = VScale(moveDir_, 1.5f);
	//移動処理
	movedPos_ = VAdd(transform_.pos, movePow_);
	transform_.pos = movedPos_;
}

void MageEnemy::Shoot(void)
{
	bulletInterval_ += SceneManager::GetInstance().GetDeltaTime();

	const float interval = 2.0f;	//弾の発射間隔
	//if (bullet_->GetState() == EnemyBullet::STATE::NONE &&
	//	bulletInterval_ >= interval)
	//{
	//	bullet_->SetStateReady();
	//	bulletInterval_ = 0.0f;
	//}

	if(bulletInterval_ >= interval &&
		bullet_->GetState() == StraightBullet::STATE::READY)
	{
		bulletInterval_ = 0.0f;
		//弾のターゲット座標をプレイヤーの位置に設定
		VECTOR targetPos = player_.GetTransform().pos;
		targetPos.y += 80.0f;
		VECTOR forward = player_.GetTransform().GetForward();
		const float dis = 200.0f;
		targetPos = VAdd(targetPos, VScale(forward, dis));
		bullet_->SetTargetPos(targetPos);
		//弾を発射
		bullet_->SetStateShot();
	}

	if (bullet_->GetState() == StraightBullet::STATE::DESTROY)return;

	//パリィ
	if (player_.GetIsParry() &&
		CommonUtility::IsHitSpheres(
			bullet_->GetSphere().GetPos(),
			bullet_->GetSphere().GetRadius(),
			player_.GetSphere().GetPos(),
			player_.GetSphere().GetRadius()))
	{
		bullet_->SetStateReverse();
		VECTOR targetPos = transform_.pos;
		targetPos.y += 80.0f;
		bullet_->SetTargetPos(targetPos);
	}

	//ダメージ判定（反射された弾と敵の当たり判定）
	if (bullet_->GetState() == StraightBullet::STATE::REVERSE &&
		CommonUtility::IsHitSpheres(
		bullet_->GetSphere().GetPos(),
		bullet_->GetSphere().GetRadius(),
		sphere_->GetPos(),
		sphere_->GetRadius()))
	{
		hp_ -= 10.0f;
		bullet_->SetStateDestroy();
		ChangeState(STATE::DAMAGE);
	}

	//プレイヤーと弾の当たり判定
	if (CommonUtility::IsHitSphereCapsule(
		bullet_->GetSphere().GetPos(),
		bullet_->GetSphere().GetRadius(),
		player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(),
		player_.GetCapsule().GetRadius()))
	{
		//プレイヤーにダメージを与える
		player_.Damage(10.0f);
		//弾を消す
		bullet_->SetStateDestroy();
		bulletInterval_ = 0.0f;
		//画面揺らし
		SceneManager::GetInstance().StartShakeScreen();
	}
}