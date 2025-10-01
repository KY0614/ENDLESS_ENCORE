#include "../Application.h"
#include "../Libs/ImGui/imgui.h"
#include "../Utility/CommonUtility.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "Common/AnimationController.h"
#include "Common/Capsule.h"
#include "Common/Sphere.h"
#include "Player.h"
#include "Enemy.h"

namespace
{
	const float TIME_ROT = 1.3f;
	const float HP_MAX = 100.0f;
	const float MOVE_SPEED = 5.0f;

	const float ATTACK_DISTANCE = 100.0f;
	const float PLAYER_DISTANCE = 300.0f;
	//追従距離
	const float FOLLOW_DISTANCE = 800.0f;
	//重力加速度
	const float MOVE_TIME = 10.0f;
	const float ATTACK_TIME = 1.0f;

	const float ATTACK_DAMAGE = 10.0f;

	//アニメーション再生速度
	const float ANIM_SPEED = 30.0f;

	const int FAR_SPHERE_NUM = 4;

	const float DOWN_TIME = 3.0f;
}

Enemy::Enemy(Player& player):player_(player)
{
	stateStep_ = 0.0f;
	state_ = STATE::NONE;
	col_ = 0xff0000;
	isAttackedNear_ = false;
	isDown_ = false;
	currentAngle_ = 0.0f;               // 初期角度は適当に設定 (atan2で初期化しても良い)
	stepDownTime_ = 0.0f;

	// 例: 1秒で 90度（π/2 ラジアン）回転する速度
	circlingSpeedRad_ = DX_PI_F / 2.0f  * 0.1f;

	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Enemy::ChangeStateNone, this));
	stateChanges_.emplace(STATE::FOLLOW, std::bind(&Enemy::ChangeStateFollow, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&Enemy::ChangeStateMove, this));
	stateChanges_.emplace(STATE::ATTACK_NEAR, std::bind(&Enemy::ChangeStateAttackNear, this));
	stateChanges_.emplace(STATE::ATTACK_FAR, std::bind(&Enemy::ChangeStateAttackFar, this));
	stateChanges_.emplace(STATE::DOWN, std::bind(&Enemy::ChangeStateDown, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Enemy::ChangeStateDead, this));
}

Enemy::~Enemy(void)
{
}

void Enemy::Init(void)
{
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::ENEMY));
	transform_.scl = CommonUtility::VECTOR_ONE;
	transform_.pos = { -60.0f, 0.0f, 250.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 20.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	sphereNear_ = std::make_unique<Sphere>(transform_);
	sphereNear_->SetLocalPos({ 0.0f, 80.0f, 50.0f });
	sphereNear_->SetRadius(30.0f);

	//遠距離攻撃用の球体コライダ
	spheresFar_.emplace_back(std::make_unique<Sphere>(transform_));
	spheresFar_.back()->SetLocalPos({ 80.0f, 185.0f, 0.0f });
	spheresFar_.back()->SetRadius(20.0f);

	spheresFar_.emplace_back(std::make_unique<Sphere>(transform_));
	spheresFar_.back()->SetLocalPos({ -80.0f, 185.0f, 0.0f });
	spheresFar_.back()->SetRadius(20.0f);

	spheresFar_.emplace_back(std::make_unique<Sphere>(transform_));
	spheresFar_.back()->SetLocalPos({ -30.0f, 230.0f, 0.0f });
	spheresFar_.back()->SetRadius(20.0f);

	spheresFar_.emplace_back(std::make_unique<Sphere>(transform_));
	spheresFar_.back()->SetLocalPos({ 30.0f, 230.0f, 0.0f });
	spheresFar_.back()->SetRadius(20.0f);

	InitAnimation();

	ChangeState(STATE::MOVE);
}

void Enemy::Update(void)
{
	//更新ステップ
	stateUpdate_();

	animationController_->Update();
	transform_.Update();
	//UpdateDebugImGui();
}

void Enemy::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	VECTOR pos = ConvWorldPosToScreenPos(transform_.pos);
	if (isDown_)DrawFormatString(pos.x, pos.z + 80.0f, 0xFFFFFF, L"DOWN!!!");

#ifdef _DEBUG

	for (int i = 0; i < FAR_SPHERE_NUM; i++)
	{
		spheresFar_[i]->Draw(0x0000ff);
	}

	if (state_ != STATE::ATTACK_NEAR)return;
	if(!isAttackedNear_)col_= 0x00ff00;
	else col_ = 0xff0000;
	sphereNear_->Draw(col_);

#endif // _DEBUG

}

void Enemy::ChangeState(const STATE state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();
}

void Enemy::InitAnimation(void)
{
	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", ANIM_SPEED);
	animationController_->Add((int)ANIM_TYPE::MOVE, path + "Walking.mv1", ANIM_SPEED);
	//初期アニメーションはアイドルを再生
	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Enemy::FollowPlayer(VECTOR& pos)
{
	// プレイヤーの位置
	VECTOR playerPos = player_.GetTransform().pos;

	//敵とプレイヤーの位置ベクトルを作成
	//プレイヤーの座標から敵の座標を引く
	VECTOR posE2P;
	posE2P = VSub(playerPos, pos);

	//位置ベクトルを正規化して方向ベクトルを作る
	// posE2P → direction
	//大きさ √をとる関数 sqrt    float用  sqrtf
	float size = sqrtf(posE2P.x * posE2P.x + posE2P.z * posE2P.z);


	//敵の移動処理
	if (size < MOVE_SPEED)
	{
		//ぶるぶるしないように
		//移動量よりも位置差が短い場合はプレイヤーに重なる
		pos = { static_cast<float>(playerPos.x) ,
			static_cast<float>(playerPos.y),
			static_cast<float>(playerPos.z) };
	}
	else
	{

		//正規化　位置ベクトルを大きさで割る
		VECTOR direction = { posE2P.x / size, posE2P.y / size,posE2P.z / size };

		//位置ベクトルを使って敵を移動
		pos.x += static_cast<float>(direction.x * MOVE_SPEED);
		pos.z += static_cast<float>(direction.z * MOVE_SPEED);

		//向き画像を決める
		//水平か鉛直を選択する
		//※数値を絶対値(abs関数)としてみる

		VECTOR dir = CommonUtility::VECTOR_ZERO;

		if (abs(direction.x) < abs(direction.y))
		{
			//鉛直の向き(UP or DOWN)
			if (direction.y < 0.0F)
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
			if (direction.x < 0.0F)
			{
				dir = CommonUtility::DIR_L;
			}
			else
			{
				dir = CommonUtility::DIR_R;
			}
		}

		VECTOR targetlPos = { playerPos.x,playerPos.y,playerPos.z };
		//敵からプレイヤーへの位置ベクトルを作成
		float angle = atan2(posE2P.x, posE2P.z);
		float angleDegrees = CommonUtility::Rad2DegF(angle);
		SetGoalRotate(angle);
	}

}

void Enemy::SetGoalRotate(double rotRad)
{
	Quaternion axis =
		Quaternion::AngleAxis(
			rotRad, CommonUtility::AXIS_Y);

	//現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	//しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}
	//目標回転を設定
	goalQuaRot_ = axis;
}

void Enemy::Rotate(void)
{
	//回転時間の減少
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();

	//回転の球面補間
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(enemyRotY_);
}

void Enemy::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateDead, this);
}

void Enemy::ChangeStateFollow(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateFollow, this);
}

void Enemy::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateMove, this);
}

void Enemy::ChangeStateAttackNear(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateAttackNear, this);
}

void Enemy::ChangeStateAttackFar(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateAttackFar, this);
}

void Enemy::ChangeStateDown(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateDown, this);
}

void Enemy::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Enemy::UpdateDead, this);
}

void Enemy::UpdateNone(void)
{
}

void Enemy::UpdateFollow(void)
{
	FollowPlayer(transform_.pos);

	//プレイヤーとの距離を測り、一定以上近づいたら追従をやめる
	VECTOR distance = VSub(player_.GetTransform().pos, transform_.pos);
	if (VSize(distance) < PLAYER_DISTANCE)
	{
		ChangeState(STATE::MOVE);
	}

	Rotate();

}

void Enemy::UpdateMove(void)
{
	Rotate();

	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > MOVE_TIME)
	{
		stateStep_ = 0.0f;
		ChangeState(STATE::ATTACK_NEAR);
	}

	VECTOR distance = VSub(player_.GetTransform().pos, transform_.pos);
	if(VSize(distance) > FOLLOW_DISTANCE)
	{
		ChangeState(STATE::FOLLOW);
	}

	//// 1. 角度を更新する
	////時間経過で角度を変化させます。プレイヤーの周りを右回り（時計回り）で動く。
	//currentAngle_ += circlingSpeedRad_ * SceneManager::GetInstance().GetDeltaTime();

	//// 角度が一周したらリセット (省略可)
	//if (currentAngle_ > DX_PI_F * 2.0f)
	//{
	//	currentAngle_ -= DX_PI_F * 2.0f;
	//}

	//// 2. プレイヤーの周りの円上の座標を計算する
	//VECTOR playerPos = player_.GetTransform().pos;

	//// X-Z平面での円運動の計算 (極座標からデカルト座標への変換)
	//// X = R * sin(θ)
	//// Z = R * cos(θ)

	//// プレイヤーからの相対位置
	//VECTOR relativePos;
	//relativePos.x = PLAYER_DISTANCE * sinf(currentAngle_);
	//relativePos.y = 0.0f; // プレイヤーの高さと合わせる
	//relativePos.z = PLAYER_DISTANCE * cosf(currentAngle_);

	////3. 敵のワールド座標を決定する
	////プレイヤーの位置 + プレイヤーからの相対位置
	//transform_.pos = VAdd(playerPos, relativePos);

	////4.プレイヤーの方を向く処理
	////移動した新しい位置からプレイヤーの方を向くように回転角度を計算し直す

	////敵からプレイヤーへのベクトル (このベクトルは原点(0,0,0)を向くベクトルと180度ずれている)
	////正しいターゲット方向ベクトルは relativePos の逆ベクトルになる
	//VECTOR posE2P = VScale(relativePos, -1.0f);

	////atan2 で角度を計算
	//float angle = atan2(posE2P.x, posE2P.z);

	//SetGoalRotate(angle);

	////回転処理を実行
	//Rotate();

}

void Enemy::UpdateAttackNear(void)
{
	Rotate();

	VECTOR distance = VSub(player_.GetTransform().pos, transform_.pos);
	if (VSize(distance) > ATTACK_DISTANCE)
	{
		isAttackedNear_ = false;
		FollowPlayer(transform_.pos);
		return;
	}
	isAttackedNear_ = true;

	if (CommonUtility::IsHitSpheres(sphereNear_->GetPos(),sphereNear_->GetRadius(),
		player_.GetSphere().GetPos(),player_.GetSphere().GetRadius()))
	{
		if (player_.GetisParry())
		{
			ChangeState(STATE::DOWN);
			isAttackedNear_ = false;
			return;
		}
	}

	//当たり判定
	if(CommonUtility::IsHitSphereCapsule(sphereNear_->GetPos(),
		sphereNear_->GetRadius(),player_.GetCapsule().GetPosTop(),
		player_.GetCapsule().GetPosDown(), player_.GetCapsule().GetRadius()))
	{
		if (player_.GetisDodge() && player_.GetisInvicible())return;
		player_.SubHp(ATTACK_DAMAGE);
		ChangeState(STATE::MOVE);
	}
	stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	if (stateStep_ > ATTACK_TIME)
	{
		stateStep_ = 0.0f;
		isAttackedNear_ = false;
		ChangeState(STATE::MOVE);
		return;
	}
}

void Enemy::UpdateAttackFar(void)
{
	if(spheresFar_.empty())
	{
		return;
	}

	//sphere
}

void Enemy::UpdateDown(void)
{
	if(stepDownTime_ > DOWN_TIME)
	{
		stepDownTime_ = 0.0f;
		isDown_ = false;
		ChangeState(STATE::MOVE);
		return;
	}
	stepDownTime_ += SceneManager::GetInstance().GetDeltaTime();
	isDown_ = true;
}

void Enemy::UpdateDead(void)
{
	
}

void Enemy::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Enemy");
	int index = 2;
	ImGui::InputInt("index", &index);
	//位置
	ImGui::Text("spheresFar pos");
	VECTOR pos = spheresFar_[index]->GetLocalPos();

	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("pos", &pos.x);
	ImGui::SliderFloat("posX", &pos.x, -800.0f, 1000.0f);
	ImGui::SliderFloat("posY", &pos.y, -800.0f, 1000.0f);
	ImGui::SliderFloat("posZ", &pos.z, -800.0f, 1000.0f);
	//spheresFar_[index]->SetLocalPos(pos);
	VECTOR localpos = spheresFar_[index]->GetLocalPos();
	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("localpos", &localpos.x);
	ImGui::SliderFloat("localposX", &localpos.x, -800.0f, 1000.0f);
	ImGui::SliderFloat("localposY", &localpos.y, -800.0f, 1000.0f);
	ImGui::SliderFloat("localposZ", &localpos.z, -800.0f, 1000.0f);
	spheresFar_[index]->SetLocalPos(localpos);
	//終了処理
	ImGui::End();
}