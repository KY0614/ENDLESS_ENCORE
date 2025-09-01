#include <DxLib.h>
#include <fstream>
#include"../Libs/nlohmann/json.hpp"
#include"../Application.h"
#include "ScoreManager.h"

ScoreManager* ScoreManager::instance_ = nullptr;

// 長いのでnamespaceの省略
using json = nlohmann::json;

void ScoreManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ScoreManager();
	}
	instance_->Init();
}

ScoreManager& ScoreManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		ScoreManager::CreateInstance();
	}
	return *instance_;
}

void ScoreManager::Init(void)
{
}

void ScoreManager::Destroy(void)
{
	delete instance_;
}

void ScoreManager::LoadScore(void)
{
	scores_.clear();
	std::ifstream ifs(Application::PATH_SCORE + "score.json");
	if (!ifs)return;//読み込み失敗

	// ファイルストリームからjsonオブジェクトに変換
	json scoreData = json::parse(ifs);

	//jsonオブジェクトから、scoreオブジェクトを取得
	const auto& scoreDatas = scoreData["Scores"];
	// scoreオブジェクトは複数あるはずなので、繰り返し処理
	for (const json& s : scoreDatas)
	{
		// 管理配列に追加
		scores_.emplace_back(s.get<int>());
	}

	//スコープが切れる際に 自動的にファイルクローズして貰えますが、
	//明示的にファイルストリームを閉じる
	ifs.close();

	UpdateRanking(); //ランキングも更新
}

void ScoreManager::SaveScore(int score)
{
	std::ifstream ifs(Application::PATH_SCORE + "score.json");
	json scoreData;
	if (ifs)
	{
		scoreData = json::parse(ifs);
		ifs.close();
	}

	if (!scoreData.contains("Scores")) {
		scoreData["Scores"] = json::array();
	}

	scoreData["Scores"].push_back(score);

	std::ofstream ofs(Application::PATH_SCORE + "score.json");
	ofs << scoreData.dump(4);
	ofs.close();
}

int ScoreManager::GetAggregateScore(void) const
{
	int total = 0;
	for (int s : scores_) {
		total += s;
	}
	return total;
}

void ScoreManager::UpdateRanking(void)
{
	//スコアをソートする
	std::vector<int> scoreList = scores_;
	std::sort(scoreList.begin(), scoreList.end(), std::greater<int>());

	//５位までのスコアを降順で入れる(スコアが５つ以上ない場合は０を入れる)
	for (int i = 0; i < RANKING_NUM && i < scoreList.size(); ++i) {
		scoreRank_[i] = scoreList[i];
	}
	for (int i = scoreList.size(); i < RANKING_NUM; ++i) {
		scoreRank_[i] = 0;
	}
}
