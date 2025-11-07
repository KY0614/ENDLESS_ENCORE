#pragma once
#include "Common/Transform.h"

class Stage
{
public:
	Stage(void);
	//デストラクタ
	~Stage(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	const Transform& GetTransform() const { return transform_; }

private:
	Transform transform_;
};

