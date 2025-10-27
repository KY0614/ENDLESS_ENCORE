#include <math.h>
#include <DxLib.h>
#include <algorithm>
#include <EffekseerForDXLib.h>
#include "../../Application.h"
#include "../../Libs/ImGui/imgui.h"
#include "../../Common/Vector2.h"
#include "../../Utility/CommonUtility.h"
#include "../Generic/InputManager.h"
#include "../Generic/SceneManager.h"
#include "../../Object/Common/Transform.h"
#include "Camera.h"

namespace
{
	const float FPS_LIMIT_X_UP_RAD = -80.0f * (DX_PI_F / 180.0f);
	const float FPS_LIMIT_X_DW_RAD = 70.0f * (DX_PI_F / 180.0f);
}

Camera::Camera(void)
{
	angles_ = VECTOR();
	cameraUp_ = VECTOR();
	mode_ = MODE::NONE;
	pos_ = CommonUtility::VECTOR_ZERO;
	targetPos_ = CommonUtility::VECTOR_ZERO;
	followTransform_ = nullptr;
	cameraNear_ = 0.0f;
	cameraFar_ = 0.0f;
	localF2CPos_ = CommonUtility::VECTOR_ZERO;
	localF2TPos_ = CommonUtility::VECTOR_ZERO;

}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	//カメラの初期設定
	ChangeMode(MODE::FIXED_POINT);

	//クリップ距離の初期設定
	cameraNear_ = CAMERA_NEAR;
	cameraFar_ = CAMERA_FAR;
	localF2CPos_ = LOCAL_F2C_POS;
	localF2TPos_ = LOCAL_F2T_POS;
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{

	//クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(cameraNear_, cameraFar_);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;

	case Camera::MODE::TOP_FIXED:
		SetBeforeDrawTopFixed();
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

	//DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

	//UpdateDebugImGui();
}

void Camera::Draw(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
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
		break;
	case Camera::MODE::TOP_FIXED:
		//カメラの初期設定
		pos_ = FIXEDTOP_CAMERA_POS;
		//注視点
		targetPos_ = FIXEDTOP_CAMERA_RELATIVE_POS;
		break;	
	case Camera::MODE::FOLLOW:
		break;
	}
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

}

void Camera::SyncFollow(void)
{
	//追従先の位置
	VECTOR pos = followTransform_->pos;

	//追従先の向き
	Quaternion followRot = Quaternion::Quaternion();

	//注視点(通常重力でいうところのY値を追従対象と同じにする)
	VECTOR localPos = rotOutX_.PosAxis(localF2TPos_);
	targetPos_ = VAdd(pos, localPos);

	//カメラ位置
	localPos = rot_.PosAxis(localF2CPos_);
	pos_ = VAdd(pos, localPos);

	//正面から設定されたY軸分、回転させる
	rotOutX_ = followRot.Mult(Quaternion::AngleAxis(angles_.y, CommonUtility::AXIS_Y));

	//正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, CommonUtility::AXIS_X));

	rot_ = Quaternion::Slerp(rot_, rot_, 0.1f);

	//カメラの上方向
	cameraUp_ = followRot.GetUp();
}

void Camera::ProcessRot(void)
{
	InputManager& ins = InputManager::GetInstance();

	//回転軸と量を決める
	float rotPow = 1.0f * DX_PI_F / 180.0f;
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

	VECTOR dir = CommonUtility::VECTOR_ZERO;
	if (ins.IsInputPressed("CameraFront"))	pos_.z += 5.0f; targetPos_.z += 5.0f;
	if (ins.IsInputPressed("CameraBack"))	pos_.z -= 5.0f; targetPos_.z -= 5.0f;
	if (ins.IsInputPressed("CameraR"))		pos_.x += 5.0f;	targetPos_.x += 5.0f;
	if (ins.IsInputPressed("CameraL"))		pos_.x -= 5.0f;	targetPos_.x -= 5.0f;

	if (ins.IsInputPressed("CameraRise"))	pos_.y += 5.0f;	targetPos_.y += 5.0f;
	if (ins.IsInputPressed("CameraDescent"))pos_.y -= 5.0f;	targetPos_.y -= 5.0f;
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	//なにもしない
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
	if (isStop)return;
	//マウスカーソルを非表示にする
	SetMouseDispFlag(false);
	Vector2 mousePos = ins.GetMousePos();
	
	angles_.y += std::clamp((mousePos.x - Application::SCREEN_SIZE_X / 2 ), -120, 120) * 0.2f / GetFPS();
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

	SyncFollow();
}