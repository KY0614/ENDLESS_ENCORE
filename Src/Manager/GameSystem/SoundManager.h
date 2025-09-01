#pragma once
#include<unordered_map>


//Dxlib内に音声データを3D空間上に再生する処理がある
//簡単に言えば音声に距離をつけることが可能
//少しばかり複雑そうなので後日に回す


class SoundManager
{
public:
	//再生の種類分けするときに使う
	enum class TYPE {
		NONE,
		BGM,
		SE
	};

	enum class SOUND {
		//ここに使用する音楽や効果音などを羅列
		NONE,

		TITLE,
		PUSH_SPACE,

		TUTORIAL,
		NEXT_PAGE,
		RETURN_PAGE,
		NOT_PAGE,
		OPEN_DOOR,

		GAME,
		TIMER,
		TIMER_FAST,
		COUNT_DOWN,
		GAME_START,
		GAME_FINISH,
		PICK_UP,
		ADD_STOCK,
		PAYING,
		MENU_BACK,
		MENU_OPEN,
		MENU_CLOSE,
		PUT_ON,

		RESULT,
		NORMAL,
		GOOD,
		GREATE,
	};

	struct SOUND_DATA
	{
		int data;		//音声データ格納
		TYPE type;		//音声データの種類	※この要素は削除するかも
		int playMode;	//音声データの再生タイプ
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SoundManager& GetInstance(void);

	/// <summary>
	/// サウンドの追加
	/// </summary>
	/// <param name="_type">音の種類分け(SEかBGMか)</param>
	/// <param name="_sound">具体的な用途</param>
	/// <param name="_data">音のデータ</param>
	void Add(const TYPE _type, const SOUND _sound, const int _data);

	/// <summary>
	/// 音声データ
	/// </summary>
	/// <param name="_sound">音声データ</param>
	void Play(const SOUND _sound);

	/// <summary>
	/// 停止処理
	/// </summary>
	/// <param name="_sound">音声データ</param>
	void Stop(const SOUND _sound);

	//解放
	void Release(void);

	/// <summary>
	/// 音量調節
	/// </summary>
	/// <param name="_sound">音声</param>
	/// <param name="_persent">調整割合(0%～100%)</param>
	void AdjustVolume(const SOUND _sound, const int _persent);

	void Destroy(void);
private:
	//インスタンス用
	static SoundManager* instance_;

	//データ格納用
	std::unordered_map<SOUND, SOUND_DATA>sounds_;

	//コンストラクタ＆デストラクタ
	SoundManager() = default;
	~SoundManager() = default;
};

