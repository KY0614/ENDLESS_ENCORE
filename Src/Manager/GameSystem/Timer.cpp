#include "../Generic/ResourceManager.h"
#include "Timer.h"

Timer::Timer(int minute, int second)
{
	//タイマーの時間は
	minute_ = minute;
	second_ = second;
	cnt_ = 0;
	isEnd_ = false;

	pos_.x = (float)Application::SCREEN_SIZE_X / 2;
	pos_.y = 0.0f;
	pos_.z = 0.0f;

	NumberImgs_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TIMER_NUMBER).handleIds_;
	colonImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TIMER_COLON).handleId_;
	imgTimerBack_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::UI_BAR).handleId_;
}

Timer::~Timer(void)
{
}

void Timer::Update()
{
	cnt_++;
	//FPS参照のカウント更新
	//FPSと同じ値のときは一秒立ったということ(60で１秒）
	if (cnt_ >= TIME_MAX + 1) {
		cnt_ = 0;

		//時間制限を減らす
		ReduceTime();
	}
}

void Timer::Draw()
{
	////背景描画
	//DrawRotaGraph(
	//	Application::SCREEN_SIZE_X / 2,
	//	TIMER_BACK_SIZE_Y / 2,
	//	TIMER_BACK_RATE,
	//	0.0f,
	//	imgTimerBack_,
	//	true);

	int num[NUM_CNT] = {
		minute_ / 10,       // 分の十の位
		minute_ % 10,       // 分の一の位
		second_ / 10,       // 秒の十の位
		second_ % 10        // 秒の一の位
	};

	//タイマーのコロン
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		NUM_POS_Y,
		1.0f, 0.0f,
		colonImg_,
		true, false
	);

	//秒数の描画
	for (int i = 0; i < NUM_CNT; i++) {
		DrawRotaGraph(
			NUM_POS_X[i],
			NUM_POS_Y,
			NUM_RATE,
			0.0f,
			NumberImgs_[num[i]],
			true);
	}
}

void Timer::Reset()
{
	minute_ = 1;
	second_ = 0;
	cnt_ = 0;
	isEnd_ = false;
}

void Timer::ReduceTime(void)
{
	//秒数を減らす
	second_--;

	//秒数が０未満になった時
	if (second_ < TIME_MIN) {
		//分数を減らす
		minute_--;
		//秒数を５９秒に
		second_ = TIME_MAX;
	}

	//分数が０未満のとき終了状態に
	if (minute_ <= TIME_MIN && second_ <= TIME_MIN)
	{
		isEnd_ = true;
	}
}