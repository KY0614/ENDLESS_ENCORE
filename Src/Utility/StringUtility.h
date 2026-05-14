#pragma once
#include<string>
#include<vector>

class StringUtility
{
public:

	/// <summary>
	/// string‚ðwstring‚É•ÏŠ·
	/// </summary>
	/// <param name="str">string‚Ì•¶Žš—ñ</param>
	/// <returns>wstringŒ^‚Ì•¶Žš—ñ</returns>
	static std::wstring String2Wstring(const std::string& str);

	/// <summary>
	/// wstring‚ðstring‚É•ÏŠ·
	/// </summary>
	/// <param name="wstr">wstring‚Ì•¶Žš—ñ</param>
	/// <returns>string‚Ì•¶Žš—ñ</returns>
	static std::string Wstring2String(const std::wstring& wstr);
	
	/// <summary>
	/// wstring‚ðUTF-8‚Ìstring‚É•ÏŠ·
	/// </summary>
	/// <param name="wstr">wstring‚Ì•¶Žš—ñ</param>
	/// <returns>UTF-8‚ÌstringŒ^•¶Žš—ñ</returns>
	static std::string Wstring2UTF8(const std::wstring& wstr);
};

