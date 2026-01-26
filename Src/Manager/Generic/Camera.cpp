#include <math.h>
#include <DxLib.h>
#include <algorithm>
#include <EffekseerForDXLib.h>
#include "../../Libs/ImGui/imgui.h"
#include "../../Application.h"
#include "../../Common/Vector2.h"
#include "../../Common/Easing.h"
#include "../../Utility/CommonUtility.h"
#include "../Generic/InputManager.h"
#include "../Generic/SceneManager.h"
#include "../../Object/Common/Transform.h"
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
	cameraUp_ = CommonUtility::VECTOR_ZERO;
	mode_ = MODE::NONE;
	pos_ = CommonUtility::VECTOR_ZERO;
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
		pos_, 
		targetPos_, 
		cameraUp_
	);

	//自由視点カメラの場合、角度からカメラを設定し直す
	if (mode_ == MODE::FREE)
	{
		SetCameraPositionAndAngle(
			pos_,
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
	return pos_;
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
	return rot_;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, pos_));
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
		pos_ = fixedPointPos_;
		targetPos_ = fixedPointTargetPos_;
		break;
	case Camera::MODE::CRANE_UP:
		pos_ = craneUpStartPos_;
		targetPos_ = craneUpTargetPos_;
		break;
	case Camera::MODE::TRACK:
		pos_ = trackStartPos_;
		break;
	case Camera::MODE::DOLLY:
		pos_ = dollyStartPos_;
		targetPos_ = dollyObjectPos_;
		break;	
	case Camera::MODE::ZOOM_OUT_DOLLY:
		pos_ = zoomOutDollyStartPos_;
		targetPos_ = zoomOutDollyTargetPos_;
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

void Camera::SetDefault(void)
{
	//カメラの初期設定
	pos_ = DEFAULT_CAMERA_POS;

	//注視点
	targetPos_ = CommonUtility::VECTOR_ZERO;

	//カメラの上方向
	cameraUp_ = CommonUtility::DIR_U;

	angles_.x = CommonUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	rot_ = Quaternion();
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
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	//正面から設定されたY軸分、回転させる
	rotOutX_ = followRot.Mult(Quaternion::AngleAxis(angles_.y, CommonUtility::AXIS_Y));

	//正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, CommonUtility::AXIS_X));
	const float rotTime = 0.1f;
	rot_ = Quaternion::Slerp(rot_, rot_, rotTime);

	//カメラの上方向
	cameraUp_ = followRot.GetUp();
}

void Camera::ProcessRot(void)
{
	InputManager& ins = InputManager::GetInstance();

	//回転軸と量を決める
	float rotPow = 1.5f * DX_PI_F / 180.0f;
	//回転処理
	if (ins.IsInputPressed("CameraDown")) { angles_.x += rotPow; }
	if (ins.IsInputPressed("CameraUp")) { angles_.x -= rotPow; }
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
	const float moveSpeed = 10.0f;
	VECTOR dir = CommonUtility::VECTOR_ZERO;
	if (ins.IsInputPressed("CameraMoveUp"))	pos_.z += moveSpeed; targetPos_.z += moveSpeed;
	if (ins.IsInputPressed("CameraMoveDown"))	pos_.z -= moveSpeed; targetPos_.z -= moveSpeed;
	if (ins.IsInputPressed("CameraMoveRight"))	pos_.x += moveSpeed; targetPos_.x += moveSpeed;
	if (ins.IsInputPressed("CameraMoveLeft"))	pos_.x -= moveSpeed; targetPos_.x -= moveSpeed;

	if (ins.IsInputPressed("CameraRise"))	pos_.y += moveSpeed;	targetPos_.y += moveSpeed;
	if (ins.IsInputPressed("CameraDescent"))pos_.y -= moveSpeed;	targetPos_.y -= moveSpeed;
}

void Camera::ProcessMouseMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	//マウスカーソルを非表示にする
	SetMouseDispFlag(false);
	Vector2 mousePos = ins.GetMousePos();

	angles_.y += std::clamp((mousePos.x - Application::SCREEN_SIZE_X / 2), -120, 120) * 0.2f / GetFPS();
	angles_.x += std::clamp((mousePos.y - Application::SCREEN_SIZE_Y / 2), -120, 120) * 0.2f / GetFPS();

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
	VECTOR endPos = VAdd(craneUpStartPos_, VScale(cameraUp_, craneUpDistance_));
	//現在地から目的地までの距離が一定以下になったら終了
	float pos2StartPos = VSize(VSub(endPos, pos_));
	const float distance = 0.5f;
	isActionEnd_ = pos2StartPos <= distance;

	if (isActionEnd_) return;
	//カメラを上昇させる
	pos_ = VAdd(pos_, VScale(cameraUp_, craneUpSpeed_));
}

void Camera::SetBeforeDrawTrack(void)
{
	//終了座標から現在座標までの距離を取得
	float pos2StartPos = VSize(VSub(trackEndPos_, pos_));
	//一定距離以下になったら行動済みにする
	const float distance = 1.0f;
	isActionEnd_ = pos2StartPos <= distance;
	if (isActionEnd_)return;

	//経過時間
	trackElapsedTime_ += SceneManager::GetInstance().GetDeltaTime();
	//制限時間内に収める
	trackElapsedTime_ = std::clamp(trackElapsedTime_, 0.0f, trackTotalTime_);
	// 各軸ごとにQuadOutイージングで補間
	pos_.x = Easing::QuadOut(
		trackElapsedTime_,trackTotalTime_, trackStartPos_.x, trackEndPos_.x);
	pos_.y = Easing::QuadOut(
		trackElapsedTime_, trackTotalTime_, trackStartPos_.y, trackEndPos_.y);
	pos_.z = Easing::QuadOut(
		trackElapsedTime_, trackTotalTime_, trackStartPos_.z, trackEndPos_.z);

	//注視座標はカメラの正面に設置
	const float lookDistance = 50.0f;
	//垂直ベクトルを計算して注視点を設定
	targetPos_ = VAdd(pos_, VScale(
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
	float pos2StartPos = VSize(VSub(dollyEndPos_, pos_));
	//一定距離以下になったら終了
	const float distance = 1.0f;
	isActionEnd_ = pos2StartPos <= distance;

	if (isActionEnd_)return;

	//経過時間
	dollyElapsedTime_ += SceneManager::GetInstance().GetDeltaTime();
	//制限時間内に収める
	dollyElapsedTime_ = std::clamp(dollyElapsedTime_, 0.0f, dollyTotalTime_);
	// 各軸ごとにQuadOutイージングで補間
	pos_.x = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.x, dollyEndPos_.x);
	pos_.y = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.y, dollyEndPos_.y);
	pos_.z = Easing::QuadOut(
		dollyElapsedTime_, dollyTotalTime_, dollyStartPos_.z, dollyEndPos_.z);
}

void Camera::SetBeforeDrawZoomOutDolly(void)
{
	//終了座標から現在座標までの距離を取得
	float pos2StartPos = VSize(VSub(zoomOutDollyEndPos_, pos_));
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
	pos_.x = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.x, zoomOutDollyEndPos_.x);
	pos_.y = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.y, zoomOutDollyEndPos_.y);
	pos_.z = Easing::QuadOut(
		zoomOutDollyElapsedTime_, zoomOutDollyTotalTime_,
		zoomOutDollyStartPos_.z, zoomOutDollyEndPos_.z);
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	pos_ = fixedPointPos_;
	targetPos_ = fixedPointTargetPos_;
}

void Camera::SetBeforeDrawTopFixed(void)
{
}

void Camera::SetBeforeDrawFollow(void)
{
	//カメラ操作
	ProcessRot();

	//追従対象との相対位置を同期
	SyncFollow();
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
		VECTOR lookDir = VSub(targetPos_, pos_);
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
	ImGui::Text("targetPos: %.2f, %.2f, %.2f", targetPos_.x, targetPos_.y, targetPos_.z);
	ImGui::Text("Pos: %.2f, %.2f, %.2f", pos_.x, pos_.y, pos_.z);
	ImGui::InputFloat("Fov", &fov_);
	ImGui::SliderFloat("Fov_Slider", &fov_, 8.0f, 170.0);
}