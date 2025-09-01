//#pragma once

#include<vector>

class ScoreManager
{
public:

	static constexpr int RANKING_NUM = 5;

	//インスタンスの生成
	static void CreateInstance(void);

	//インスタンスの取得
	static ScoreManager& GetInstance(void);

	void Init(void);

	//リソースの破棄
	void Destroy(void);

	//
	void LoadScore(void);
	//void LoadCurrentScore(void);
	//void LoadAggScore(void);

	//ゲーム終了時のスコア保存用
	void SaveScore(int score);

	int GetCurrentScore(void)const { return currentScore_; }

	//全スコアを加算した結果を取得
	int GetAggregateScore(void)const;

	//ランキング順のスコアを取得
	int GetRankingScore(int element)const { return scoreRank_[element]; }

	void SetCurrentScore(int score) { currentScore_ = score; }

private:

	//シングルトン用インスタンス
	static ScoreManager* instance_;

	//スコア	
	std::vector<int> scores_;
	int currentScore_;		//今回のスコア
	int scoreRank_[6];	//ランキング形式のスコア

	void UpdateRanking(void);
};

