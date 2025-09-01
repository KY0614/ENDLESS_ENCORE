#pragma once
#include "../Manager/GameSystem/ScoreManager.h"
#include "../Common/Vector2.h"

class Score
{
public:
	//スコア加算スピード
	static constexpr int ADD_SCORE_SPEED = 8;		//基本加算スピード
	static constexpr int ADD_CURRENT_SPEED = 5;		//現在スコアの加算スピード
	static constexpr int ADD_TOTALSCORE_SPEED = 123;//総スコア加算スピード

	//イージング関連
	static constexpr float START_SLIDE_X = -500.0f;	//X開始位置
	static constexpr float END_SLIDE_X = (float)(Application::SCREEN_SIZE_X / 6);	//X終了位置
	static constexpr float START_SLIDE_Y = 690.0f;	//Y開始位置
	static constexpr float END_SLIDE_Y = 420.0f;	//Y終了位置
	static constexpr float NEXT_SLIDE_START_X = -125.0f;	//次のイージングを開始する目標位置
	static constexpr float SLIDE_MAX_TIME = 1.0f;	//目標時間
	static constexpr float BLINK_SPEED = 0.5f;		//ハイライト点滅の間隔

	//ゲージ関連
	static constexpr float MAX_GAUGE_TIME = 1.0f;		//ゲージのイージング目標時間
	static constexpr float FIRST_GAUGE_SPEED = 0.5f;	//最初に表示するゲージのスピード
	static constexpr float CURRENT_GAUGE_SPEED = 0.5f;	//最後に表示するゲージのスピード
	static constexpr int GAUGE_POS_X = Application::SCREEN_SIZE_X / 2 + 450;
	static constexpr int GAUGE_POS_Y = Application::SCREEN_SIZE_Y / 2 - 150;
	
	//ランク関連
	static constexpr int RANK_NUM = 4;		//ランクの数
	static constexpr int RANK_C_MAX = 500;	//Cランクの最大値
	static constexpr int RANK_B_MAX = 1000;	//Bランクの最大値
	static constexpr int RANK_A_MAX = 1500;	//Aランクの最大値
	static constexpr int RANK_S_MAX = 1999;	//Sランクの最大値

	static constexpr int RANK_C_MIN = 0;	//Cランクの最小値
	static constexpr int RANK_B_MIN = 501;	//Bランクの最小値
	static constexpr int RANK_A_MIN = 1001;	//Aランクの最小値
	static constexpr int RANK_S_MIN = 1501;	//Sランクの最小値

	static constexpr float RANK_SCORE_MARIGINE_X = 150.0f;	//ランキングスコアをラベルの大きさ分ずらす用
	static constexpr float RANK_SCORE_MARIGINE_Y = 120.0f;	//ランキング毎の縦間隔（描画する際にずらすため）
	static constexpr int RANK_SCORE_POS_Y = 100;				//ランキングY座標

	static constexpr int LOGO_HEIGHT = 1024;

	struct RankInfo 
	{
		int startVal_ = 0, endVal_ = 0;	//ランクごとの上限下限値
		float currentRate_ = 0.0f;		//
		float displayedRate_ = 0.0f;	//
		int gaugeImg_ = -1;		//ゲージ画像	
		bool isFull_ = false;
	};

	enum class RANK 
	{
		C,
		B,
		A,
		S,
		MAX
	};

	enum class TOTALSCR_PHASE 
	{
		COUNT_UP,       // カウントアップ中
		SHOW_CURRENT,   // 今回のスコアを別表示
		MOVE_TO_TOTAL,  // 移動中
		MERGE,          // 合体演出
		FINISH          // 終了
	};

	enum class STATE
	{
		PLAY_SCORE,
		TOTAL_SCORE
	};

	// コンストラクタ
	Score(void);

	// デストラクタ
	~Score(void);

	void Init(void);
	void Update(void);
	void Draw(void);

private:
	STATE state_;

	RANK rank_;
	TOTALSCR_PHASE phase_;

	std::map<STATE, std::function<void(void)>> stateChange_;

	std::function<void(void)> stateUpdate_;
	std::function<void(void)> stateDraw_;

	bool isCurrentScrDraw_;
	bool isRankingScrDraw_;
	bool isGaugeDraw_;
	bool playSE_;

	//今回のゲームのスコア
	int currentScr_;

	//全プレイヤーの総スコア
	int totalScr_;

	//ランキングスコアを滑らかに表示させるよう
	float slideX_[ScoreManager::RANKING_NUM];		//移動させる座標
	float slideXTime_[ScoreManager::RANKING_NUM];	//経過時間
	bool isMove_[ScoreManager::RANKING_NUM];		//移動中かどうか（ディレイをかけて表示する用）
	
	//ハイライト点滅表示用インデックス
	int highLightIdx_;	
	//ハイライト点滅を繰り返す用の時間
	float blinkTime_;

	//スコアによるランク表示用
	RankInfo rankData_[RANK_NUM];

	//ゲージを進ませる用の時間
	float gaugeTime_;

	//現在ゲージを進ませているランクのインデックス
	int currentRankIdx_;

	float slideY_;
	float slideYTime_;

	//ランキングのラベル画像（１位：～５位：）
	int* rankLabelImgs_;
	//ランキングの背景画像
	int rankingBackImg_;

	//数字用
	int* numberImgs_;
	//「現在スコア」ラベル画像
	int currentScrImg_;
	//ランクごとの文字画像（C,B,A,S)
	int* ranksImgs_;
	int circleShadowImg_;
	int decoImg_;

	int pushImg_;	//押下画像
	
	float scale_;

	void DrawVariableScore(int score,int posX,int posY, float scale);
	void DrawRankingScore(int score,int posX,int posY,int hightLight);
	void DrawScore(int score, int posX, int posY);

	void ChangeState(STATE state);
	void ChangePlayScore(void);
	void ChangeTotalScore(void);

	void UpdatePlayScore(void);
	void UpdateTotalScore(void);

	void DrawPlayScore(void);
	void DrawTotalScore(void);

	void CalcPercentFromRank(void);

	RANK GetRankFromScore(int score);

	void InitRankInfo(void);

	void LoadImages(void);
};

