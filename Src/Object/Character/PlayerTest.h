#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <DxLib.h>
#include "CharactorBase.h"

class ParryBar;
class HPBar;
class AnimationController;
class Collider;
class Capsule;
class Sphere;
class ModelRenderer;
class ModelMaterial;

class PlayerTest : public CharactorBase
{
public:

	//状態
	enum class STATE
	{
		NONE,
		WAKE_UP,	//起き上がる
		WAIT,		//待機
		PLAY,		//操作可能
		BACKSTAB,	//バックスタブ
		DEAD,		//死亡
	};

	//アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,		//待機
		WAKE_UP,	//起き上がる
		WALK,		//歩く
		RUN,		//走り
		JUMP,		//ジャンプ
		PARRY,		//パリィ
		DODGE,		//回避
		DEATH,		//死亡
		BACKSTAB,	//バックスタブ
		USE_ITEM,	//アイテム使用
	};

	//コンストラクタ
	PlayerTest(void);
	//デストラクタ
	~PlayerTest(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void);

	/// <summary>
	///	更新処理
	/// </summary>
	void UpdateState(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	/// <summary>
	/// バーUIの描画
	/// </summary>
	void DrawBarUI(void);

	/// <summary>
	/// YOU DIEDの描画
	/// </summary>
	void DrawDead(void);

	/// <summary>
	/// VICTORYの描画
	/// </summary>
	void DrawVictory(void);

	/// <summary>
	/// 文字列を描画
	/// </summary>
	/// <param name="img">描画する画像</param>
	void DrawResultImage(const int img);

	/// <summary>
	/// コライダーの削除
	/// </summary>
	void ClearCollider(void);

	/// <summary>
	/// 衝突用カプセルの取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Capsule& GetCapsule(void) const;

	/// <summary>
	/// 衝突用球体の取得
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	const Sphere& GetSphere(void) const;

	/// <summary>
	/// 状態がPLAYかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:状態がPLAYの場合　false:それ以外</returns>
	bool IsPlay(void) const;

	/// <summary>
	/// プレイヤーがアクションを終了したかどうか（エンカウント中のアニメーションや移動など)
	/// </summary>
	/// <returns>true:終了した　false:終了していない</returns>
	bool IsActionEnd(void)const { return isActionEnd_; }

	/// <summary>
	/// 状態を取得する
	/// </summary>
	/// <returns>現在の状態</returns>
	const STATE& GetState(void) const { return state_; }

	/// <summary>
	/// ダメージを与える
	/// </summary>
	/// <param name="subHp">ダメージ量</param>
	void Damage(float subHp);

	/// <summary>
	/// 回避中かどうかを取得する
	/// </summary>
	/// <returns>true:回避中　false:回避してない</returns>
	const bool& GetIsDodge(void)const { return isDodge_; }

	/// <summary>
	/// パリィ中かどうかを取得する
	/// </summary>
	/// <returns>true:パリィ中　false:パリィしてない</returns>
	const bool& GetIsParry(void)const { return isParry_; }

	/// <summary>
	/// 座標の設定
	/// </summary>
	/// <param name="pos">指定する座標</param>
	void SetPos(const VECTOR& pos) { transform_.pos = pos; }

	/// <summary>
	/// Z座標を設定
	/// </summary>
	/// <param name="pos">設定するZ座標</param>
	void SetPosZ(const float& pos) { transform_.pos.z = pos; }

	/// <summary>
	/// HPを設定
	/// </summary>
	/// <param name="hp">指定するHP</param>
	void SetHP(const float hp) { hp_ = hp; }

	/// <summary>
	/// バックスタブ用Y軸回転の設定
	/// </summary>
	/// <param name="rotY">設定するY軸回転値</param>
	void SetBackstabRotY(const Quaternion& rotY);

	/// <summary>
	/// 操作可能な状態へ
	/// </summary>
	void Play(void);

	/// <summary>
	/// 待機状態へ
	/// </summary>
	void Wait(void);

	/// <summary>
	/// 致命攻撃状態へ
	/// </summary>
	void Backstab(void);

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name=""></param>
	void UpdateImGui(void)override;

protected:

	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

	virtual void JumpAnimationPlay(void) override;

private:
	//UI
	std::unique_ptr<HPBar> hpBar_;	//HPバー
	std::unique_ptr<ParryBar> parryBar_;//パリィバー

	//マテリアル・レンダラー
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;

	//状態管理
	STATE state_;

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//HP
	float hp_;		//現在HP
	float maxHp_;	//最大HP

	//移動スピード
	float speed_;


	float stepWalk_;	//歩きモーション完了までの時間経過

	//回転
	Quaternion playerRotY_;		//Y軸回転
	Quaternion goalQuaRot_;		//目標回転
	float stepRotTime_;			//回転完了までの時間経過

	//衝突チェック	
	VECTOR gravHitPosDown_;	//重力方向の当たり判定位置
	VECTOR gravHitPosUp_;	//重力と逆方向の当たり判定位置

	//カプセル
	std::unique_ptr<Capsule> capsule_;
	//球体
	std::unique_ptr<Sphere> sphere_;

	//足煙エフェクト
	int effectSmokeResId_;	//エフェクトリソースID
	int effectSmokePlayId_;	//エフェクト再生ID
	float stepFootSmoke_;	//足煙エフェクト発生までの時間経過

	//パリィエフェクト
	int effectParryResId_;	//エフェクトリソースID
	int effectParryPlayId_;	//エフェクト再生ID

	//バックスタブ位置
	float stepBackstab_;	//バックスタブ完了までの時間経過

	//フレームごとの移動値
	VECTOR moveDiff_;

	//無限ジャンプ
	bool isJumpUnlimited_;

	//ジャンプの入力受付時間
	float stepJump_;

	//回避判定
	bool isDodge_;
	float stepDodge_;
	bool isDecelerate_;	//減速中かどうか

	//パリィ判定
	bool isParry_;
	float stepParry_;

	//結果表示用文字列のアルファ値
	int resultImgAlpha_;

	//行動終了判定
	bool isActionEnd_;

	//布擦れSE再生フラグ
	bool clothSE_;

	//結果表示用画像ハンドル
	int victoryImg_;	//勝利画像
	int diedImg_;		//敗北画像

	/// <summary>
	/// サウンド初期化
	/// </summary>
	void InitSound(void);

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// 当たり判定の初期化
	/// </summary>
	void InitCollider(void);

	/// <summary>
	/// アニメーション初期化
	/// </summary>
	void InitAnimation(void);

	/// <summary>
	/// UI初期化
	/// </summary>
	/// <param name=""></param>
	void InitUI(void);

	/// <summary>
	/// 戦闘開始時の初期化
	/// </summary>
	/// <param name=""></param>
	void InitBattle(void);

	/// <summary>
	/// 最大HPを設定
	/// </summary>
	/// <param name="maxHp">最大HP</param>
	void SetMaxHP(const float maxHp) { maxHp_ = maxHp; }

	//状態遷移処理--------------------------------------------------------

	/// <summary>
	/// 状態を変更する
	/// </summary>
	/// <param name="state">変更する状態</param>
	void ChangeState(const STATE& state);

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);
	/// <summary>
	/// 状態遷移：WAKE_UP
	/// </summary>
	void ChangeStateWakeUp(void);
	/// <summary>
	/// 状態遷移：WAIT
	/// </summary>
	void ChangeStateWait(void);
	/// <summary>
	/// 状態遷移：PLAY
	/// </summary>
	void ChangeStatePlay(void);
	/// <summary>
	/// 状態遷移：BACKSTAB
	/// </summary>
	void ChangeStateBackstab(void);
	/// <summary>
	/// 状態遷移：DEAD
	/// </summary>
	void ChangeStateDead(void);

	//状態更新処理------------------------------------------------------

	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void);
	/// <summary>
	/// 更新：WAKE_UP
	/// </summary>
	void UpdateWakeUp(void);
	/// <summary>
	/// 状態更新：WAIT
	/// </summary>
	void UpdateWait(void);
	/// <summary>
	/// 更新：PLAY
	/// </summary>
	void UpdatePlay(void);
	/// <summary>
	/// 更新：BACKSTAB
	/// </summary>
	void UpdateBackstab(void);
	/// <summary>
	/// 更新：DEAD
	/// </summary>
	void UpdateDead(void);

	//移動------------------------------------------------------------

	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="">WASDで移動する処理</param>
	void ProcessMove(void);

	//ジャンプ--------------------------------------------------------

	/// <summary>
	/// ジャンプ処理
	/// </summary>
	void ProcessJump(void);

	//回避------------------------------------------------------------

	/// <summary>
	/// 回避処理
	/// </summary>
	/// <param name=""></param>
	void ProcessDodge(void);

	//パリィ------------------------------------------------------------

	/// <summary>
	/// パリィ処理
	/// </summary>
	/// <param name=""></param>
	void ProcessParry(void);

	//モデルの回転----------------------------------------------------

	/// <summary>
	/// 目標回転角度の設定
	/// </summary>
	/// <param name="rotRad"></param>
	void SetGoalRotate(double rotRad);

	/// <summary>
	/// 回転処理
	/// </summary>
	void Rotate(void);

	//衝突判定---------------------------------------

	// 衝突判定
	void CollisionReserve(void) override;

	//------------------------------------------------

	/// <summary>
	/// 着地モーションが終了したかどうか
	/// </summary>
	/// <returns>true: 着地モーションが終了した場合　false:それ以外</returns>
	bool IsEndLanding(void) const;

	/// <summary>
	/// 回避モーションが終了したかどうか
	/// </summary>
	/// <param name=""></param>
	/// <returns>true:回避モーションが終了した場合　false:それ以外</returns>
	bool IsEndDodge(void) const;

	/// <summary>
	/// 足煙エフェクトの発生処理
	/// </summary>
	void EffectFootSmoke(void);

	/// <summary>
	/// パリィエフェクトの発生処理
	/// </summary>
	void EffectParry(void);

	/// <summary>
	/// パリィエフェクト位置の更新
	/// </summary>
	/// <param name=""></param>
	void EffectParryPosUpdate(void);
};