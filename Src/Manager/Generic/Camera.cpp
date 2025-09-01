#include <math.h>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../../Libs/ImGui/imgui.h"
#include "../../Utility/AsoUtility.h"
#include "../Generic/InputManager.h"
#include "../../Object/Common/Transform.h"
#include "Camera.h"

Camera::Camera(void)
{
	angles_ = VECTOR();
	cameraUp_ = VECTOR();
	mode_ = MODE::NONE;
	pos_ = AsoUtility::VECTOR_ZERO;
	targetPos_ = AsoUtility::VECTOR_ZERO;
	followTransform_ = nullptr;
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{

	ChangeMode(MODE::FIXED_POINT);
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

	case Camera::MODE::TOP_FIXED:
		SetBeforeDrawTopFixed();
		break;

	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;

	case Camera::MODE::FREE:
		SetBeforeDrawFree();
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
	targetPos_ = AsoUtility::VECTOR_ZERO;

	//カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.x = AsoUtility::Deg2RadF(30.0f);
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
	VECTOR localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	//カメラ位置
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	//正面から設定されたY軸分、回転させる
	rotOutX_ = followRot.Mult(Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y));

	//正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

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

	VECTOR dir = AsoUtility::VECTOR_ZERO;
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

void Camera::UpdateDebugImGui(void)
{
	//ウィンドウタイトル&開始処理
	ImGui::Begin("Camera");

	//位置
	ImGui::Text("position");
	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("Pos", &pos_.x);
	ImGui::SliderFloat("PosX", &pos_.x, -800.0f, 1000.0f);
	ImGui::SliderFloat("PosY", &pos_.y, -800.0f, 1000.0f);
	ImGui::SliderFloat("PosZ", &pos_.z, -800.0f, 1000.0f);

	//位置
	ImGui::Text("target");
	//構造体の先頭ポインタを渡し、xyzと連続したメモリ配置へアクセス
	ImGui::InputFloat3("target", &targetPos_.x);
	ImGui::SliderFloat("targetX", &targetPos_.x, -800.0f, 1000.0f);
	ImGui::SliderFloat("targetY", &targetPos_.y, -800.0f, 1000.0f);
	ImGui::SliderFloat("targetZ", &targetPos_.z, -800.0f, 1000.0f);

	//終了処理
	ImGui::End();
}