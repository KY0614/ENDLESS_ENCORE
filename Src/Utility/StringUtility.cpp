#include <DxLib.h>
#include<cassert>
#include "StringUtility.h"

std::wstring StringUtility::String2Wstring(const std::string& str)
{
    std::wstring ret;
    //一度目の呼び出しは文字列数を知るため
    int result = MultiByteToWideChar(CP_ACP,
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

std::string StringUtility::Wstring2String(const std::wstring& wstr)
{
    std::string ret;
    //一度目の呼び出しは文字列数を知るため
    int result = WideCharToMultiByte(
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

std::string StringUtility::Wstring2UTF8(const std::wstring& wstr)
{
	//空文字列のときは空文字列を返す
    if (wstr.empty())return std::string();
    std::string ret;
    //一度目の呼び出しは文字列数を知るため
    int result = WideCharToMultiByte(
        CP_UTF8,
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
        CP_UTF8,
        0,
        wstr.c_str(),//入力文字列
        static_cast<int>(wstr.length()),
        ret.data(),
        static_cast<int>(ret.size()),
        nullptr,
        nullptr);
    return ret;
}
