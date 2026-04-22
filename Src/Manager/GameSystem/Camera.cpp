#include <math.h>
#include <DxLib.h>
#include <algorithm>
#include <EffekseerForDXLib.h>
#include "../../Libs/ImGui/imgui.h"
#include "../../Application.h"
#include "../../Common/Vector2.h"
#include "../../Common/Easing.h"
#include "../../Utility/CommonUtility.h"
#include "../GameSystem/InputManager.h"
#include "../Generic/SceneManager.h"
#include "../../Object/Common/Transform.h"
#include "../Object/Common/Geometry/Sphere.h"
#include "Camera.h"

namespace
{
	//マウス操作用
	const float FPS_LIMIT_X_UP_RAD = -80.0f * (DX_PI_F / 180.0f);	//上限
	const float FPS_LIMIT_X_DW_RAD = 70.0f * (DX_PI_F / 180.0f);	//下限
	//視野角
	const float DEFAULT_CAMERA_FOV = 60.0f;
}

Camera::Camera(void)
{
	angles_ = CommonUtility::VECTOR_ZERO;
	transform_.quaRot.GetUp() = CommonUtility::VECTOR_ZERO;
	mode_ = MODE::NONE;
	transform_.pos = CommonUtility::VECTOR_ZERO;
	targetPos_ = CommonUtility::VECTOR_ZERO;
	fixedPointPos_ = CommonUtility::VECTOR_ZERO;
	fixedPointTargetPos_ = CommonUtility::VECTOR_ZERO;
	craneUpStartPos_ = CommonUtility::VECTOR_ZERO;
	craneUpTargetPos_ = CommonUtility::VECTOR_ZERO;
	craneUpDistance_ = 0.0f;
	craneUpSpeed_ = 0.0f;
	trackStartPos_ = CommonUtility::VECTOR_ZERO;
	trackEndPos_ = CommonUtility::VECTOR_ZERO;
	trackDir_ = CommonUtility::VECTOR_ZERO;
	trackTotalTime_ = 0.0f;
	trackElapsedTime_ = 0.0f;
	dollyStartPos_ = CommonUtility::VECTOR_ZERO;
	dollyEndPos_ = CommonUtility::VECTOR_ZERO;
	dollyObjectPos_ = CommonUtility::VECTOR_ZERO;
	object2CameraDistance_ = 0.0f;
	dollyTotalTime_ = 0.0f;
	dollyElapsedTime_ = 0.0f;
	followTransform_ = nullptr;
	targetTransform_ = nullptr;
	isLockOn_ = false;
	isActionEnd_ = false;
	fov_ = 0.0f;
	zoomOutDollyStartPos_ = CommonUtility::VECTOR_ZERO;
	zoomOutDollyEndPos_ = CommonUtility::VECTOR_ZERO;
	zoomOutDollyTargetPos_ = CommonUtility::VECTOR_ZERO;
	zoomOutFov_ = 0.0f;
	zoomOutDollyTotalTime_ = 0.0f;
	zoomOutDollyElapsedTime_ = 0.0f;
	backstabStartPos_ = CommonUtility::VECTOR_ZERO;
	backstabEndPos_ = CommonUtility::VECTOR_ZERO;
	backstabTargetPos_ = CommonUtility::VECTOR_ZERO;
	stepBackstab_ = 0.5f;
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	//カメラの初期設定
	ChangeMode(MODE::FIXED_POINT);

	//視野角の初期設定
	fov_ = DEFAULT_CAMERA_FOV;
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{
	//クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;

	case Camera::MODE::CRANE_UP:
		SetBeforeDrawCraneUp();
		break;

	case Camera::MODE::TRACK:
		SetBeforeDrawTrack();
		break;

	case Camera::MODE::DOLLY:
		SetBeforeDrawDolly();
		break;

	case Camera::MODE::ZOOM_OUT_DOLLY:
		SetBeforeDrawZoomOutDolly();
		break;

	case Camera::MODE::BACKSTAB:
		SetBeforeDrawBackstab();
		break;

	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;

	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;

	case Camera::MODE::MOUSE:
		SetBeforeDrawMouse();
		break;

	default:
		break;
	}
	//カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		transform_.pos, 
		targetPos_, 
		transform_.quaRot.GetUp()
	);

	//自由視点カメラの場合、角度からカメラを設定し直す
	if (mode_ == MODE::FREE)
	{
		SetCameraPositionAndAngle(
			transform_.pos,
			angles_.x,
			angles_.y,
			angles_.z
		);
	}

	//視野角の設定
	SetupCamera_Perspective(fov_ * DX_PI_F / 180.0f);

	//DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();
}

void Camera::Draw(void)
{
}

void Camera::SetBackstabCamera(const VECTOR& pos, const VECTOR& targetPos)
{
	backstabStartPos_ = transform_.pos;
	backstabEndPos_ = pos;
	backstabTargetPos_ = targetPos;
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

void Camera::SetTarget(const Transform* target)
{
	targetTransform_ = target;
}

VECTOR Camera::GetPos(void) const
{
	return transform_.pos;
}

VECTOR Camera::GetAngles(void) const
{
	return angles_;
}

VECTOR Camera::GetTargetPos(void) const
{
	return targetPos_;
}

Quaternion Camera::GetQuaRot(void) const
{
	return transform_.quaRot;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, transform_.pos));
}

void Camera::ChangeMode(MODE mode)
{
	//カメラの初期設定
	SetDefault();

	//カメラモードの変更
	mode_ = mode;

	//変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		transform_.pos = fixedPointPos_;
		targetPos_ = fixedPointTargetPos_;
		break;
	case Camera::MODE::CRANE_UP:
		transform_.pos = craneUpStartPos_;
		targetPos_ = craneUpTargetPos_;
		break;
	case Camera::MODE::TRACK:
		transform_.pos = trackStartPos_;
		break;
	case Camera::MODE::DOLLY:
		transform_.pos = dollyStartPos_;
		targetPos_ = dollyObjectPos_;
		break;	
	case Camera::MODE::ZOOM_OUT_DOLLY:
		transform_.pos = zoomOutDollyStartPos_;
		targetPos_ = zoomOutDollyTargetPos_;
		break;	
	case Camera::MODE::BACKSTAB:
		transform_.pos = backstabStartPos_;
		targetPos_ = backstabTargetPos_;
		break;	
	case Camera::MODE::FREE:
		break;
	}
}

void Camera::SetFixedPointPos(const VECTOR& pos, const VECTOR& targetPos)
{
	fixedPointPos_ = pos;
	fixedPointTargetPos_ = targetPos;
}

void Camera::SetCraneUpPos(
	const VECTOR& startPos,
	const float& distance,
	const VECTOR& targetPos,
	const float& craneUpSpeed)
{
	craneUpStartPos_ = startPos;
	craneUpDistance_ = distance;
	craneUpTargetPos_ = targetPos;
	craneUpSpeed_ = craneUpSpeed;
}

void Camera::SetTrackQuadOut(
	const VECTOR& startPos,
	const VECTOR& endPos,
	const float& totalMoveTime)
{
	trackStartPos_ = startPos;
	trackEndPos_ = endPos;
	trackDir_ = VNorm(VSub(endPos, startPos));
	trackTotalTime_ = totalMoveTime;
	trackElapsedTime_ = 0.0f;
}

void Camera::SetDollyQuadOut(
	const VECTOR& startPos,
	const VECTOR& endPos,
	const VECTOR& objectPos,
	const float& object2CameraDistance,
	const float& totalMoveTime)
{
	dollyStartPos_ = startPos;
	dollyEndPos_ = endPos;
	dollyObjectPos_ = objectPos;
	object2CameraDistance_ = object2CameraDistance;
	dollyTotalTime_ = totalMoveTime;
	dollyElapsedTime_ = 0.0f;
}

void Camera::SetZoomOutDolly(
	const float& endFov,
	const VECTOR& startPos,
	const VECTOR& endPos,
	const VECTOR& targetPos,
	const float& totalMoveTime)
{
	zoomOutFov_ = endFov;
	zoomOutDollyStartPos_ = startPos;
	zoomOutDollyEndPos_ = endPos;
	zoomOutDollyTargetPos_ = targetPos;
	zoomOutDollyTotalTime_ = totalMoveTime;
	zoomOutDollyElapsedTime_ = 0.0f;
}

void Camera::InitCollider(void)
{
	//球コライダ
	sphere_ = std::make_unique<Sphere>(transform_);
	sphere_->SetLocalPos(CommonUtility::VECTOR_ZERO);
	sphere_->SetRadius(COL_CAPSULE_SPHERE);
}

void Camera::SetDefault(void)
{
	//カメラの初期設定
	transform_.pos = DEFAULT_CAMERA_POS;

	//注視点
	targetPos_ = CommonUtility::VECTOR_ZERO;

	//カメラの上方向
	transform_.quaRot.GetUp() = CommonUtility::DIR_U;

	angles_.x = CommonUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	transform_.quaRot = Quaternion();
	isActionEnd_ = false;
}

void Camera::SyncFollow(void)
{
	//追従先の位置
	VECTOR pos = followTransform_->pos;

	//追従先の向き
	Quaternion followRot = Quaternion::Quaternion();

	//注視点(通常重力でいうところのY値を追従対象と同じにする)
	VECTOR localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	//カメラ位置
	localPos = transform_.quaRot.PosAxis(LOCAL_F2C_POS);
	transform_.pos = VAdd(pos, localPos);

	//正面から設定されたY軸分、回転させる
	rotOutX_ = followRot.Mult(Quaternion::AngleAxis(angles_.y, CommonUtility::AXIS_Y));

	//正面から設定されたX軸分、回転させる
	transform_.quaRot = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, CommonUtility::AXIS_X));
	const float rotTime = 0.1f;
	transform_.quaRot = Quaternion::Slerp(transform_.quaRot, transform_.quaRot, rotTime);

	//カメラの上方向
	transform_.quaRot.GetUp() = followRot.GetUp();
}

void Camera::Collision(void)
{
	//プレイヤーのルートフレーム
	VECTOR start = MV1GetFramePosition(followTransform_->modelId, 1);
	for (const auto& hitCol : colliders_)
	{
		//モデル以外は処理を飛ばす
		//if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;
		//派生クラスへキャスト
		//const ColliderModel* colliderModel =
		//	dynamic_cast<const ColliderModel*>(hitCol);
		if (hitCol.lock() == nullptr) continue;
		//線分で衝突判定
		auto hits = MV1CollCheck_LineDim(
			hitCol.lock()->modelId_,
			-1,
			transform_.pos,
			start
		);
		//追従対象に一番近い衝突点を探す
		bool isCollision = false;
		MV1_COLL_RESULT_POLY hitPoly;
		double minDist = DBL_MAX;
		for (int i = 0; i < hits.HitNum; i++)
		{
			const auto& hit = hits.Dim[i];
			//除外フレームは無視する
			//〇〇〇

			//衝突判定
			isCollision = true;
			//距離判定
			float dist = VSize(VSub(hit.HitPosition, start));
			if (dist < minDist)
			{
				// 追従対象に一番近い衝突点を優先
				minDist = dist;
				hitPoly = hit;
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
		if (!isCollision)
		{
			// 衝突していなければ次のコライダへ
			continue;
		}
		// カメラ位置から注視点への方向
		VECTOR dirToTarget = VNorm(VSub(start, transform_.pos));
		// 衝突点の少し手前にカメラを置く
		transform_.pos =
			VAdd(hitPoly.HitPosition, VScale(dirToTarget, COLLISION_BACK_DIS));

		// 球体コライダが無ければ処理を抜ける
		if (sphere_ == nullptr) continue;
		// 衝突補正処理
		int sphereCnt = 0;
		while (sphereCnt < CNT_TRY_COLLISION_CAMERA)
		{
			// 球体と三角形の当たり判定
			int isHitSphere = HitCheck_Sphere_Triangle(
				transform_.pos, sphere_->GetRadius(),
				hitPoly.Position[0], hitPoly.Position[1], hitPoly.Position[2]);
			// 衝突していたら法線方向に押し戻し
			transform_.pos = VAdd(transform_.pos, VScale(hitPoly.Normal, COLLISION_BACK_DIS));
			sphereCnt++;
		}
	}
}

void Camera::ProcessRot(void)
{
	InputManager& ins = InputManager::GetInstance();

	//回転軸と量を決める
	float rotPow = 1.5f * DX_PI_F / 180.0f;
	//回転処理
	if (ins.IsInputPressed("CameraUp")) { angles_.x += rotPow; }
	if (ins.IsInputPressed("CameraDown")) { angles_.x -= rotPow; }
	if (ins.IsInputPressed("CameraLeft")) { angles_.y -= rotPow; }
	if (ins.IsInputPressed("CameraRight")) { angles_.y += rotPow; }

	//x軸回転の制限（上は４０度、下は１５度）
	if (angles_.x > LIMIT_X_UP_RAD)
	{
		angles_.x = LIMIT_X_UP_RAD;
	}
	else if (angles_.x < -LIMIT_X_DW_RAD)
	{
		angles_.x = -LIMIT_X_DW_RAD;
	}
}

void Camera::ProcessMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	//移動速度
	const float moveSpeed = 10.0f;
	VECTOR dir = CommonUtility::VECTOR_ZERO;
	if (ins.IsInputPressed("CameraMoveUp"))	transform_.pos.z += moveSpeed; targetPos_.z += moveSpeed;
	if (ins.IsInputPressed("CameraMoveDown"))	transform_.pos.z -= moveSpeed; targetPos_.z -= moveSpeed;
	if (ins.IsInputPressed("CameraMoveRight"))	transform_.pos.x += moveSpeed; targetPos_.x += moveSpeed;
	if (ins.IsInputPressed("CameraMoveLeft"))	transform_.pos.x -= moveSpeed; targetPos_.x -= moveSpeed;

	if (ins.IsInputPressed("CameraRise"))	transform_.pos.y += moveSpeed;	targetPos_.y += moveSpeed;
	if (ins.IsInputPressed("CameraDescent"))transform_.pos.y -= moveSpeed;	targetPos_.y -= moveSpeed;
}

void Camera::ProcessMouseMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	//マウスカーソルを非表示にする
	SetMouseDispFlag(false);
	Vector2 mousePos = ins.GetMousePos();
	//
	angles_.y += std::clamp(
		(mousePos.x - Application::SCREEN_SIZE_X / 2), -120, 120) * 0.2f / GetFPS();
	angles_.x += std::clamp((
		mousePos.y - Application::SCREEN_SIZE_Y / 2), -120, 120) * 0.2f / GetFPS();

	// マウスの位置を画面中央に戻す
	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);

	if (angles_.x <= FPS_LIMIT_X_UP_RAD)
	{
		angles_.x = FPS_LIMIT_X_UP_RAD;
	}
	if (angles_.x >= FPS_LIMIT_X_DW_RAD)
	{
		angles_.x = FPS_LIMIT_X_DW_RAD;
	}
}

void Camera::SetBeforeDrawCraneUp(void)
{
	//スタート座標から現在座標までの距離を取得
	VECTOR endPos = VAdd(craneUpStartPos_, VScale(transform_.quaRot.GetUp(), craneUpDistance_));
	//現在地から目的地までの距離が一定以下になったら終了
	float pos2StartPos = VSize(VSub(endPos, transform_.pos));
	const float distance = 0.5f;
	isActionEnd_ = pos2StartPos <= distance;

	if (isActionEnd_) return;
	//カメラを上昇させる
	transform_.pos = VAdd(transform_.pos, VScale(transform_.quaRot.GetUp(), craneUpSpeed_));
}

void Camera::SetBeforeDrawTrack(void)
{
	//終了座標から現在座標までの距離を取得
	float pos2StartPos = VSize(VSub(trackEndPos_, transform_.pos));
	//一定距離以下になったら行動済みにする
	const float distance = 1.0f;
	isActionEnd_ = pos2StartPos <= distance;
	if (isActionEnd_)return;

	//経過時間
	trackElapsedTime_ += SceneManager::GetInstance().GetDeltaTime();
	//制限時間内に収める
	trackElapsedTime_ = std::clamp(trackElapsedTime_, 0.0f, trackTotalTime_);
	// 各軸ごとにQuadOutイージングで補間
	transform_.pos.x = Easing::QuadOut(
		trackElapsedTime_,trackTotalTime_, trackStartPos_.x, trackEndPos_.x);
	transform_.pos.y = Easing::QuadOut(
		trackElapsedTime_, trackTotalTime_, trackStartPos_.y, trackEndPos_.y);
	transform_.pos.z = Easing::QuadOut(
		trackElapsedTime_, trackTotalTime_, trackStartPos_.z, trackEndPos_.z);

	//注視座標はカメラの正面に設置
	const float lookDistance = 50.0f;
	//垂直ベクトルを計算して注視点を設定
	targetPos_ = VAdd(transform_.pos, VScale(
		VGet(-trackDir_.z, trackDir_.y, -trackDir_.x), lookDistance));
}

void Camera::SetBeforeDrawDolly(void)
{
	//終了座標(目的位置)を計算
	//被写体から距離を取った位置を終了座標とする
	VECTOR endPos = VSub(
		dollyObjectPos_,
		VScale(VNorm(VSub(dollyObjectPos_, dollyStartPos_)), object2CameraDistance_));

	//終了座標から現在座標までの距離を取得
	float pos2StartPos = VSize(VSub(dollyEndPos_, transform_.pos));
	//一定距離以下になったら終了
	const float distance = 1.0f;
	isActionEnd_ = pos2StartPos <= distance;

	if (isActionEnd_)return;

	//経過時間
	dollyElapsedTime_ += SceneManager::GetInstance().GetDeltaTime();
	//制限時間内に収める
	dollyElapsedTime_ = std::clamp(dollyElapsedTime_, 0.0f, dollyTotalTime_);
	// 各軸ごとにQuadOutイージングで補間
	transform_.pos.x = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.x, dollyEndPos_.x);
	transform_.pos.y = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.y, dollyEndPos_.y);
	transform_.pos.z = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.z, dollyEndPos_.z);
}

void Camera::SetBeforeDrawZoomOutDolly(void)
{
	//終了座標から現在座標までの距離を取得
	float pos2StartPos = VSize(VSub(zoomOutDollyEndPos_, transform_.pos));
	//一定距離以下になったら終了
	const float distance = 1.0f;
	isActionEnd_ = pos2StartPos <= distance;

	if (isActionEnd_)return;

	//経過時間
	zoomOutDollyElapsedTime_ += SceneManager::GetInstance().GetDeltaTime();
	//制限時間内に収める
	zoomOutDollyElapsedTime_ = std::clamp(
		zoomOutDollyElapsedTime_, 0.0f, zoomOutDollyTotalTime_);

	//視野角の補間
	fov_ = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		fov_, zoomOutFov_);

	//各軸ごとにQuadOutイージングで補間
	transform_.pos.x = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.x, zoomOutDollyEndPos_.x);
	transform_.pos.y = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.y, zoomOutDollyEndPos_.y);
	transform_.pos.z = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.z, zoomOutDollyEndPos_.z);
}

void Camera::SetBeforeDrawBackstab(void)
{
	stepBackstab_ -= SceneManager::GetInstance().GetDeltaTime();
	float t = std::clamp(1.0f - (stepBackstab_ / 0.5f), 0.0f, 1.0f);
	transform_.pos = CommonUtility::Lerp(
		backstabStartPos_, backstabEndPos_, t);
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	//固定位置に設定
	transform_.pos = fixedPointPos_;
	targetPos_ = fixedPointTargetPos_;
}

void Camera::SetBeforeDrawFollow(void)
{
	//カメラ操作
	ProcessRot();

	//追従対象との相対位置を同期
	SyncFollow();

	//衝突判定
	Collision();
}

void Camera::SetBeforeDrawFree(void)
{
	//カメラ操作
	ProcessRot();
	//移動操作
	ProcessMove();	
}

void Camera::SetBeforeDrawMouse(void)
{
	InputManager& ins = InputManager::GetInstance();
	static bool isStop = false;
	
	if(SceneManager::GetInstance().GetSceneID() != SceneManager::SCENE_ID::GAME)
	{
		isStop = true;
		SetMouseDispFlag(true);
	}
	else isStop = false;

	if (isStop)return;

	if (ins.IsClickMouseLeft())
	{
		isLockOn_ = !isLockOn_;
	}
	if (isLockOn_ && targetTransform_ != nullptr)
	{
		targetPos_ = targetTransform_->pos;
		VECTOR lookDir = VSub(targetPos_, transform_.pos);
		angles_.y = atan2f(lookDir.x, lookDir.z); // Y軸回転 (方位角)
		angles_.x = -atan2f(lookDir.y, VSize(VGet(lookDir.x, 0, lookDir.z))); // X軸回転 (仰角)
	}

	//マウス操作
	ProcessMouseMove();
	//追従
	SyncFollow();
}

void Camera::UpdateImGui(void)
{
	//デバッグ用ImGui表示
	//注視座標
	ImGui::Text("targetPos: %.2f, %.2f, %.2f", targetPos_.x, targetPos_.y, targetPos_.z);
	//座標
	ImGui::Text("Pos: %.2f, %.2f, %.2f", transform_.pos.x, transform_.pos.y, transform_.pos.z);
	//視野角(数値入力)
	ImGui::InputFloat("Fov", &fov_);
	//視野角(スライダー)
	const float fovMin = 8.0f;
	const float fovMax = 170.0f;
	ImGui::SliderFloat("Fov_Slider", &fov_, fovMin, fovMax);
}