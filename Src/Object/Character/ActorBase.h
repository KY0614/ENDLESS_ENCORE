#pragma once
#include<vector>
#include<memory>
#include "../Common/Transform.h"

class Geometry;

class ActorBase
{

public:

	//コンストラクタ
	ActorBase(void);

	//デストラクタ
	virtual ~ActorBase(void);

	virtual void Init(void);
	virtual void Update(void);
	virtual void Draw(void);

	//モデルの基本情報を取得する
	const Transform& GetTransform(void) const;

	/// <summary>
	/// 指定されたフレーム名(ボーン)に対応する座標を取得する
	/// </summary>
	/// <param name="frameName">フレームの名前</param>
	/// <returns>フレームの座標</returns>
	const VECTOR GetFramePos(const std::wstring& frameName) const;

	/// <summary>
	/// 衝突判定に用いられるコライダーを追加する
	/// </summary>
	/// <param name="collider">コライダー情報</param>
	void AddCollider(std::weak_ptr<Collider> collider);

protected:
	//モデル制御の基本情報
	Transform transform_;

	//衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	//丸影
	int imgShadow_;

	/// <summary>
	/// 影の描画処理
	/// </summary>
	void DrawShadow(void);
};
