#pragma once
#include<string>

class StringUtility
{
public:

	/// <summary>
	/// string‚ðwstring‚É•ÏŠ·
	/// </summary>
	/// <param name="str">string‚Ì•¶Žš—ñ</param>
	/// <returns>wstringŒ^‚Ì•¶Žš—ñ</returns>
	static std::wstring StringToWstring(const std::string& str);

	/// <summary>
	/// wstring‚ðstring‚É•ÏŠ·
	/// </summary>
	/// <param name="wstr">wstring‚Ì•¶Žš—ñ</param>
	/// <returns>string‚Ì•¶Žš—ñ</returns>
	static std::string WstringToString(const std::wstring& wstr);

private:

};

