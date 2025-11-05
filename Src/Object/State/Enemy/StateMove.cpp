#include "../../../Manager/Generic/SceneManager.h"
#include "StateMove.h"

StateMove::StateMove(void)
{
}

StateMove::~StateMove(void)
{
}

void StateMove::StateInit(void)
{
}

void StateMove::StateUpdate(void)
{
	//stateStep_ += SceneManager::GetInstance().GetDeltaTime();
	//if (stateStep_ > MOVE_TIME)
	//{
	//	//プレイヤーとの距離を測り、一定以上離れていたら遠距離攻撃
	//	//それ以外は近距離攻撃
	//	if (CheckPlayerDistance() < ATTACK_NEAR_DISTANCE)
	//	{
	//		ChangeState(STATE::ATTACK_NEAR);
	//	}
	//	else
	//	{
	//		animationController_->Play((int)ANIM_TYPE::CAST_SPELL, false);
	//		std::vector<STATE> attackState = { STATE::SHOT_ONE, STATE::SHOT_ALL };
	//		// 乱数生成器の初期化
	//		std::random_device rd; // 非決定的な乱数生成器
	//		std::mt19937 engine(rd()); // メルセンヌ・ツイスタ法による乱数生成器
	//		std::shuffle(attackState.begin(), attackState.end(), engine);
	//		//遠距離攻撃
	//		const int bulletNum = 5;
	//		CreateBullet(bulletNum);
	//		ChangeState(attackState[0]);
	//		return;
	//	}
	//}

	////移動処理
	//Move();

	////すごく離れていたら追従状態に遷移
	//if (CheckPlayerDistance() > FOLLOW_DISTANCE)
	//{
	//	animationController_->Play((int)ANIM_TYPE::RUN);
	//	ChangeState(STATE::FOLLOW);
	//}
}

void StateMove::Move(void)
{
	//if (CheckPlayerDistance() > PLAYER_DISTANCE)
	//{
	//	//プレイヤーに近づく
	//	FollowPlayer(transform_.pos);
	//	ChangeState(STATE::FOLLOW);
	//}
	//else
	//{
	//	animationController_->Play((int)ANIM_TYPE::IDLE);
	//}

	//static float stepTime = 0.0f;
	//stepTime += SceneManager::GetInstance().GetDeltaTime();

	//if (stepTime > 2.0f)
	//{
	//	stepTime = 0.0f;
	//	moveDir_ = CommonUtility::DIR_R;
	//}

}