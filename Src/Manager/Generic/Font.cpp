#include <DxLib.h>
#include<string>
#include <vector>
#include <map>
#include "Font.h"



std::wstring 
WstringFromString(const std::string& str)
{
	std::wstring ret;
	//確保しなければならないサイズを取得
	int size = MultiByteToWideChar(CP_ACP,0,
		str.c_str(),str.length(),
		nullptr,0);
	//変換後の文字列を格納するために調整
	ret.resize(size);
	//変換実行
	MultiByteToWideChar(CP_ACP,0,
		str.c_str(),str.length(),
		ret.data(), ret.size());
	return ret;
}

Font::Font(void)
{
}

Font::~Font(void)
{
}

void Font::Init(void)
{
	//ブロック１
	FileHeader header;
	auto fontFileH = FileRead_open(L"Data/FontData.fnt");
	FileRead_read(&header, sizeof(header), fontFileH);

	auto size = sizeof(BlockInfo);
	BlockInfo blockInfo;
	FileRead_read(&blockInfo, sizeof(blockInfo), fontFileH);

	FileInfo info;
	blockInfo.size - sizeof(info);

	FileRead_read(&info, sizeof(info), fontFileH);
	auto pathSize = blockInfo.size - sizeof(info);
	std::string fontName;
	fontName.resize(pathSize);
	FileRead_read(fontName.data(), pathSize, fontFileH);

	//ブロック２
	CommonInfo commonInfo;
	FileRead_read(&commonInfo, sizeof(commonInfo), fontFileH);
	//ブロック情報
	FileRead_read(&blockInfo, sizeof(blockInfo), fontFileH);

	//ブロック３
	std::vector<std::string> pageNames;
	std::vector<int> pageHandles;//画像ハンドル
	pageNames.resize(commonInfo.pages);
	for (auto& name : pageNames)
	{
		auto nameSize = blockInfo.size / commonInfo.pages;
		name.resize(nameSize);
		FileRead_read(name.data(), nameSize, fontFileH);
		std::string path = "Font/";
		path + name;
		int pageH = LoadGraph(WstringFromString(path).c_str());
		pageHandles.push_back(pageH);
	}

	//ブロック４ 文字情報
	auto charNum = blockInfo.size / sizeof(CharInfo);
	std::vector<CharInfo> charInfos;
	charInfos.resize(charNum);
	FileRead_read(charInfos.data(), blockInfo.size, fontFileH);
	FileRead_close(fontFileH);

	std::map<uint32_t, CutInfo> cutInfos;
	for (const auto& ci : charInfos)
	{
		CutInfo cut;
		cut.x = ci.x;
		cut.y = ci.y;
		cut.w = ci.width;
		cut.h = ci.height;
		cut.xOffset = ci.xOffset;
		cut.yOffset = ci.yOffset;
		cut.xAdvance = ci.xAdvance;
		cut.page = ci.page;
		cutInfos[ci.id] = cut;
	}

	//DrawMyFontstring(0, 0, L"Hello World", cutInfos, pageHandles);
	//DrawMyFontFormatString(0, 0, cutInfos, pageHandles, L"%s : %d", L"World",65535);
}

void Font::DrawMyFontstring(int x,int y,const std::wstring& str,
	std::map<uint32_t, CutInfo>& cutInfoTable,
	std::vector<int>& pageHs)
{
	for (const auto& wc : str)
	{
		const auto& cutInfo = cutInfoTable[wc];
		DrawRectGraph(
			x + cutInfo.xOffset,
			y + cutInfo.yOffset,
			cutInfo.x, cutInfo.y,
			cutInfo.w, cutInfo.h,
			pageHs[cutInfo.page],
			true);
		x += cutInfo.xAdvance;
	}
}

void Font::DrawMyFontFormatString(int x, int y,
	std::map<uint32_t, CutInfo>& cutInfoTable,
	std::vector<int>& pageHs, const std::wstring& formatStr, ...)
{
	const int bufferSize = 256;
	va_list arg;
	va_start(arg, formatStr.c_str());
	std::wstring formattedStr;
	//十分なサイズを確保
	formattedStr.resize(formatStr.size() + bufferSize);
	vswprintf(formattedStr.data(), formatStr.c_str(), arg);
	va_end(arg);
	DrawMyFontstring(x, y, formattedStr, cutInfoTable, pageHs);
}