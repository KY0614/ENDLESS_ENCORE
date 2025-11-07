#include<DxLib.h>
#include<cassert>
#include "Input.h"

namespace {
	//ヘッダ部
	struct Header {
		char signature[4];
		float version;
		uint32_t dataSize;//4バイト
	};

	//アナログスティックのしきい値
	const int MAX_ANALOG_STICK = 10000;	//スティックの最大値
	const int MAX_ANALOG_TRIGGER = 128;	//トリガーの最大値
}

Input::Input()
{
	ResetTable();
	Load();

	analogInputTable_[AnalogInputType::L_UP] = [](const XINPUT_STATE& state) {
		return state.ThumbLY > MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::L_DOWN] = [](const XINPUT_STATE& state) {
		return state.ThumbLY < -MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::L_RIGHT] = [](const XINPUT_STATE& state) {
		return state.ThumbLX > MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::L_LEFT] = [](const XINPUT_STATE& state) {
		return state.ThumbLX < -MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::R_UP] = [](const XINPUT_STATE& state) {
		return state.ThumbRY > MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::R_DOWN] = [](const XINPUT_STATE& state) {
		return state.ThumbRY < -MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::R_LEFT] = [](const XINPUT_STATE& state) {
		return state.ThumbRX > MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::R_RIGHT] = [](const XINPUT_STATE& state) {
		return state.ThumbRX < -MAX_ANALOG_STICK;
		};
	analogInputTable_[AnalogInputType::L_TRIGGER] = [](const XINPUT_STATE& state) {
		return state.LeftTrigger > MAX_ANALOG_TRIGGER;
		};
	analogInputTable_[AnalogInputType::R_TRIGGER] = [](const XINPUT_STATE& state) {
		return state.RightTrigger > MAX_ANALOG_TRIGGER;
		};

	for (const auto& keyvalue : inputTable_)
	{
		currentInput_[keyvalue.first] = false;
	}
	lastInput_ = currentInput_;
}

bool Input::IsTrigerred(const std::string& eventcode) const
{
	//containsはキーがあるかどうか
	if (!currentInput_.contains(eventcode))return false;

	//atは読み取り専用的な、
	return currentInput_.at(eventcode) && !lastInput_.at(eventcode);
}

bool Input::IsPressed(const std::string& eventcode) const
{
	if (!currentInput_.contains(eventcode)) return false;
	return currentInput_.at(eventcode);
}

void Input::Update(void)
{
	//押した顔してないか記録する部分
	lastInput_ = currentInput_;//前のプッシュ情報を記憶

	//キーボード情報
	char keyState[256] = {};
	GetHitKeyStateAll(keyState);

	//パッド情報
	int padState = GetJoypadInputState(DX_INPUT_PAD1);

	//マウス情報
	int mouseState = GetMouseInput();

	//アナログ情報
	XINPUT_STATE xinputState = {};
	GetJoypadXInputState(DX_INPUT_PAD1, &xinputState);

	//テーブルの行を回す
	for (const auto& keyvalue : inputTable_)
	{
		//特定のキーの入力情報を取得
		for (auto input : keyvalue.second)
		{
			bool pressed = false;
			if (input.type == PeripheralType::KEYBOARD)
			{
				pressed = keyState[input.code];
			}
			else if (input.type == PeripheralType::GAMEPAD)
			{
				pressed = padState&input.code;
			}
			else if (input.type == PeripheralType::MOUSE)
			{
				pressed = mouseState & input.code;
			}
			else if (input.type == PeripheralType::X_ANALOG)
			{
				pressed = analogInputTable_[(AnalogInputType)input.code](xinputState);
			}
			currentInput_[keyvalue.first] = pressed;
			if (pressed)break;
		}
	}
}

void Input::ResetTable()
{
	inputTable_ = {
	{"Back",{
		{PeripheralType::KEYBOARD,KEY_INPUT_RETURN},
		{PeripheralType::GAMEPAD,PAD_INPUT_L},//selectボタン
		//{PeripheralType::mouse,MOUSE_INPUT_LEFT}}//左クリック	
		}
	},

	{"Pause",{
		{PeripheralType::KEYBOARD,KEY_INPUT_P},
		{PeripheralType::GAMEPAD,PAD_INPUT_R}}	//STARTボタン
	},
	};

	inputTable_["Reset"] = { {PeripheralType::KEYBOARD,KEY_INPUT_R},
							/*{PeripheralType::gamepad, PAD_INPUT_A},*/
							/*{PeripheralType::x_analog,(int)AnalogInputType::l_trigger}*/ };

	inputTable_["Decide"] = { {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
							{PeripheralType::GAMEPAD, PAD_INPUT_B},
							/*{PeripheralType::x_analog,(int)AnalogInputType::l_trigger}*/ };

	inputTable_["Parry"] = { {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
							{PeripheralType::GAMEPAD, PAD_INPUT_B},
							/*{PeripheralType::x_analog,(int)AnalogInputType::l_trigger}*/ };

	inputTable_["Dodge"] = { {PeripheralType::KEYBOARD,KEY_INPUT_LSHIFT},
							{PeripheralType::GAMEPAD, PAD_INPUT_A},
							{PeripheralType::MOUSE, MOUSE_INPUT_RIGHT} };

	inputTable_["Dash"] = { {PeripheralType::KEYBOARD,KEY_INPUT_LCONTROL},
							{PeripheralType::GAMEPAD, PAD_INPUT_Y},
							{PeripheralType::X_ANALOG,(int)AnalogInputType::L_TRIGGER} };

	inputTable_["Jump"] = { {PeripheralType::KEYBOARD,KEY_INPUT_F},
							{PeripheralType::GAMEPAD, PAD_INPUT_Y},
							/*{PeripheralType::x_analog,(int)AnalogInputType::r_trigger}*/ };

	inputTable_["Up"] = {	{PeripheralType::KEYBOARD,KEY_INPUT_W},
							{PeripheralType::GAMEPAD,PAD_INPUT_UP},
							{PeripheralType::X_ANALOG,(int)AnalogInputType::L_UP} };

	inputTable_["Down"] = { {PeripheralType::KEYBOARD,KEY_INPUT_S},
							{PeripheralType::GAMEPAD,PAD_INPUT_DOWN},
							{PeripheralType::X_ANALOG,(int)AnalogInputType::L_DOWN} };

	inputTable_["Right"] = {{PeripheralType::KEYBOARD,KEY_INPUT_D},
							{PeripheralType::GAMEPAD,PAD_INPUT_RIGHT},
							{PeripheralType::X_ANALOG,(int)AnalogInputType::L_RIGHT} };

	inputTable_["Left"] = { {PeripheralType::KEYBOARD,KEY_INPUT_A},
							{PeripheralType::GAMEPAD,PAD_INPUT_LEFT},
							{PeripheralType::X_ANALOG,(int)AnalogInputType::L_LEFT} };


	inputTable_["CameraShake"] = { {PeripheralType::KEYBOARD,KEY_INPUT_X},
		/*{PeripheralType::gamepad, PAD_INPUT_A},*/
		/*{PeripheralType::x_analog,(int)AnalogInputType::l_trigger}*/ };

	
	inputTable_["CameraUp"] = { {PeripheralType::KEYBOARD,KEY_INPUT_UP},
					/*{PeripheralType::GAMEPAD,PAD_INPUT_UP},*/
					{PeripheralType::X_ANALOG,(int)AnalogInputType::R_UP} };

	inputTable_["CameraDown"] = { {PeripheralType::KEYBOARD,KEY_INPUT_DOWN},
							/*{PeripheralType::GAMEPAD,PAD_INPUT_DOWN},*/
							{PeripheralType::X_ANALOG,(int)AnalogInputType::R_DOWN} };

	inputTable_["CameraRight"] = { {PeripheralType::KEYBOARD,KEY_INPUT_RIGHT},
							/*{PeripheralType::GAMEPAD,PAD_INPUT_RIGHT},*/
							{PeripheralType::X_ANALOG,(int)AnalogInputType::R_LEFT} };

	inputTable_["CameraLeft"] = { {PeripheralType::KEYBOARD,KEY_INPUT_LEFT},
							/*{PeripheralType::GAMEPAD,PAD_INPUT_LEFT},*/
							{PeripheralType::X_ANALOG,(int)AnalogInputType::R_RIGHT} };

	inputTable_["CameraRise"] = { {PeripheralType::KEYBOARD,KEY_INPUT_Q}};

	inputTable_["CameraDescent"] = { {PeripheralType::KEYBOARD,KEY_INPUT_E} };
}

void Input::Save()
{
	FILE* fp = nullptr;
	auto err = fopen_s(&fp, "input.conf", "wb");
	if (fp == nullptr)return;

	Header header = {};
	header.version = 1.00f;
	header.dataSize = inputTable_.size();
	std::copy_n("kycf",4,header.signature);
	fwrite(&header, sizeof(header), 1, fp);

	//データ部
	for (const auto& row : inputTable_)
	{
		//イベント名
		auto eventName = row.first;
		uint8_t nameSize = static_cast<uint8_t>(eventName.size());
		fwrite(&nameSize, sizeof(nameSize), 1, fp);
		fwrite(eventName.data(), sizeof(char), nameSize, fp);

		//データ部
		uint32_t rowSize = row.second.size();
		fwrite(&rowSize, sizeof(rowSize), 1, fp);
		fwrite(row.second.data(), sizeof(InputState), rowSize, fp);

	}

	fclose(fp);
}

void Input::Load()
{
	int handle = FileRead_open(L"input.conf");
	if (handle == 0)return;
	assert(handle != 0);
	//ヘッダ部読み込み
	Header header = {};
	FileRead_read(&header, sizeof(header), handle);

	//データ部
	for (int i = 0; i < header.dataSize;++i)
	{
		//イベント名
		uint8_t nameSize = 0;
		FileRead_read(&nameSize, sizeof(nameSize),handle);
		std::string eventName;
		eventName.resize(nameSize);
		FileRead_read(eventName.data(), eventName.size(), handle);

		//データ部
		uint32_t rowSize = 0;
		FileRead_read(&rowSize, sizeof(rowSize),handle);
		//if (rowSize > 1000000) break; // 異常値防止
		std::vector<InputState> rowData(rowSize);
		FileRead_read(rowData.data(), sizeof(InputState)*rowSize, handle);
		inputTable_[eventName] = rowData;
	}

	FileRead_close(handle);
}
