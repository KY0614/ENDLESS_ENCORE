#pragma once
#include "EnemyStateBase.h"
class StateMove : public EnemyStateBase
{
public:
	StateMove(void);
	~StateMove(void);

	void StateInit(void) override;
	void StateUpdate(void) override;

private:

	void Move(void);
};

