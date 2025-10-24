#pragma once
#include<vector>
#include<memory>
#include "Common/Transform.h"

class Geometry;

class ActorBase
{

public:

	//当たり判定情報
	struct ColliderParameter
	{
		std::unique_ptr<Geometry> geometry_;	//形状情報
		std::shared_ptr<Collider> collider_;	//全体の当たり判定情報
	};

	//コンストラクタ
	ActorBase(void);

	//デストラクタ
	virtual ~ActorBase(void);

	virtual void Init(void);
	virtual void Update(void);
	virtual void Draw(void);

	const Transform& GetTransform(void) const;

protected:
	//モデル制御の基本情報
	Transform transform_;

	/// <summary>
	/// 当たり判定作成(形状情報作成後)
	/// </summary>
	/// <param name="_tag">自身の当たり判定タグ</param>
	/// <param name="_Geometry">自身の形状情報</param>
	/// <param name="_notHitTags">衝突させないタグ</param>
	void MakeCollider(const std::set<Collider::TYPE> _tag, std::unique_ptr<Geometry> _geometry, const std::set<Collider::TYPE> _notHitTags = {});

};
