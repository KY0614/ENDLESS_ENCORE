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

	//追従位置からカメラ位置までの相対座標
	static constexpr VECTOR LOCAL_F2C_POS = { 0.0f, 90.0f, -340.0f };

	//追従位置から注視点までの相対座標
	static constexpr VECTOR LOCAL_F2T_POS = { 0.0f, -60.0f, 575.0f };

	//カメラ座標関連の定数---------------------------------------------------------------------

	static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 500.0f, -800.0f };			//カメラの初期座標
	static constexpr VECTOR FIXEDTOP_CAMERA_POS = { 20.0f, 600.0f, -360.0f };		//固定カメラの初期座標

	static constexpr VECTOR RELATIVE_C2T_POS = { 0.0f, 165.0f, 200.0f };			//カメラ位置から注視点までの相対座標
	static constexpr VECTOR FIXEDTOP_CAMERA_RELATIVE_POS = { 20.0f, 0.0f, 65.0f };	//固定カメラ位置から注視点までの相対座標

	static constexpr VECTOR RELATIVE_F2C_POS_FOLLOW = { 0.0f, 500.0f, -500.0f };	//追従対象からカメラ位置までの相対座標(完全追従)


	//カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 90.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 90.0f * (DX_PI_F / 180.0f);

	//カメラ揺らし関連の定数--------------------------------------------------------------------

	static constexpr float TIME_SHAKE = 0.5f;		//時間

	static constexpr float WIDTH_SHAKE = 5.0f;		//幅

	static constexpr float SPEED_SHAKE = 40.0f;		//スピード
	
	//カメラモード
	enum class MODE
	{
		NONE,
		CRANE_UP,	//カメラを上昇させる
		TRACK,		//移動させる(向き固定)
		FIXED_POINT,//固定カメラ
		TOP_FIXED,	//上部固定
		FOLLOW,		//追従
		FREE,		//自由
		MOUSE,		//マウスで操作
		PAUSE,		//一時停止
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

	void SetFixedPointPos(const VECTOR& pos,const VECTOR& targetPos);

	//カメラの上昇開始位置と移動距離の設定
	void SetCraneUpPos(const VECTOR& startPos, const float& distance,const VECTOR& targetPos);

	/// <summary>
	/// トラックカメラ(回転せずに直線移動するカメラ)の設定
	/// </summary>
	/// <param name="startPos">移動開始地点</param>
	/// <param name="endPos">移動終了地点</param>
	/// <param name="moveDir">移動方向</param>
	/// <param name="moveSpeed">移動速度</param>
	void SetTrackCamera(const VECTOR& startPos,
		const VECTOR& endPos,
		const float& moveSpeed);

	//追従対象の設定
	void SetFollow(const Transform* follow);

	//注視対象の設定
	void SetTarget(const Transform* target);

private:

	//カメラが追従対象とするTransform
	const Transform* followTransform_;
	//注視対象とするTransform
	const Transform* targetTransform_;

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

	VECTOR craneUpStartPos_;
	VECTOR craneUpTargetPos_;
	float craneUpDistance_;

	VECTOR trackStartPos_;
	VECTOR trackEndPos_;
	VECTOR trackDir_;
	float trackDistance_;
	float trackSpeed_;

	//ロックオンしているかどうか true:ロックオン中
	bool isLockOn_;

	//カメラを初期位置に戻す
	void SetDefault(void);

	//追従対象との位置同期を取る
	void SyncFollow(void);

	//カメラ操作
	void ProcessRot(void);
	void ProcessMove(void);
	void ProcessMouseMove(void);

	//モード別更新ステップ
	void SetBeforeDrawCraneUp(void);
	void SetBeforeDrawTrack(void);
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawTopFixed(void);
	void SetBeforeDrawFollow(void);
	void SetBeforeDrawFree(void);
	void SetBeforeDrawMouse(void);

	float cameraNear_;
	float cameraFar_;

	VECTOR localF2CPos_;
	VECTOR localF2TPos_;

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);
};

