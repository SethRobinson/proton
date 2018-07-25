//  ***************************************************************
//  FontPacker - Creation date: 03/26/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FontPacker_h__
#define FontPacker_h__

#include "main.h"
#include "GUI/RTFontFileFormat.h"

class FontPacker
{
public:
	FontPacker();
	virtual ~FontPacker();
	bool PackFont(std::string fileName);

private:

	bool WriteHeader(FILE *fp, std::string fntFile, rtfont_header &header);
	bool WriteFontStates(FILE *fp, TextScanner &t);
	bool WriteHeaderBitMapFontGenerator(FILE *fp, std::string fntFile, rtfont_header &header);
	bool WriteHeaderBitMapFontGeneratorXML(FILE *fp, std::string fntFile, rtfont_header &header);
	std::vector<FontState> m_fontStates;

	struct BMFGChar
	{
		short id;
		short x;
		short y;
		short width;
		short height;
		short xoffset;
		short yoffset;
		short xadvance;
	};

};

#endif // FontPacker_h__
