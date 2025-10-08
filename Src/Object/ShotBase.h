#pragma once
#include <vector>
#include "Common/Transform.h"

class ShotBase
{
public:
	ShotBase(void);
	~ShotBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	const Transform& GetTransform(void) const;

protected:

	//ƒ‚ƒfƒ‹§Œä‚ÌŠî–{î•ñ
	Transform transform_;

};

