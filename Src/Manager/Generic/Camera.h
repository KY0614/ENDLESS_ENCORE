#pragma once
#include <DxLib.h>
#include "../../Common/Quaternion.h"
class Transform;
class Planet;

class Camera
{

public:

	//カメラスピード(度)
	static constexpr float SPEED = 1.0f;

	//カメラクリップ：NEAR
	static constexpr float CAMERA_NEAR = 10.0f;

	//カメラクリップ：NEAR
	static constexpr float CAMERA_FAR = 30000.0f;

	//カメラの初期座標
	//static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 100.0f, -500.0f };

	//追従位置からカメラ位置までの相対座標
	static constexpr VECTOR LOCAL_F2C_POS = { 0.0f, 50.0f, -400.0f };

	//追従位置から注視点までの相対座標
	static constexpr VECTOR LOCAL_F2T_POS = { 0.0f, -500.0f, 500.0f };

	//カメラ座標関連の定数---------------------------------------------------------------------

	static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 500.0f, -800.0f };			//カメラの初期座標
	//static constexpr VECTOR FIXEDTOP_CAMERA_POS = { 20.0f, 500.0f, -370.0f };		//固定カメラの初期座標
	//static constexpr VECTOR FIXEDTOP_CAMERA_POS = { 20.0f, 570.0f, -325.0f };		//固定カメラの初期座標
	//static constexpr VECTOR FIXEDTOP_CAMERA_POS = { 20.0f, 450.0f, -530.0f };		//固定カメラの初期座標
	static constexpr VECTOR FIXEDTOP_CAMERA_POS = { 20.0f, 600.0f, -360.0f };		//固定カメラの初期座標

	static constexpr VECTOR RELATIVE_C2T_POS = { 0.0f, 165.0f, 200.0f };			//カメラ位置から注視点までの相対座標
	//static constexpr VECTOR FIXEDTOP_CAMERA_RELATIVE_POS = { 20.0f, 0.0f, 137.0f };	//固定カメラ位置から注視点までの相対座標
	//static constexpr VECTOR FIXEDTOP_CAMERA_RELATIVE_POS = { 20.0f, 0.0f, 15.0f };	//固定カメラ位置から注視点までの相対座標
	static constexpr VECTOR FIXEDTOP_CAMERA_RELATIVE_POS = { 20.0f, 0.0f, 65.0f };	//固定カメラ位置から注視点までの相対座標


	static constexpr VECTOR RELATIVE_F2C_POS_FOLLOW = { 0.0f, 500.0f, -500.0f };	//追従対象からカメラ位置までの相対座標(完全追従)


	//カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 90.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 90.0f * (DX_PI_F / 180.0f);
	
	//カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		TOP_FIXED,
		FOLLOW,
		FREE
	};

	Camera(void);
	~Camera(void);

	void Init(void);
	void Update(void);
	void SetBeforeDraw(void);
	void Draw(void);

	//カメラ位置
	VECTOR GetPos(void) const;
	//カメラの操作角度
	VECTOR GetAngles(void) const;
	//カメラの注視点
	VECTOR GetTargetPos(void) const;

	//カメラ角度
	Quaternion GetQuaRot(void) const;
	//X回転を抜いたカメラ角度
	Quaternion GetQuaRotOutX(void) const;
	//カメラの前方方向
	VECTOR GetForward(void) const;

	//カメラモードの変更
	void ChangeMode(MODE mode);

	//追従対象の設定
	void SetFollow(const Transform* follow);


private:

	//カメラが追従対象とするTransform
	const Transform* followTransform_;

	std::weak_ptr<Planet> planet_;

	//カメラモード
	MODE mode_;

	//カメラの位置
	VECTOR pos_;

	//カメラ角度(rad)
	VECTOR angles_;

	//X軸回転が無い角度
	Quaternion rotOutX_;

	//カメラ角度
	Quaternion rot_;

	//注視点
	VECTOR targetPos_;

	//カメラの上方向
	VECTOR cameraUp_;

	//カメラを初期位置に戻す
	void SetDefault(void);

	//追従対象との位置同期を取る
	void SyncFollow(void);

	//カメラ操作
	void ProcessRot(void);
	void ProcessMove(void);

	//モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawTopFixed(void);
	void SetBeforeDrawFollow(void);
	void SetBeforeDrawFree(void);

	void UpdateDebugImGui(void);
};

