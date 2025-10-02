#include "../Manager/Generic/InputManager.h"
#include "../Manager/Generic/SceneManager.h"
#include "AdvertiseScene.h"

AdvertiseScene::AdvertiseScene(void)
{
	movieHandle_ = 0;
	movieFrame_ = 0;
	totalMovieFrame_ = 0;
}

AdvertiseScene::~AdvertiseScene(void)
{
	DeleteGraph(movieHandle_);
}

void AdvertiseScene::Init(void)
{
	//ムービーをロードして再生する
	movieHandle_ = LoadGraph(L"Data/Movie/advertise.mp4");
	PlayMovieToGraph(movieHandle_);
	//ムービーの総フレーム数を取得する
	totalMovieFrame_ = GetMovieTotalFrameToGraph(movieHandle_);
}

void AdvertiseScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	//入力があったらタイトルシーンへ
	if(ins.IsInputTriggered("Dadge"))
	{
		SceneManager::GetInstance().ChangeScene(
			SceneManager::SCENE_ID::TITLE);
		return;	//シーン切り替えされたら以降の処理は行わない
	}

	//ムービーが最後まで再生されたらタイトルシーンへ
	if(TellMovieToGraph(movieHandle_) >= totalMovieFrame_ - 1)
	{
		SceneManager::GetInstance().ChangeScene(
			SceneManager::SCENE_ID::TITLE);
		return;
	}
}

void AdvertiseScene::Draw(void)
{
	DrawString(0, 0, L"AdvertiseScene", 0xffffffff);

	DrawGraph(0, 0, movieHandle_, true);
}
