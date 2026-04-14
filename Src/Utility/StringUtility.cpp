#include <DxLib.h>
#include<cassert>
#include "StringUtility.h"

std::wstring StringUtility::StringToWstring(const std::string& str)
{
    std::wstring ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = MultiByteToWideChar(CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        static_cast<int>(str.length()),
        nullptr,
        0);

    assert(result >= 0);

    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = MultiByteToWideChar(CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),//入力文字列
        static_cast<int>(str.length()),
        ret.data(),
        static_cast<int>(ret.size()));

    return ret;
}

std::string StringUtility::WstringToString(const std::wstring& wstr)
{
    std::string ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        static_cast<int>(wstr.length()),
        nullptr,
        0,
        nullptr,
        nullptr);
    assert(result >= 0);
    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        static_cast<int>(wstr.length()),
        ret.data(),
        static_cast<int>(ret.size()),
        nullptr,
        nullptr);
    return ret;
}

std::wstring StringUtility::GetWStringFromString(const std::string& str)
{
    if (!str.empty())
    {
        return StringToWstring(str);
	}
    return std::wstring();
}
