#include "ClearPlayer.h"

ClearPlayer::ClearPlayer(void)
{
}

ClearPlayer::~ClearPlayer(void)
{
}

void ClearPlayer::Init(void)
{
}

void ClearPlayer::Update(void)
{
}

void ClearPlayer::Draw(void)
{
}

void ClearPlayer::Init3DModel(void)
{
}

void ClearPlayer::InitAnimation(void)
{
}

void ClearPlayer::ChangeState(const STATE& state)
{
	//ó‘Ô•ÏX
	state_ = state;

	//Šeó‘Ô‘JˆÚ‚Ì‰Šúˆ—
	stateChanges_[state_]();
}