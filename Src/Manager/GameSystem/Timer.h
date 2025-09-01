#pragma once
#include <DxLib.h>
#include "../../Application.h"

class Timer
{
public:
	//時間の上限・下限
	static constexpr int TIME_MAX = 59;
	static constexpr int TIME_MIN = 0;

	//タイマー背景サイズ
	static constexpr float TIMER_BACK_RATE = 0.8f;
	static constexpr int TIMER_BACK_SIZE_X = 298 * TIMER_BACK_RATE;
	static constexpr int TIMER_BACK_SIZE_Y = 168 * TIMER_BACK_RATE;

	//数字の間隔
	static constexpr int MARGIN_SIZE = 50;
	static constexpr int MARGIN_SIZE_COLON = 10; // コロン部分のマージン

	//数字大きさ
	static constexpr float NUM_RATE = 0.75f;

	//数字描画数
	static constexpr int NUM_CNT = 4;

	//数字の描画位置
	static constexpr int NUM_POS_X[NUM_CNT] = {
		Application::SCREEN_SIZE_X / 2 - (MARGIN_SIZE * 2) + MARGIN_SIZE_COLON,
		Application::SCREEN_SIZE_X / 2 - MARGIN_SIZE + MARGIN_SIZE_COLON,
		Application::SCREEN_SIZE_X / 2 + MARGIN_SIZE,
		Application::SCREEN_SIZE_X / 2 + (MARGIN_SIZE * 2),
	};
	static constexpr int NUM_POS_Y = 40;

	Timer(int minute,int second);
	~Timer(void);

	void Update();
	void Draw();
	void Reset();

	bool IsEnd(void) const { return isEnd_; }

	int GetSecond(void)const { return second_; }
	int GetMinute(void)const { return minute_; }

private:
	int minute_;	//分数
	int second_;	//秒数

	bool isEnd_;	//タイマー終了の知らせ

	int cnt_;	//カウンター

	VECTOR pos_;	//位置

	int imgTimerBack_;	//背景
	int colonImg_;		//コロン（：）
	int* NumberImgs_;	//数字画像

	void ReduceTime(void);
};

