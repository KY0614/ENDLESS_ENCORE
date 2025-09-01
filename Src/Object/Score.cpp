#include <DxLib.h>
#include <algorithm>
#include <cmath>
#include "../Application.h"
#include "../Manager/GameSystem/SoundManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/ResourceManager.h"
#include "../Common/Easing.h"
#include "Score.h"

Score::Score(void)  
{  
    isCurrentScrDraw_ = false;  
    isRankingScrDraw_ = false;  
    isGaugeDraw_ = false;  
    playSE_ = false;  
    currentScr_ = 0;  
    totalScr_ = 0;  
    highLightIdx_ = -1;  
    currentRankIdx_ = -1;  

    for (int i = 0; i < ScoreManager::RANKING_NUM; ++i)  
    {  
        slideX_[i] = 0.0f;  
        slideXTime_[i] = 0.0f;  
        isMove_[i] = false;  
    }  

    for (int i = 0; i < RANK_NUM; ++i)  
    {  
        rankData_[i] = {};  
    }  

    blinkTime_ = 0.0f;  
    gaugeTime_ = 0.0f;  
    slideY_ = 0.0f;  
    slideYTime_ = 0.0f;  

    circleShadowImg_ = -1;  
    currentScrImg_ = -1;
    rankingBackImg_ = -1; 
	pushImg_ = -1;
    ranksImgs_ = nullptr; 

    state_ = STATE::PLAY_SCORE;  
    rank_ = RANK::MAX;  
    phase_ = TOTALSCR_PHASE::COUNT_UP;  
    numberImgs_ = nullptr;  
	rankLabelImgs_ = nullptr;
    decoImg_ = 0;  
    scale_ = 0.0f;  

    stateChange_.emplace(STATE::PLAY_SCORE, std::bind(&Score::ChangePlayScore, this));  
    stateChange_.emplace(STATE::TOTAL_SCORE, std::bind(&Score::ChangeTotalScore, this));  
}

Score::~Score(void)
{
}

void Score::Init(void)
{
	auto& scr = ScoreManager::GetInstance();
	scr.LoadScore();

	auto& sound = SoundManager::GetInstance();
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::NORMAL,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::SCORE_NORMAL).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::NORMAL, 256 / 2);
	
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::GOOD,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::SCORE_GOOD).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::GOOD, 256 / 2);
	
	sound.Add(SoundManager::TYPE::SE, SoundManager::SOUND::GREATE,
		ResourceManager::GetInstance().Load(ResourceManager::SRC::SCORE_GREATE).handleId_);
	sound.AdjustVolume(SoundManager::SOUND::GREATE, 256 / 2);

	//ランクごとの情報初期化
	InitRankInfo();

	//画像読み込み
	LoadImages();

	//今回のスコアをランキングに照らし合わせてランクイン位置を初期化
	for (int i = 0; i < ScoreManager::RANKING_NUM; ++i)
	{
		slideX_[i] = START_SLIDE_X;

		if (scr.GetRankingScore(i) == scr.GetCurrentScore())
		{
			highLightIdx_ = i;
		}
	}

	slideY_ = START_SLIDE_Y;
	isMove_[0] = true;
	currentRankIdx_ = 0;
	rank_ = GetRankFromScore(scr.GetCurrentScore());

	//画面の大きさに合わせて拡大率を変える
	scale_ = static_cast<float>(Application::SCREEN_SIZE_Y) /
		static_cast<float>(Application::SCREEN_MAX_SIZE_Y);

	ChangeState(STATE::PLAY_SCORE);
}

void Score::Update(void)
{
	stateUpdate_();
}

void Score::Draw(void)
{
	stateDraw_();
}

void Score::ChangeState(STATE state)
{
	state_ = state;

	stateChange_[state_]();
}

void Score::ChangePlayScore(void)
{
	stateUpdate_ = std::bind(&Score::UpdatePlayScore, this);
	stateDraw_ = std::bind(&Score::DrawPlayScore, this);
}

void Score::ChangeTotalScore(void)
{
	stateUpdate_ = std::bind(&Score::UpdateTotalScore, this);
	stateDraw_ = std::bind(&Score::DrawTotalScore, this);
}

void Score::UpdatePlayScore(void)
{
	auto& sound = SoundManager::GetInstance();
	//ステート遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Interact"))
	{
		//ChangeState(STATE::TOTAL_SCORE);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	auto& scr = ScoreManager::GetInstance();
	//今回のスコアまで０から加算する
	if (currentScr_ >= scr.GetCurrentScore())
	{
		currentScr_ = scr.GetCurrentScore();
		isCurrentScrDraw_ = true;
	}
	else
	{
		currentScr_ += ADD_SCORE_SPEED;
	}

	CalcPercentFromRank();

	//イージングをかけて移動させる
	for (int i = 0; i < ScoreManager::RANKING_NUM; ++i)
	{
		if (!isMove_[i])break;
		//移動中
		slideXTime_[i] += SceneManager::GetInstance().GetDeltaTime() * 0.5f;
		slideX_[i] = Easing::CubicOut(
			slideXTime_[i], SLIDE_MAX_TIME, START_SLIDE_X, END_SLIDE_X);

		//移動座標が終了座標を超えそうになったら移動終了
		if (slideX_[i] > END_SLIDE_X)
		{
			slideX_[i] = END_SLIDE_X;
			slideXTime_[i] = SLIDE_MAX_TIME;
			if (i >= RANK_NUM - 1)isRankingScrDraw_ = true;
		}
	}
	//ディレイを付けるため、
	//終了座標より手前ぐらいのところで次を移動させる
	for (int i = 0; i < (ScoreManager::RANKING_NUM - 1); ++i)
	{
		if (!isMove_[i + 1] && slideX_[i] > NEXT_SLIDE_START_X)
		{
			isMove_[i + 1] = true;
			break;
		}
	}

	//今回のスコアがランクインしていたら点滅させる
	if (highLightIdx_ != -1)
	{
		blinkTime_ += SceneManager::GetInstance().GetDeltaTime() * BLINK_SPEED;
	}

	if (isRankingScrDraw_ && isCurrentScrDraw_ && isGaugeDraw_ && !playSE_)
	{
		if (rank_ == RANK::C)sound.Play(SoundManager::SOUND::NORMAL);
		if (rank_ == RANK::B)sound.Play(SoundManager::SOUND::GOOD);
		if (rank_ >= RANK::A)sound.Play(SoundManager::SOUND::GREATE);
		playSE_ = true;
	}
}

void Score::UpdateTotalScore(void)
{
	auto& scr = ScoreManager::GetInstance();

	switch (phase_)
	{
	case TOTALSCR_PHASE::COUNT_UP:
		//総スコアまで加算する（今回のスコアを引いた分）
		if (totalScr_ >= scr.GetAggregateScore() - currentScr_)
		{
			totalScr_ = scr.GetAggregateScore() - currentScr_;
			phase_ = TOTALSCR_PHASE::SHOW_CURRENT;
		}
		else
		{
			totalScr_ += ADD_TOTALSCORE_SPEED;
		}
		break;

	case TOTALSCR_PHASE::SHOW_CURRENT:
		//移動中
		slideYTime_ += SceneManager::GetInstance().GetDeltaTime() * 0.5f;
		slideY_ = Easing::CubicOut(
			slideYTime_, SLIDE_MAX_TIME, START_SLIDE_Y, END_SLIDE_Y);

		//移動座標が終了座標を超えそうになったら移動終了
		if (slideY_ < END_SLIDE_Y)
		{
			slideY_ = END_SLIDE_Y;
			slideYTime_ = 0.0f;
			phase_ = TOTALSCR_PHASE::MOVE_TO_TOTAL;
		}

		break;

	case TOTALSCR_PHASE::MOVE_TO_TOTAL:
		//移動中
		slideYTime_ += SceneManager::GetInstance().GetDeltaTime() * 0.5f;
		slideY_ = Easing::BackIn(
			slideYTime_, SLIDE_MAX_TIME, END_SLIDE_Y, 320.0f,0.0f);

		//移動座標が終了座標を超えそうになったら移動終了
		if (slideY_ < 320.0f)
		{
			slideY_ = 320.0f;
			slideYTime_ = 0.0f;
			phase_ = TOTALSCR_PHASE::MERGE;
		}
		break;

	case TOTALSCR_PHASE::MERGE:
		slideY_ = -100.0f;
		//今回のスコアを加算する
		if (totalScr_ >= scr.GetAggregateScore())
		{
			totalScr_ = scr.GetAggregateScore();
			phase_ = TOTALSCR_PHASE::FINISH;
		}
		else
		{
			totalScr_ += ADD_CURRENT_SPEED;
		}
		break;

	case TOTALSCR_PHASE::FINISH:
		// 入力受付 or 他の演出へ遷移
		break;
	}

	//シーン遷移 
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsInputTriggered("Interact"))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void Score::DrawPlayScore(void)
{
	auto& scr = ScoreManager::GetInstance();

	//ランキングの背景
	DrawRotaGraph3(
		Application::SCREEN_SIZE_X / 4,
		Application::SCREEN_SIZE_Y / 4 - 150,
		161,
		272,
		scale_ * 3.0f, scale_ * 2.2f,
		0.0f, rankingBackImg_,
		true);

	const int BANNER_SIZE = 500;
	const float scaleX = static_cast<float>(Application::SCREEN_SIZE_X) /
		static_cast<float>(Application::SCREEN_MAX_SIZE_X);
	//装飾
	DrawRotaGraph3(Application::SCREEN_SIZE_X - (BANNER_SIZE * scaleX), BANNER_SIZE * scale_,
		250,250,
		scaleX * 2.0f, scale_ * 2.0f,
		0.0f,
		decoImg_,
		true, false);

	//現在のスコア
	DrawVariableScore(currentScr_, 356,
		Application::SCREEN_SIZE_Y - 128, scale_);

	//「現在のスコア」ラベル
	DrawRotaGraph(306,
				Application::SCREEN_SIZE_Y - 128,
				scale_, 0.0f,
				currentScrImg_, true);

	//ゲージの背景
	DrawRotaGraph(GAUGE_POS_X,
		GAUGE_POS_Y,
		scale_ * 3.0f, 0.0f, circleShadowImg_,
		true, false);

	//ゲージ本体
	for (int i = 0; i < static_cast<int>(rank_) + 1; ++i)
	{
		// DrawCircleGaugeを使った描画
		DrawCircleGauge(
			GAUGE_POS_X,
			GAUGE_POS_Y,
			rankData_[i].displayedRate_ * 100.0f,
			rankData_[i].gaugeImg_,
			0.0f,
			3.0f,
			false,false
		);
	}

	if (rankData_[(int)rank_].isFull_)
	{
		switch (rank_)
		{
		case Score::RANK::C:
			DrawRotaGraph(GAUGE_POS_X,
				GAUGE_POS_Y,
				scale_ * 0.5f, 0.0f, ranksImgs_[0],
				true, false);
			break;

		case Score::RANK::B:
				DrawRotaGraph(GAUGE_POS_X,
					GAUGE_POS_Y,
					scale_ * 0.5f, 0.0f, ranksImgs_[1],
					true, false);
			break;

		case Score::RANK::A:
			DrawRotaGraph(GAUGE_POS_X,
				GAUGE_POS_Y,
				scale_ * 0.5f, 0.0f, ranksImgs_[2],
				true, false);
			break;

		case Score::RANK::S:
			DrawRotaGraph(GAUGE_POS_X,
				GAUGE_POS_Y,
				scale_ * 0.5f, 0.0f, ranksImgs_[3],
				true, false);
			break;

		default:
			break;
		}

		int logoScl = (int)((float)(LOGO_HEIGHT / 2) * scale_);
		//pushspaceの画像
		DrawRotaGraph(
			Application::SCREEN_SIZE_X / 2,
			Application::SCREEN_SIZE_Y / 2 + logoScl,
			2.0f, 0.0, pushImg_, true);
	}

	for (int i = 0; i < ScoreManager::RANKING_NUM; ++i)
	{
		//点滅表示用フラグと色
		bool isBlink = (i == highLightIdx_) && (fmod(blinkTime_, 1.0f) < 0.5f);
		int col = isBlink ? 100 : 255;

		DrawRankingScore(scr.GetRankingScore(i),
			(slideX_[i] + (RANK_SCORE_MARIGINE_X * scale_)),
			RANK_SCORE_POS_Y + (RANK_SCORE_MARIGINE_Y * i * scale_), col);

		DrawRotaGraph(slideX_[i], RANK_SCORE_POS_Y + (RANK_SCORE_MARIGINE_Y * i * scale_),
			scale_ * 0.8f, 0.0f, rankLabelImgs_[i],
			true, false);
	}
}

void Score::DrawTotalScore(void)
{
#ifdef _DEBUG

	auto& scr = ScoreManager::GetInstance();

	int line = 1;	//行
	int lineHeight = 40;	//行

	SetFontSize(24);		
	DrawFormatString(Application::SCREEN_SIZE_X / 2 - 200,
			Application::SCREEN_SIZE_Y / 2,
			0xFFFFFF, L"全プレイヤーの総スコア : ￥ % d", totalScr_);
	
	if (phase_ >= TOTALSCR_PHASE::SHOW_CURRENT && phase_ < TOTALSCR_PHASE::MERGE)
	{
		DrawFormatString(Application::SCREEN_SIZE_X / 2 - 200, slideY_, 0xFFFFFF,
			L"今回のスコア : ￥%d", currentScr_);
	}

	SetFontSize(16);

#endif // _DEBUG

}

void Score::CalcPercentFromRank(void)
{
	auto& scr = ScoreManager::GetInstance();
	int score = scr.GetCurrentScore();
	//現在のランクをインデックスとして保持
	int rankIdx = static_cast<int>(rank_);

	//
	for (int i = 0; i <= currentRankIdx_; ++i)
	{
		auto& data = rankData_[i];
		int prevIdx = 0;
		if (i > 0)prevIdx = i - 1;
		//現在のランクより低いランクの処理
		if (i < rankIdx)
		{
			if (data.isFull_)continue;
			//達成済みなのでゲージは満タン
			data.currentRate_ = 1.0f;
			
			//前のランクゲージが既に満タンで今回も満タンの場合、
			//ゲージの始まりのほうの速度を早くしておきたい
			if (rankData_[prevIdx].isFull_)
			{
				//ゲージをイージングをかけて滑らかに増やしていく
				gaugeTime_ += SceneManager::GetInstance().GetDeltaTime();
				data.displayedRate_ = Easing::Linear(
					gaugeTime_, MAX_GAUGE_TIME,
					0.0f, data.currentRate_);
			}
			else 
			{
				//ゲージをイージングをかけて滑らかに増やしていく
				gaugeTime_ += SceneManager::GetInstance().GetDeltaTime() * FIRST_GAUGE_SPEED;
				//ゲージをイージングをかけて滑らかに増やしていく
				data.displayedRate_ = Easing::CubicIn(
					gaugeTime_, MAX_GAUGE_TIME,
					0.0f, data.currentRate_);
			}
			// イージング終了判定
			if (rankData_[i].displayedRate_ > rankData_[i].currentRate_)
			{
				data.displayedRate_ = data.currentRate_;
				currentRankIdx_++;		//次のランクへ
				gaugeTime_ = 0.0f;      //リセット
				data.isFull_ = true;
			}
		}//現在のランクの処理
		else if (i == currentRankIdx_)
		{
			if (data.isFull_)break;
			//スコアを現在のランクの範囲を比例計算する（後で100をかけてパーセントにする）
			rankData_[i].currentRate_ = (
				static_cast<float>(score - rankData_[rankIdx].startVal_) /
			static_cast<float>(rankData_[rankIdx].endVal_ - rankData_[rankIdx].startVal_));

			//ゲージをイージングをかけて滑らかに増やしていく
			gaugeTime_ += SceneManager::GetInstance().GetDeltaTime() * CURRENT_GAUGE_SPEED;
			data.displayedRate_ = Easing::CubicOut(
				gaugeTime_, MAX_GAUGE_TIME,
				0.0f, data.currentRate_);

			//イージング終了判定
			if (rankData_[i].displayedRate_ >= rankData_[i].currentRate_)
			{
				data.displayedRate_ = data.currentRate_;
				gaugeTime_ = 0.0f;      //リセット
				data.isFull_ = true;
				isGaugeDraw_ = true;
			}
		}
	}
}

Score::RANK Score::GetRankFromScore(int score)
{
	if (score <= RANK_C_MAX) return RANK::C;
	else if (score < RANK_A_MIN) return RANK::B;
	else if (score < RANK_S_MIN) return RANK::A;
	else return RANK::S;
}

void Score::InitRankInfo(void)
{
	//ランクごとに初期化
	rankData_[0].gaugeImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANK_C).handleId_;
	rankData_[0].startVal_ = 0;
	rankData_[0].endVal_ = RANK_C_MAX;
	rankData_[0].currentRate_ = 0.0f;
	rankData_[0].displayedRate_ = 0.0f;

	rankData_[1].gaugeImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANK_B).handleId_;
	rankData_[1].startVal_ = RANK_C_MAX + 1;
	rankData_[1].endVal_ = RANK_B_MAX;
	rankData_[1].currentRate_ = 0.0f;
	rankData_[1].displayedRate_ = 0.0f;

	rankData_[2].gaugeImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANK_A).handleId_;
	rankData_[2].startVal_ = RANK_B_MAX + 1;
	rankData_[2].endVal_ = RANK_A_MAX;
	rankData_[2].currentRate_ = 0.0f;
	rankData_[2].displayedRate_ = 0.0f;

	rankData_[3].gaugeImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANK_S).handleId_;
	rankData_[3].startVal_ = RANK_A_MAX + 1;
	rankData_[3].endVal_ = RANK_S_MAX;
	rankData_[3].currentRate_ = 0.0f;
	rankData_[3].displayedRate_ = 0.0f;
}

void Score::LoadImages(void)
{
	//円ゲージの背景画像読み込み
	circleShadowImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::UI_CIRCLESHADOW).handleId_;

	//スコア用数字画像読み込み
	numberImgs_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::SCORE_NUMBER).handleIds_;

	//ランキングラベル画像読み込み
	rankLabelImgs_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANKING_LABEL).handleIds_;

	//現在スコアラベル画像読み込み
	currentScrImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::CURRENT_SCORE).handleId_;

	//ランキングの背景画像読み込み
	rankingBackImg_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANKING_BACK).handleId_;

	//装飾画像読み込み
	decoImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PINK_BANNER).handleId_;

	//ランクごとの文字画像
	ranksImgs_ = ResourceManager::GetInstance().
		Load(ResourceManager::SRC::RANKS).handleIds_;

	//画像読み込み
	pushImg_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::PUSH_SPACE).handleId_;

}

void Score::DrawVariableScore(int score, int posX, int posY,float scale)
{
	std::string str = std::to_string(score);
	const int digitWidth = 128 * scale;
	const float strScale = scale;

	// 右寄せ：画面右端から totalWidth 分左へずらす
	int marigineX = posX + 256;

	for (int i = 0; i < str.size(); ++i)
	{
		char ch = str[i];
		if ('0' <= ch && ch <= '9')
		{
			int digit = ch - '0';
			DrawRotaGraph(
				marigineX + static_cast<int>(i * digitWidth * strScale), posY,
				strScale, 0.0f,
				numberImgs_[digit], true);
		}
	}
}

void Score::DrawRankingScore(int score, int posX, int posY, int hightLight)
{
	std::string str = std::to_string(score);
	const int digitWidth = 80;

	for (int i = 0; i < str.size(); ++i)
	{
		char ch = str[i];
		if ('0' <= ch && ch <= '9')
		{
			int digit = ch - '0';
			SetDrawBright(255, 255, hightLight);
			DrawRotaGraph(
				posX + static_cast<int>(i * digitWidth * scale_), posY,
				scale_ * 0.8f, 0.0f,
				numberImgs_[digit], true);
			SetDrawBright(255, 255, 255);
		}
	}
}

void Score::DrawScore(int score, int posX, int posY)
{
	std::string str = std::to_string(score);
	const int digitWidth = 70;
	for (int i = 0; i < str.size(); ++i)
	{
		char ch = str[i];
		if ('0' <= ch && ch <= '9')
		{
			int digit = ch - '0';
			DrawRotaGraph(
				50 + i * digitWidth, 50,
				scale_, 0.0f,
				numberImgs_[digit], true);
		}
	}
}