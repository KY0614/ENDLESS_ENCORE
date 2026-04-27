#pragma once
#include <DxLib.h>

class Transform;

class ColliderBase
{
public:
	//形状
	enum class SHAPE
	{
		NONE,	//無効
		LINE,	//線分
		SPHERE,	//球
		CAPSULE,//カプセル
		MODEL,	//モデル
	};

	//衝突種別
	enum class TAG
	{
		STAGE,	//ステージ
		PLAYER,	//プレイヤー
		PLAYER_ATK,		//プレイヤーの攻撃
		PLAYER_PARRY,	//プレイヤーのパリィ		
		ENEMY,			//敵
		ENEMY_BULLET,	//敵の弾
	};

	//コンストラクタ
	ColliderBase(SHAPE shape, TAG tag, const Transform* follow);
	//デストラクタ
	virtual ~ColliderBase(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	/// <summary>
	/// 追従先の取得
	/// </summary>
	/// <returns>追従先のTransformポインタ</returns>
	const Transform* GetFollow(void) const { return follow_; };

	/// <summary>
	/// 追従先の設定
	/// </summary>
	/// <param name="follow">追従先のTransformポインタ</param>
	void SetFollow(Transform* follow);

	/// <summary>
	/// 形状を取得
	/// </summary>
	/// <returns>形状</returns>
	SHAPE GetShape(void) const { return shape_; }

	/// <summary>
	/// 衝突種別を取得
	/// </summary>
	/// <returns>衝突種別</returns>
	TAG GetTag(void) const { return tag_; }

protected:

	//デバッグ表示の色
	static constexpr int COLOR_VALID = 0xff0000;
	static constexpr int COLOR_INVALID = 0xaaaaaa;

	//形状
	SHAPE shape_;

	//衝突種別
	TAG tag_;

	//追従先
	const Transform* follow_;

	//有効フラグ
	bool isValid_;

	// ローカル座標をワールド座標に変換
	VECTOR GetRotPos(const VECTOR& localPos) const;

	// デバッグ用描画
	virtual void DrawDebug(int color) = 0;
};