#pragma once

#include<DxLib.h>		//対応表のため
#include<unordered_map>		//対応表のため
#include<functional>		//対応表のため
#include<string>	//対応キーのため
#include<vector>	//デバイス追加のため

/// <summary>
/// 周辺機器種別
/// </summary>
enum class PeripheralType {
	keyboard,	//キーボード
	gamepad,	//ゲームパッド
	mouse,		//マウス
	x_analog,	//Xboxコントローラーのアナログ入力
	end			//for文で回すとき用に定義
};

/// <summary>
/// アナログ入力種別
/// </summary>
enum class AnalogInputType {
	none,	//押してない
	l_up,	//左スティックの上
	l_down,	//左スティックの下
	l_left,	//左スティックの左
	l_right,//左スティックの右
	r_up,	//右スティックの上
	r_down,	//右スティックの下
	r_left,	//右スティックの左
	r_right,//右スティックの右
	l_trigger,
	r_trigger
};

class KeyConfigScene;

class Input
{
	friend KeyConfigScene;	//privateもいじていいよ

public:
	//コンストラクタ
	Input();

	/// <summary>
	///	指定されたコードが押された瞬間か
	/// </summary>
	/// <param name="eventcode">対象のイベントコード</param>
	/// <returns>押した瞬間:true / false :押されていない</returns>
	bool IsTrigerred(const std::string& eventcode)const;

	/// <summary>
	/// 指定されたコードが押されている間か
	/// </summary>
	/// <param name="eventcode">対象のイベントコード</param>
	/// <returns>押されている間:true / false :押されていない</returns>
	bool IsPressed(const std::string& eventcode)const;

	void Update(void);//入力状態を更新する

private:

	struct InputState {
		PeripheralType type;	//周辺機器種別
		uint32_t code;			//入力コード(汎用)
	};

	using InputTable_t = std::unordered_map<std::string, std::vector<InputState>>;
	InputTable_t inputTable_;	//イベントと実際の入力の対応表
	InputTable_t tmpInputTable_;//一時的な入力テーブル

	using InputData_t = std::unordered_map<std::string, bool>;
	InputData_t currentInput_;	//そのイベントに対応するボタンが押されている状態か
	InputData_t lastInput_;		//そのイベントに対応するボタンが押されている状態か(直前)

	std::vector<std::string> inputlistForDisplay_;

	using AnalogInputTable_t = 
		std::unordered_map < AnalogInputType, std::function<bool(const XINPUT_STATE&)>>;

	AnalogInputTable_t analogInputTable_;


	/// <summary>
	/// 入力テーブルのセーブ
	/// </summary>
	void Save();

	void Load();

	void ResetTable();
};

