#include <cassert>
#include<EffekseerForDXLib.h>
#include "../Application.h"
#include "../Utility/CommonUtility.h"
#include "../Libs/ImGui/imgui.h"
#include "../Common/DebugDrawFormat.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/Camera.h"
#include "Common/AnimationController.h"
#include "Common/ControllerAnimation.h"
#include "Common/Capsule.h"
#include "Common/Sphere.h"
#include "Common/Collider.h"
#include "Player.h"
#include "Enemy.h"

namespace
{
	//ジャンプ力
	const float JUMP_POW = 7.5f; 
	//重力加速度
	const float GRAVITY_POW = 15.0f;

	//アニメーション再生速度
	const float ANIM_SPEED = 30.0f;

	const float HP_MAX = 50.0f;
}

Player::Player(void)
{
	animationController_ = nullptr;
	controllerAnimation_ = nullptr;
	state_ = STATE::NONE;
	hp_ = 0.0f;
	//状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Player::ChangeStateDead, this));

	gravHitPosDown_ = CommonUtility::VECTOR_ZERO;
	gravHitPosUp_ = CommonUtility::VECTOR_ZERO;

	chestFrmNo_ = 0;
	chestPos_ = CommonUtility::VECTOR_ZERO;

	hipFrmNo_ = 0;

	effectSmokePlayId_ = -1;
	effectSmokeResId_ = -1;
	stepFootSmoke_ = -1.0f;
	stepJump_ = -1.0f;
	isJump_ = false;
	speed_ = -1.0f;
	movedPos_ = CommonUtility::VECTOR_ZERO;
	moveDir_ = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	moveDiff_ = CommonUtility::VECTOR_ZERO;
	jumpPow_ = CommonUtility::VECTOR_ZERO;
	playerRotY_ = Quaternion::Quaternion();
	goalQuaRot_ = Quaternion::Quaternion();
	stepRotTime_ = 0.0f;
	stepParry_ = 0.0f;
	imgShadow_ = -1;
	isJumpUnlimited_ = false;
	jumpVelocity_ = CommonUtility::VECTOR_ZERO;
	isDodge_ = false;
	stepDodge_ = 0.0f;
	isInvincible_ = false;
	isParry_ = false;
	col_ = -1;
}

Player::~Player(void)
{
}

void Player::Init(void)
{
	//モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	transform_.scl = {0.7f,0.7f,0.7f};
	transform_.pos = { -60.0f, 0.0f, 30.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, CommonUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	//丸影画像
	imgShadow_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	//胸の位置取得
	chestFrmNo_ = MV1SearchFrame(transform_.modelId, L"mixamorig:Hips");
	chestPos_ = MV1GetFramePosition(transform_.modelId, chestFrmNo_);

	//カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 20.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos({ 0.0f, 80.0f, 70.0f });
	sphere_->SetRadius(40.0f);

	//足煙エフェクト
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;	

	//アニメーションの設定
	InitAnimation();

	//初期状態
	ChangeState(STATE::PLAY);

	//歩きエフェクトの発生間隔
	stepFootSmoke_ = TERM_FOOT_SMOKE;

	hp_ = 50.0f;
	col_ = 0x000000;
}

void Player::Update(void)
{
	//更新ステップ
	stateUpdate_();

	//ヒップの位置更新
	//prevPos_ = MV1GetAttachAnimFrameLocalPosition(transform_.modelId,0, hipFrmNo_);

	//アニメーション再生
	animationController_->Update();
	//controllerAnimation_->Update();
	
	//胸の位置更新
	hipMovedPos_ = MV1GetAttachAnimFrameLocalPosition(transform_.modelId, 0, hipFrmNo_);

	transform_.Update();

#ifdef _DEBUG

	UpdateDebugImGui();

#endif // _DEBUG
}

void Player::Draw(void)
{
	//モデルの描画
	MV1DrawModel(transform_.modelId);

	//丸影描画
	DrawShadow();
#ifdef _DEBUG

	DebugDraw();

#endif // _DEBUG
}

void Player::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

const Capsule& Player::GetCapsule(void) const
{
	return *capsule_;
}

const Sphere& Player::GetSphere(void) const
{
	return *sphere_;
}

bool Player::IsPlay(void) const
{
	//状態がPLAYかどうかを返す
	return state_ == STATE::PLAY;
}

void Player::InitAnimation(void)
{
	//アニメーションコントローラーの生成とアニメーションの登録
	const std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", ANIM_SPEED);
	animationController_->Add((int)ANIM_TYPE::WALK, path + "Walking.mv1", ANIM_SPEED);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Running.mv1", ANIM_SPEED);
	animationController_->Add((int)ANIM_TYPE::JUMP, path + "Jump.mv1", ANIM_SPEED);
	animationController_->Add((int)ANIM_TYPE::DODGE, path + "Roll.mv1", 40.0f);
	//初期アニメーションはアイドルを再生
	animationController_->Play((int)ANIM_TYPE::IDLE);

	//ヒップの位置取得
	hipFrmNo_ = MV1SearchFrame(transform_.modelId, L"mixamorig:Hips");
	MV1SetAttachAnimTime(transform_.modelId, hipFrmNo_, 0.0f);
	prevPos_ = MV1GetAttachAnimFrameLocalPosition(transform_.modelId, 0, hipFrmNo_);
	//int IdleAnimHandle = MV1LoadModel(L"Data/Model/Player/Idle.mv1");
	//int WalkAnimHandle = MV1LoadModel(L"Data/Model/Player/Walking.mv1");
	//int RunAnimHandle = MV1LoadModel(L"Data/Model/Player/Running.mv1");
	//int JumpAnimHandle = MV1LoadModel(L"Data/Model/Player/Jump.mv1");
	//int DodgeAnimHandle = MV1LoadModel(L"Data/Model/Player/Rolling.mv1");
	//controllerAnimation_ = std::make_unique<ControllerAnimation>(transform_.modelId);
	//controllerAnimation_->Add("Idle", IdleAnimHandle, ANIM_SPEED);
	//controllerAnimation_->Add("Walk", WalkAnimHandle, ANIM_SPEED);
	//controllerAnimation_->Add("Run", RunAnimHandle, ANIM_SPEED);
	//controllerAnimation_->Add("Jump", JumpAnimHandle, ANIM_SPEED);
	//controllerAnimation_->Add("Dodge", DodgeAnimHandle, ANIM_SPEED);

	////初期アニメーションはアイドルを再生
	//controllerAnimation_->Play("Idle");
}

void Player::ChangeState(STATE state)
{
	//状態変更
	state_ = state;

	//各状態遷移の初期処理
	stateChanges_[state_]();

}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Player::UpdateDead, this);
}

void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{
	MV1SetMaterialDifColor(transform_.modelId, 0, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));

	//移動処理
	ProcessMove();

	//ジャンプ処理
	//ProcessJump();
	ProcessJumpTest();

	//回避処理
	ProcessDodge();

	//パリィ処理
	ProcessParry();

	//移動方向に応じた回転
	Rotate();

	//重力による移動量
	CalcGravityPow();

	//衝突判定
	Collision();

	//歩きエフェクト
	EffectFootSmoke();

	//重力方向に沿って回転させる
	transform_.quaRot = Quaternion::Quaternion();
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);
}

void Player::UpdateDead(void)
{
}

void Player::DrawShadow(void)
{
	int i, j;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;
	int ModelHandle;

	//ライティングを無効にする
	SetUseLighting(FALSE);

	//Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	//テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	//影を落とすモデルの数だけ繰り返し
	for (auto c : colliders_)
	{
		//チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル

		ModelHandle = c.lock()->modelId_;

		float PLAYER_SHADOW_HEIGHT = 700.0f;
		float PLAYER_SHADOW_SIZE = 50.0f;

		//プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(ModelHandle, -1, transform_.pos, VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)), PLAYER_SHADOW_SIZE);

		//頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		//球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			//ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			//ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			//ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			//ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			//影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		//検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}

	//ライティングを有効にする
	SetUseLighting(TRUE);

	//Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);
}

void Player::ProcessMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	double rotRad = 0;

	if (ins.IsInputTriggered("Reset"))
	{
		transform_.pos = { -60.0f, 0.0f, 30.0f };
	}

	//WASDで位置を変える
	VECTOR dir = CommonUtility::VECTOR_ZERO;
	movePow_ = CommonUtility::VECTOR_ZERO;
	bool isUp = false;
	if (ins.IsInputPressed("Up"))
	{
		isUp = true;
		dir = VAdd(dir, cameraRot.GetForward());
		rotRad = CommonUtility::Deg2RadD(0.0);
	}
	bool isLeft = false;
	if (ins.IsInputPressed("Left"))
	{
		isLeft = true;
		dir = VAdd(dir, cameraRot.GetLeft());
		rotRad = CommonUtility::Deg2RadD(270.0);
	}
	bool isDown = false;
	if (ins.IsInputPressed("Down"))
	{ 
		isDown = true;
		dir = VAdd(dir,cameraRot.GetBack());
		rotRad = CommonUtility::Deg2RadD(180.0);
	}
	bool isRight = false;
	if (ins.IsInputPressed("Right"))
	{
		isRight = true;
		dir = VAdd(dir, cameraRot.GetRight());
		rotRad = CommonUtility::Deg2RadD(90.0);
	}

	if(isUp && isLeft)
	{
		rotRad = CommonUtility::Deg2RadD(315.0);
	}
	else if(isUp && isRight)
	{
		rotRad = CommonUtility::Deg2RadD(45.0);
	}
	else if(isDown && isLeft)
	{
		rotRad = CommonUtility::Deg2RadD(225.0);
	}
	else if(isDown && isRight)
	{
		rotRad = CommonUtility::Deg2RadD(135.0);
	}

	if (!CommonUtility::EqualsVZero(dir))
	{
		dir = VNorm(dir);

		//カメラのY軸角度だけ取得（XZ平面の回転だけで十分）
		float camYRad = mainCamera->GetQuaRot().y;

		//回転行列を使って入力ベクトルを回す（XZ平面）
		float sinY = sinf(camYRad);
		float cosY = cosf(camYRad);
		VECTOR worldDir = VGet(0.0f, 0.0f, 0.0f);
		worldDir.x = dir.x * cosY - dir.z * sinY;
		worldDir.y = 0.0f;
		worldDir.z = dir.x * sinY + dir.z * cosY;

		//ジャンプ中に加速しないように
		if (!isJump_ && !isDodge_)
		{
			//移動速度の設定
			speed_ = ins.IsInputPressed("Dash") ? SPEED_RUN : SPEED_MOVE;
		}
		moveDir_ = worldDir;
		movePow_ = VScale(dir, speed_);
		//プレイヤーの向きを移動方向に合わせる
		//double goalRotRad = atan2(worldDir.x, worldDir.z); // ラジアン
		SetGoalRotate(rotRad);

		if (!isJump_ && IsEndLanding() && !isDodge_)
		{
			//アニメーション
			if (speed_ == SPEED_RUN)
			{
				animationController_->Play((int)ANIM_TYPE::RUN);
				//controllerAnimation_->Play("Run");
			}
			else
			{
				animationController_->Play((int)ANIM_TYPE::WALK);
				//controllerAnimation_->Play("Walk");
			}
		}
	}
	else
	{
		if (!isJump_ && IsEndLanding() && !isDodge_)
		{
			animationController_->Play((int)ANIM_TYPE::IDLE);
			//controllerAnimation_->Play("Idle");
		}
	}

}

void Player::ProcessJump(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputPressed("Jump");

	// ジャンプ
	if (isHit && (isJump_ || IsEndLanding()))
	{

		if (!isJump_)
		{
			//無理やりアニメーション
			animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
			animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);/*
			controllerAnimation_->Play("Jump", true, 13.0f, 25.0f);
			controllerAnimation_->SetEndLoop(23.0f, 25.0f, 5.0f);*/
		}

		isJump_ = true;

		// ジャンプの入力受付時間を減らす
		stepJump_ += SceneManager::GetInstance().GetDeltaTime();
		if (stepJump_ < 0.5f)
		{
			jumpPow_ = VScale(CommonUtility::DIR_U, 35.0f);
		}

	}

	// ボタンを離したらジャンプ力に加算しない
	if (!isHit)
	{
		stepJump_ = 0.5f;
	}

}

void Player::ProcessJumpTest(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Jump");

	//ジャンプ
	if (isHit && IsEndLanding())
	{
		isJump_ = true;
		//ジャンプの初速度を設定
		//ここでは、JUMP_POWを初速としてv0に相当する値を設定します
		jumpPow_.y = JUMP_POW;

		// ダッシュジャンプの飛距離を出すために、水平方向の移動速度を初速に加算
		//jumpPow_.x = movePow_.x;
		//jumpPow_.z = movePow_.z;

		// ダッシュジャンプの飛距離を出すために、水平方向の移動速度を初速に加算
		jumpPow_.x = movePow_.x * 0.01f;
		jumpPow_.z = movePow_.z * 0.01f;

		//無理やりアニメーション
		animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
		animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);

		////無理やりアニメーション
		//controllerAnimation_->Play("Jump", true, 13.0f, 25.0f);
		//controllerAnimation_->SetEndLoop(23.0f, 25.0f, 5.0f);
	}

}

void Player::ProcessDodge(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Dodge");
	// ルートフレーム（Hipフレーム）の現在の行列を取得
	MATRIX hipMatrix = MGetIdent();
	if (isHit && (isDodge_ || IsEndDodge()))
	{
		isDodge_ = true;
		animationController_->Play((int)ANIM_TYPE::DODGE,false);
		//controllerAnimation_->Play("Dodge", false);
		prevPos_ = MV1GetAttachAnimFrameLocalPosition(transform_.modelId, 0, hipFrmNo_);
	}

	if (!isDodge_)return;
	stepDodge_ += SceneManager::GetInstance().GetDeltaTime();

	VECTOR dodgeAnimMove = VSub(hipMovedPos_,prevPos_);
	float movePow = VSize(animationController_->GetMovePow());
	VECTOR moveDir = VScale(moveDir_, movePow);
	transform_.pos = VAdd(transform_.pos, moveDir);
	animMovePow_ = VAdd(transform_.pos, moveDir);
	prevPos_ = hipMovedPos_;

	////アニメーションが終了したら回避終了
	//if(controllerAnimation_->IsEnd())
	//{
	//	isDodge_ = false;
	//}

	//アニメーションが終了したら回避終了
	if(animationController_->IsEnd())
	{
		isDodge_ = false;
		stepDodge_ = 0.0f;
	}

	if(stepDodge_ > 0.2f && stepDodge_ < 1.5f)
	{
		isInvincible_ = true;
	}
	else
	{
		isInvincible_ = false;
	}

	if(isInvincible_)MV1SetMaterialDifColor(transform_.modelId, 0, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
}

void Player::ProcessParry(void)
{
	InputManager& ins = InputManager::GetInstance();
	bool isHit = ins.IsInputTriggered("Parry");
	if (isHit)
	{
		isParry_ = true;
	}

	if (!isParry_)return;
	stepParry_ += SceneManager::GetInstance().GetDeltaTime();
	col_ = 0xff0000;
	if(stepParry_ > 0.8f)
	{
		col_ = 0x000000;
		isParry_ = false;
		stepParry_ = 0.0f;
	}
}

void Player::SetGoalRotate(double rotRad)
{
	//目標回転にカメラのY軸角度を加算
	VECTOR cameraRot = mainCamera->GetAngles();
	Quaternion axis =
		Quaternion::AngleAxis(
			(double)cameraRot.y + rotRad, CommonUtility::AXIS_Y);
	
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

void Player::Rotate(void)
{
	//回転時間の減少
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();
	
	//回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

void Player::Collision(void)
{
	//現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);
	
	//衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();
	
	//移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;
}

void Player::CollisionCapsule(void)
{
	//カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	//カプセルとの衝突判定
	for (const auto c : colliders_)
	{
		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());
		//衝突した複数のポリゴンと衝突回避するまで、
		//プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];
			//地面と異なり、衝突回避位置が不明なため、何度か移動させる
			//この時、移動させる方向は、移動前座標に向いた方向であったり、
			//衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{
				//再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				//最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					//法線の方向にちょっとだけ移動させる
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 2.0f));
					//カプセルも一緒に移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;
				}
				break;
			}
		}
		//検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void Player::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = CommonUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = CommonUtility::DIR_U;

	// 重力の強さ
	float gravityPow = GRAVITY_POW;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = CommonUtility::VECTOR_ZERO;
			//jumpVelocity_ = CommonUtility::VECTOR_ZERO;
			//stepJump_ = 0.0f;
			if (isJump_)
			{
				// 着地モーション
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
				// 着地モーション
				//controllerAnimation_->Play(
				//	"Jump", false, 29.0f, 45.0f, false, true);
			}
			isJump_ = false;
		}

	}
}

void Player::CalcGravityPow(void)
{
	// ジャンプ中の場合のみ重力を適用
	if (isJump_)
	{
		// 重力による速度の減少
		// v = v0 + at の式に相当
		jumpPow_.y -= GRAVITY_POW * SceneManager::GetInstance().GetDeltaTime();
	}
	else
	{
		// 地面にいる場合はジャンプ力をリセット
		//jumpPow_ = CommonUtility::VECTOR_ZERO;

		// 重力方向
		VECTOR dirGravity = CommonUtility::DIR_D;

		// 重力の強さ
		float gravityPow = GRAVITY_POW;

		//重力
		VECTOR gravity = VScale(dirGravity, gravityPow);
		jumpPow_ = VAdd(jumpPow_, gravity);

		// 内積
		float dot = VDot(dirGravity, jumpPow_);
		if (dot >= 0.0f)
		{
			// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
			jumpPow_ = gravity;
		}
	}
}

bool Player::IsEndLanding(void) const
{
	bool ret = true;
	//無限ジャンプモードの場合は常にtrue
	if (isJumpUnlimited_)return ret;

	// アニメーションがジャンプではない
	//if (controllerAnimation_->GetPlayType() != "Jump")
	//{
	//	return ret;
	//}

	//// アニメーションが終了しているか
	//if (controllerAnimation_->IsEnd())
	//{
	//	return ret;	//終了している
	//}

	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;	//終了している
	}

	return false;
}

bool Player::IsEndDodge(void) const
{
	bool ret = true;
	// アニメーションが回避ではない
	//if (controllerAnimation_->GetPlayType() != "Dodge")
	//{
	//	return ret;
	//}

	//// アニメーションが終了しているか
	//if (controllerAnimation_->IsEnd())
	//{
	//	return ret;	//終了している
	//}
	// アニメーションが回避ではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::DODGE)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;	//終了している
	}

	return false;
}

void Player::EffectFootSmoke(void)
{
	stepFootSmoke_ -= SceneManager::GetInstance().GetDeltaTime();

	float len = CommonUtility::MagnitudeF(moveDiff_);

	if (len >= 1.0f &&
		stepFootSmoke_ < 0.0f)
	{

		stepFootSmoke_ = speed_ == SPEED_RUN ? 0.5f : TERM_FOOT_SMOKE;
		//stepFootSmoke_ = TERM_FOOT_SMOKE;

		//エフェクト再生
		effectSmokePlayId_ = PlayEffekseer3DEffect(effectSmokeResId_);

		//大きさ
		float SCALE = 5.0f;
		SetScalePlayingEffekseer3DEffect(effectSmokePlayId_, SCALE, SCALE, SCALE);

		//位置の設定
		SetPosPlayingEffekseer3DEffect(
			effectSmokePlayId_,
			transform_.pos.x,
			transform_.pos.y,
			transform_.pos.z);
	}
}

void Player::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Player");

	if (ImGui::Button("Normal Jump"))
	{
		isJumpUnlimited_ = false;
	}

	if (ImGui::Button("Unlimited Jump"))
	{
		isJumpUnlimited_ = true;
	}

	////位置
	//ImGui::Text("localF2TPos");
	////構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	//ImGui::InputFloat3("localF2TPos", &localF2TPos_.x);
	//ImGui::SliderFloat("localF2TPosX", &localF2TPos_.x, -800.0f, 1000.0f);
	//ImGui::SliderFloat("localF2TPosY", &localF2TPos_.y, -800.0f, 1000.0f);
	//ImGui::SliderFloat("localF2TPosZ", &localF2TPos_.z, -800.0f, 1000.0f);

	//終了処理
	ImGui::End();
}

void Player::DebugDraw(void)
{
	int lineH = 1;

	DebugDrawFormat::FormatString(L"HP : %.2f",
		hp_,
		lineH);
	DebugDrawFormat::FormatString(L"移動 : WASD",
		stepDodge_,
		lineH);
	DebugDrawFormat::FormatString(L"パリィ : SPACE",
		0,
		lineH);
	DebugDrawFormat::FormatString(L"回避 : LSHIFT",
		0,
		lineH);
	//DrawFormatString(0, 40, 0xffffff, L"pos : %.2f, %.2f, %.2f", transform_.pos.x,
	//	transform_.pos.y, transform_.pos.z);
	//DrawFormatString(0, 60, 0xffffff, L"jumpPow : %.2f, %.2f, %.2f", jumpPow_.x,
	//	jumpPow_.y, jumpPow_.z);
	//DrawFormatString(0, 80, 0xffffff, L"isDodge : %d", isDodge_);

	sphere_->Draw(col_);
}