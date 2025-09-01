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
}

Input::Input()
{
	ResetTable();
	Load();

	inputlistForDisplay_ = {
		"Back","pause","Dash","Interact"
	};
	analogInputTable_[AnalogInputType::l_up] = [](const XINPUT_STATE& state) {
		return state.ThumbLY > 10000;
	};	
	analogInputTable_[AnalogInputType::l_down] = [](const XINPUT_STATE& state) {
		return state.ThumbLY < -10000;
	};
	analogInputTable_[AnalogInputType::l_right] = [](const XINPUT_STATE& state) {
		return state.ThumbLX > 10000;
	};
	analogInputTable_[AnalogInputType::l_left] = [](const XINPUT_STATE& state) {
		return state.ThumbLX < -10000;
	};
	analogInputTable_[AnalogInputType::r_up] = [](const XINPUT_STATE& state) {
		return state.ThumbRY > 10000;
	};
	analogInputTable_[AnalogInputType::r_down] = [](const XINPUT_STATE& state) {
		return state.ThumbRY < -10000;
	};
	analogInputTable_[AnalogInputType::r_left] = [](const XINPUT_STATE& state) {
		return state.ThumbRX > 10000;
	};
	analogInputTable_[AnalogInputType::r_right] = [](const XINPUT_STATE& state) {
		return state.ThumbRX < -10000;
	};
	analogInputTable_[AnalogInputType::l_trigger] = [](const XINPUT_STATE& state) {
		return state.LeftTrigger > 128;
	};
	analogInputTable_[AnalogInputType::r_trigger] = [](const XINPUT_STATE& state) {
		return state.RightTrigger > 128;
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
			if (input.type == PeripheralType::keyboard)
			{
				pressed = keyState[input.code];
			}
			else if (input.type == PeripheralType::gamepad)
			{
				pressed = padState&input.code;
			}
			else if (input.type == PeripheralType::mouse)
			{
				pressed = mouseState & input.code;
			}
			else if (input.type == PeripheralType::x_analog)
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
		{PeripheralType::keyboard,KEY_INPUT_RETURN},
		{PeripheralType::gamepad,PAD_INPUT_A},//Aボタン
		//{PeripheralType::mouse,MOUSE_INPUT_LEFT}}//左クリック	
		}
	},
	{"pause",{
		{PeripheralType::keyboard,KEY_INPUT_P},
		{PeripheralType::gamepad,PAD_INPUT_R}}	//STARTボタン
	},
	};

	inputTable_["Dash"] = { {PeripheralType::keyboard,KEY_INPUT_LCONTROL},
							{PeripheralType::gamepad, PAD_INPUT_Y},
							{PeripheralType::x_analog,(int)AnalogInputType::l_trigger} };

	inputTable_["Interact"] = { {PeripheralType::keyboard,KEY_INPUT_SPACE},
							{PeripheralType::gamepad, PAD_INPUT_B},
							/*{PeripheralType::x_analog,(int)AnalogInputType::r_trigger}*/ };

	inputTable_["Up"] = {	{PeripheralType::keyboard,KEY_INPUT_UP},
							{PeripheralType::gamepad,PAD_INPUT_UP},
							{PeripheralType::x_analog,(int)AnalogInputType::l_up} };

	inputTable_["Down"] = { {PeripheralType::keyboard,KEY_INPUT_DOWN},
							{PeripheralType::gamepad,PAD_INPUT_DOWN},
							{PeripheralType::x_analog,(int)AnalogInputType::l_down} };

	inputTable_["Right"] = {{PeripheralType::keyboard,KEY_INPUT_RIGHT},
							{PeripheralType::gamepad,PAD_INPUT_RIGHT},
							{PeripheralType::x_analog,(int)AnalogInputType::l_right} };

	inputTable_["Left"] = { {PeripheralType::keyboard,KEY_INPUT_LEFT},
							{PeripheralType::gamepad,PAD_INPUT_LEFT},
							{PeripheralType::x_analog,(int)AnalogInputType::l_left} };
	//
	//inputTable_["CameraUp"] = { {PeripheralType::keyboard,KEY_INPUT_W},
	//				{PeripheralType::gamepad,PAD_INPUT_UP},
	//				{PeripheralType::x_analog,(int)AnalogInputType::l_up} };

	//inputTable_["CameraDown"] = { {PeripheralType::keyboard,KEY_INPUT_S},
	//						{PeripheralType::gamepad,PAD_INPUT_DOWN},
	//						{PeripheralType::x_analog,(int)AnalogInputType::l_down} };

	//inputTable_["CameraRight"] = { {PeripheralType::keyboard,KEY_INPUT_D},
	//						{PeripheralType::gamepad,PAD_INPUT_RIGHT},
	//						{PeripheralType::x_analog,(int)AnalogInputType::l_right} };

	//inputTable_["CameraLeft"] = { {PeripheralType::keyboard,KEY_INPUT_A},
	//						{PeripheralType::gamepad,PAD_INPUT_LEFT},
	//						{PeripheralType::x_analog,(int)AnalogInputType::l_left} };

	//inputTable_["CameraFront"] = { {PeripheralType::keyboard,KEY_INPUT_I}};
	//inputTable_["CameraBack"] = { {PeripheralType::keyboard,KEY_INPUT_K}};
	//inputTable_["CameraR"] = { {PeripheralType::keyboard,KEY_INPUT_L}};
	//inputTable_["CameraL"] = { {PeripheralType::keyboard,KEY_INPUT_J}};

	//inputTable_["CameraRise"] = { {PeripheralType::keyboard,KEY_INPUT_Q}};

	//inputTable_["CameraDescent"] = { {PeripheralType::keyboard,KEY_INPUT_E} };

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
