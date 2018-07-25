#ifndef RTFontFileFormat_h__
#define RTFontFileFormat_h__

#include "../util/RTFileFormat.h"

#define C_RTFILE_FONT_HEADER "RTFONT"
struct rtfont_header
{
	RTFileHeader rtFileHeader;

	//our custom header
	short charSpacing;
	short lineHeight;
	short lineSpacing;
	short shadowXOffset;
	short shadowYOffset;
	short firstChar;
	short lastChar; //lastChar-firstChar is how many character definitions are coming
	short blankCharWidth;
	short fontStateCount; //how many FontState data thingies are coming
	short kerningPairCount;
	byte reserved[124];
};

struct rtfont_charData
{
	short bmpPosX, bmpPosY;
	short charSizeX, charSizeY;
	short charBmpOffsetX; //used by Bitmap Font Generator only
	short charBmpOffsetY;
	float charBmpPosU, charBmpPosV; //used by Well Oiled font maker only, ignored by us
	float charBmpPosU2, charBmpPosV2; //used by Well Oiled font maker only, ignored by us
	short xadvance; //used by Bitmap Font Generator only
};



struct KerningPair
{
	short first, second;
	signed char amount;
};


class FontChar
{
public:
	rtfont_charData data;
};


class FontState
{
public:
	FontState(){};
	FontState(char triggerChar, unsigned int color) : m_triggerChar(triggerChar), m_color(color) {};
	unsigned int m_color;
	char m_triggerChar;
};

typedef std::deque<FontState> FontStateStack;


#endif // RTFontFileFormat_h__
