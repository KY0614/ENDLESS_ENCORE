#pragma once
#include "ActorBase.h"

class ShotBase : public ActorBase
{
	public:
		ShotBase(void);
		~ShotBase(void);

		virtual void Init(void) = 0;
		virtual void Update(void) = 0;
		virtual void Draw(void) = 0;

protected:
	
};

