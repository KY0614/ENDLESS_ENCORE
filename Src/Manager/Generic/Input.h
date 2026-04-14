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
	KEYBOARD,	//キーボード
	GAMEPAD,	//ゲームパッド
	MOUSE,		//マウス
	X_ANALOG,	//Xboxコントローラーのアナログ入力
	END			//for文で回すとき用に定義
};

/// <summary>
/// アナログ入力種別
/// </summary>
enum class AnalogInputType {
	NONE,		//押してない
	L_UP,		//左スティックの上
	L_DOWN,		//左スティックの下
	L_LEFT,		//左スティックの左
	L_RIGHT,	//左スティックの右
	R_UP,		//右スティックの上
	R_DOWN,		//右スティックの下
	R_LEFT,		//右スティックの左
	R_RIGHT,	//右スティックの右
	L_TRIGGER,	//左トリガー
	R_TRIGGER	//右トリガー
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

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name=""></param>
	void Update(void);


private:
	/// <summary>
		///	入力状態
		/// </summary>
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

	//アナログ入力対応表
	using AnalogInputTable_t =
		std::unordered_map<AnalogInputType, std::function<bool(const XINPUT_STATE&)>>;
	AnalogInputTable_t analogInputTable_;

	/// <summary>
	/// 入力対応表を初期化する
	/// </summary>
	void ResetTable();
};

