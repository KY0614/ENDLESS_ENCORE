#pragma once
#include<cstdint>

class Font
{
public:
	struct FileHeader
	{
		char signature[3];
		byte version;
	};
#pragma pack (push,1)
	struct BlockInfo
	{
		uint8_t id;		//blockType 1 uint 0
		///3byte padidng
		uint32_t size;	//blockSize 4 uint 1
	};
#pragma
	struct FileInfo
	{
		uint16_t fontSize;
		uint8_t bit;	//bitField;
		uint8_t cset;	//charSet;
		uint16_t stretchH;
		uint8_t aa;//aa
		uint8_t pdup;//paddingUp
		uint8_t pdrt;//paddingRight
		uint8_t pddn;//paddingDown
		uint8_t pdlf;//paddingLeft
		uint8_t spH;//spacingHoriz
		uint8_t spV; //spacingVertical
		uint8_t outline;
		
	};
#pragma pack (push,1)
	struct CommonInfo
	{
		uint16_t lineHeight;
		uint16_t base;
		uint16_t scaleW;
		uint16_t scaleH;
		uint16_t pages;
		uint8_t bitField; //bitField
		uint8_t alphaChnl;
		uint8_t redChnl;
		uint8_t greenChnl;
		uint8_t blueChnl;
	};
#pragma pack (pop)

	struct CharInfo
	{
		uint32_t id;	//id	4	uint	0
		uint16_t x;		//x	2	uint	4
		uint16_t y;		//y	2	uint	6
		uint16_t width;	//width	2	uint	8
		uint16_t height;//height	2	uint	10
		int16_t xOffset;//xOffset	2	int	12
		int16_t yOffset;//yOffset	2	int	14
		int16_t xAdvance;//xAdvance	2	int	16
		uint8_t page;	//page	1	uint	18
		uint8_t chnl;	//chn1 1 bits 19 bit 0 : blue 1 : green 2 : red 3 : alpha
	};

	struct CutInfo
	{
		int x, y, w, h;
		int xOffset, yOffset;
		int xAdvance;
		int page;
	};

	Font(void);
	~Font(void);

	void Init(void);
	void Update(void);
	void Draw(void);

private:

	void DrawMyFontstring(int x, int y, const std::wstring& str,
		std::map<uint32_t, CutInfo>& cutInfoTable,
		std::vector<int>& pageHs);

	void DrawMyFontFormatString(int x, int y,
		std::map<uint32_t, CutInfo>& cutInfoTable,
		std::vector<int>& pageHs, const std::wstring& formatStr, ...);
};