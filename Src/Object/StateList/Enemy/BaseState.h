#pragma once
class BaseState
{
public:
	//èÛë‘
	enum class STATE
	{
		NONE,
		FOLLOW,
		MOVE,			//
		ATTACK_NEAR,	//
		SHOT_ONE,		//
		SHOT_ALL,		//
		ATTACK_CHARGE,	//
		BACKSTAB,			//
		DOWN,			//
		DEAD,			//
	};

	BaseState(void);
	~BaseState(void);

	virtual void StateInit(void) = 0;
	virtual void StateUpdate(void) = 0;

protected:

	void ChangeState(STATE state);

private:
	STATE state_;
};

