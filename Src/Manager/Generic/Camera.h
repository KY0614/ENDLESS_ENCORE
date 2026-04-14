#pragma once
#include <DxLib.h>
#include "../../Common/Quaternion.h"
#include "../../Object/ActorBase.h"

class Sphere;
class Transform;

class Camera : public ActorBase
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

	//カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 90.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 90.0f * (DX_PI_F / 180.0f);

	//カメラモード
	enum class MODE
	{
		NONE,
		CRANE_UP,	//カメラを上昇させる(向き固定)
		TRACK,		//左右に移動させる(向き固定)
		DOLLY,		//被写体に対してカメラを前後移動させる
		ZOOM_OUT_DOLLY,	//ズームアウトしながらドリー移動
		FIXED_POINT,//固定カメラ
		FOLLOW,		//追従
		FREE,		//自由
		MOUSE,		//マウスで操作
		PAUSE,		//一時停止
	};

	//コンストラクタ
	Camera(void);
	//デストラクタ
	~Camera(void)override;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void)override;

	/// <summary>
	/// 状態ごとの描画前処理
	/// </summary>
	void SetBeforeDraw(void);

	/// <summary>
	/// 描画処理
	/// </summary>
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

	/// <summary>
	/// カメラがアクションを終了したかどうか
	/// </summary>
	/// <returns>true:終了,false:まだ終了していない</returns>
	const bool& IsActionEnd() const { return isActionEnd_; }

	//カメラモードの変更
	void ChangeMode(MODE mode);

	void SetTargetPos(const VECTOR& targetPos) { targetPos_ = targetPos; }

	/// <summary>
	/// 固定カメラの設定
	/// </summary>
	/// <param name="pos">固定位置</param>
	/// <param name="targetPos">カメラの注視座標</param>
	void SetFixedPointPos(const VECTOR& pos,const VECTOR& targetPos);

	/// <summary>
	/// クレーンアップカメラの設定
	/// </summary>
	/// <param name="startPos">移動開始地点</param>
	/// <param name="distance">移動距離</param>
	/// <param name="targetPos">カメラの注視座標</param>
	void SetCraneUpPos(
		const VECTOR& startPos,
		const float& distance,
		const VECTOR& targetPos,
		const float& craneUpSpeed);

	/// <summary>
	/// トラックカメラの設定（イージングQuadOut)
	/// </summary>
	/// <param name="startPos">移動開始地点</param>
	/// <param name="endPos">移動終了地点</param>
	/// <param name="totalMoveTime">移動にかかる総時間</param>
	void SetTrackQuadOut(
		const VECTOR& startPos,
		const VECTOR& endPos,
		const float& totalMoveTime = 1.0f);

	/// <summary>
	/// ドリーインカメラの設定
	/// </summary>
	/// <param name="startPos">移動開始地点</param>
	/// <param name="objectPos">被写体の座標</param>
	/// <param name="object2CameraDistance">カメラから被写体までの距離</param>
	/// <param name="totalMoveTime">移動にかかる総時間</param>
	void SetDollyQuadOut(
		const VECTOR& startPos,
		const VECTOR& endPos,
		const VECTOR& objectPos,
		const float& object2CameraDistance,
		const float& totalMoveTime = 1.0f);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="endFov">最終的な視野角(60より大きい)</param>
	/// <param name="startPos"></param>
	/// <param name="endPos"></param>
	/// <param name="targetPos"></param>
	/// <param name="totalMoveTime"></param>
	void SetZoomOutDolly(
		const float& endFov,
		const VECTOR& startPos,
		const VECTOR& endPos, 
		const VECTOR& targetPos,
		const float& totalMoveTime = 1.0f);

	//追従対象の設定
	void SetFollow(const Transform* follow);

	//注視対象の設定
	void SetTarget(const Transform* target);

	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui(void);

private:
	//カメラの当たり判定に用いる球
	std::unique_ptr<Sphere> sphere_;

	//カメラが追従対象とするTransform
	const Transform* followTransform_;
	//注視対象とするTransform
	const Transform* targetTransform_;

	//カメラモード
	MODE mode_;

	//カメラ角度(rad)
	VECTOR angles_;

	//X軸回転が無い角度
	Quaternion rotOutX_;

	//注視点
	VECTOR targetPos_;

	//固定カメラ用座標
	VECTOR fixedPointPos_;			//カメラ位置
	VECTOR fixedPointTargetPos_;	//注視点

	//クレーンアップ用
	VECTOR craneUpStartPos_;	//開始位置
	VECTOR craneUpTargetPos_;	//注視点
	float craneUpDistance_;		//移動距離
	float craneUpSpeed_;		//移動速度

	//トラックカメラ用
	VECTOR trackStartPos_;		//開始位置
	VECTOR trackEndPos_;		//終了位置
	VECTOR trackDir_;			//移動方向
	float trackTotalTime_;		//総移動時間
	float trackElapsedTime_;	//経過時間

	//ドリーカメラ用
	VECTOR dollyStartPos_;		//開始位置
	VECTOR dollyEndPos_;		//終了位置
	VECTOR dollyObjectPos_;		//被写体の座標
	float object2CameraDistance_;//カメラから被写体までの距離
	float dollyTotalTime_;		//総移動時間
	float dollyElapsedTime_;	//経過時間

	//ズームアウト用
	VECTOR zoomOutDollyStartPos_;	//開始位置
	VECTOR zoomOutDollyEndPos_;		//終了位置
	VECTOR zoomOutDollyTargetPos_;	//被写体の座標
	float zoomOutFov_;				//最終FOV
	float zoomOutDollyTotalTime_;	//総移動時間
	float zoomOutDollyElapsedTime_;	//経過時間

	//ロックオンしているかどうか true:ロックオン中
	bool isLockOn_;

	//カメラアクションが終了したかどうか true:終了
	bool isActionEnd_;

	//視野角
	float fov_;

	// 衝突時の押し戻し試行回数
	static constexpr int CNT_TRY_COLLISION_CAMERA = 30;
	// 衝突時の押し戻し量
	static constexpr float COLLISION_BACK_DIS = 2.0f;
	// 衝突判定用球体半径
	static constexpr float COL_CAPSULE_SPHERE = 50.0f;

	/// <summary>
	/// コライダー初期化
	/// </summary>
	void InitCollider(void);

	//カメラを初期位置に戻す
	void SetDefault(void);

	//追従対象との位置同期を取る
	void SyncFollow(void);

	//衝突処理
	void Collision(void);

	//カメラ操作
	void ProcessRot(void);	//回転処理
	void ProcessMove(void);	//移動処理
	void ProcessMouseMove(void);	//マウス移動処理

	//モード別更新ステップ
	void SetBeforeDrawCraneUp(void);
	void SetBeforeDrawTrack(void);
	void SetBeforeDrawDolly(void);
	void SetBeforeDrawZoomOutDolly(void);
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawTopFixed(void);
	void SetBeforeDrawFollow(void);
	void SetBeforeDrawFree(void);
	void SetBeforeDrawMouse(void);

};

