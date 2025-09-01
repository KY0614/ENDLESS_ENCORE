#include<DxLib.h>
#include<cassert>
#include "SoundManager.h"

SoundManager* SoundManager::instance_ = nullptr;

void SoundManager::CreateInstance(void)
{
	if (instance_ == nullptr) {
		instance_ = new SoundManager();
	}
}

SoundManager& SoundManager::GetInstance(void)
{
	return *instance_;
}

/// <summary>
/// サウンドの追加
/// </summary>
/// <param name="_type">音の種類分け(SEかBGMか)</param>
/// <param name="_sound">具体的な用途</param>
/// <param name="_data">音のデータ</param>
void SoundManager::Add(const TYPE _type, const SOUND _sound, const int _data)
{
	//連想配列内にすでに要素が入っているかを検索
	//入っていたら処理終了
	if (sounds_.find(_sound) != sounds_.end())return;


	//再生するときデータの種類によって
	//ループ再生か単発かを判断する
	int mode = -1;
	if (_type == TYPE::BGM) mode = DX_PLAYTYPE_LOOP;
	else mode = DX_PLAYTYPE_BACK;

	//新規データのため情報を追加
	sounds_.emplace(_sound, SOUND_DATA{ _data,_type ,mode});
}

/// <summary>
/// 音声データ
/// </summary>
/// <param name="_sound">音声データ</param>
void SoundManager::Play(const SOUND _sound)
{
	//元データがないときは警告
	if (sounds_.find(_sound) == sounds_.end())assert("設定していない音声を再生しようとしています。");

	//再生処理
	PlaySoundMem(sounds_[_sound].data, sounds_[_sound].playMode);
}

///<summary>
///停止処理
///</summary>
///<param name="_sound">音声データ</param>
void SoundManager::Stop(const SOUND _sound)
{
	//元データがないときは警告
	if (sounds_.find(_sound) == sounds_.end())assert("設定していない音声を停止しようとしています。");
	StopSoundMem(sounds_[_sound].data);
}

void SoundManager::Release(void)
{
	//配列要素善削除
	sounds_.clear();
}

/// <summary>
/// 音量調節
/// </summary>
/// <param name="_sound">音声</param>
/// <param name="_persent">調整割合(0%～100%)</param>
void SoundManager::AdjustVolume(const SOUND _sound, const int _persent)
{
	//元データがないときは警告
	if (sounds_.find(_sound) == sounds_.end())assert("設定していない音声を設定しようとしています。");
	ChangeVolumeSoundMem(255 * _persent / 100, sounds_[_sound].data);
}

void SoundManager::Destroy(void)
{
	Release();
	delete instance_;
}
