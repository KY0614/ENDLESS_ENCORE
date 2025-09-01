#pragma once
#include "Common/Transform.h"

class SceneManager;

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


protected:

	//モデル制御の基本情報
	Transform transform_;

	Transform sphereTran_;
};
