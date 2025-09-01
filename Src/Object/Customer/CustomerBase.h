#pragma once
#include <memory>
#include "../ActorBase.h"

class AnimationController;

class CustomerBase : public ActorBase
{
public:
	//お客の初期位置
	static constexpr VECTOR CUSTOMER_POS = {-139.0f, 0.0f, 271.0f};		//画面左のほう

	//アニメーションの速度
	static constexpr float IDLE_ANIM_SPEED = 20.0f;		//待機アニメーションの速度
	static constexpr float WALK_ANIM_SPEED = 30.0f;		//歩行アニメーションの速度

	//お客の種類（タイプ）
	enum class TYPE
	{
		NONE,	//なし
		HOT,	//ホットの見た目
		ICE,	//アイスの見た目
	};

	//お客の状態
	enum class STATE
	{
		IDLE,	//待機状態
		WALK,	//歩行状態
	};

	//お客の反応
	enum class REACTION
	{	
		NONE,	//なし
		GOOD,	//良い反応
		SOSO,	//普通の反応
		BAD,	//悪い反応
	};

	//コンストラクタ
	CustomerBase(void);

	//デストラクタ
	~CustomerBase(void) = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">設定する座標</param>
	virtual void Init(const VECTOR pos);

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update(void)override;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw(void)override = 0;

	//一定間隔だけ進む
	
	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="">右に一定間隔で移動する</param>
	void Move(void);

	//設定関数---------------------------------------------------

	/// <summary>
	/// 客の種類を設定する
	/// </summary>
	/// <param name="type">設定する種類</param>
	void SetType(const TYPE type) { type_ = type; }

	/// <summary>
	/// X座標を設定
	/// </summary>
	/// <param name="posX">X座標</param>
	void SetPosX(const float posX) { transform_.pos.x = posX; }

	/// <summary>
	/// お客の状態を設定する
	/// </summary>
	/// <param name="state">設定する状態</param>
	void SetState(const STATE state) { state_ = state; }

	/// <summary>
	/// お客の反応を設定する
	/// </summary>
	/// <param name="reaction">設定する反応</param>
	void SetReaction(const REACTION reaction) { reaction_ = reaction; }

	/// <summary>
	/// 目標の回転角度（ラジアン）を設定します。
	/// </summary>
	/// <param name="rotRad">設定する回転角度（ラジアン単位）</param>
	void SetGoalRotate(const double rotRad);

	/// <summary>
	/// お客を非表示にする
	/// </summary>
	/// <param name="">falseにして非表示</param>
	void IsNotVisible(void) { isVisible_ = false; }

	//--------------------------------------------------------------------------
	//取得関数------------------------------------------------------------------

	/// <summary>
	/// カウンター前の球体との当たり判定
	/// </summary>
	/// <returns>true:当たっている , false:当たっていない</returns>
	bool CollisionCounter(void);

	/// <summary>
	/// お客がカウンターの前にいるかどうかを確認する
	/// </summary>
	/// <param name=""></param>
	/// <returns>true：カウンターの位置を越えている false：越えていない</returns>
	bool CheckCounterToCustomer(void);

	/// <summary>
	/// お客の座標を取得する
	/// </summary>
	/// <returns>お客の座標</returns>
	VECTOR GetPos(void)const { return transform_.pos; }

protected:

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	//お客の種類
	TYPE type_;
	//お客の状態
	STATE state_;
	//お客の反応
	REACTION reaction_;

	//表示するかどうか
	bool isVisible_;

	/// <summary>
	/// パラメータを設定する
	/// </summary>
	/// <param name="">お客ごとのパラメーター</param>
	virtual void SetParam(void) = 0;

	/// <summary>
	/// アニメーションの初期化処理
	/// </summary>
	virtual void InitAnimation(void) = 0;

private:

	//お客のY軸の回転角度
	Quaternion customerRotY_;

	//回転角度の目標値
	Quaternion goalQuaRot_;

	//回転時間
	float stepRotTime_;

	/// <summary>
	/// Y軸を中心に回転する
	/// </summary>
	void RotateY(void);

	/// <summary>
	/// 状態ごとのアニメーション処理
	/// </summary>
	/// <param name="">アニメーション再生</param>
	void StateAnimation(void);
};

