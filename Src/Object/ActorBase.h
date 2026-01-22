#pragma once
#include<vector>
#include<memory>
#include "Common/Transform.h"

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

	const Transform& GetTransform(void) const;

	const VECTOR& GetFramePos(const std::wstring& frameName) const;

	/// <summary>
	/// 衝突判定に用いられるコライダーを追加する
	/// </summary>
	/// <param name="collider"></param>
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
