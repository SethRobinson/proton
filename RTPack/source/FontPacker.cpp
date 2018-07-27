#include "FontPacker.h"
#include "FileSystem/FileManager.h"
#include "util/rapidxml/rapidxml.hpp"
#include "util/MiscUtils.h"
#include "util/ResourceUtils.h"
#include "App.h"

using namespace std;
using namespace rapidxml;

FontPacker::FontPacker()
{

}

FontPacker::~FontPacker()
{
}


bool FontPacker::WriteHeaderBitMapFontGenerator(FILE *fp, string fntFile, rtfont_header &header)
{
	TextScanner t(fntFile, false);

	memcpy(header.rtFileHeader.fileTypeID, C_RTFILE_FONT_HEADER, 6);

	vector<string> parms = StringTokenize(t.m_lines[1], "=");

	//vector<string> parms = CL_String::tokenize(t.m_lines[1], "=");
	header.charSpacing = 0;
	header.lineHeight = StringToInt(parms[1]);
	header.lineSpacing = StringToInt(parms[1]); //2 would be "base", whatever that is
	header.shadowXOffset = 0;
	header.shadowYOffset = 0;

	string line;
	header.kerningPairCount = 0;

	while (string(line = t.GetMultipleLineStrings("kerning first", "=")).length() > 0)
	{
		header.kerningPairCount++;
	}

	parms = StringTokenize(t.m_lines[3], "=");
	int charCount = StringToInt(parms[1]);

	header.firstChar = 32;
	header.lastChar = 255;

	fwrite(&header, 1, sizeof(rtfont_header), fp);
	//add the character data
	rtfont_charData charData;

	int skipped = 0;

	for (int i = 0; i < header.lastChar - header.firstChar; i++)
	{
		if ((int)t.m_lines.size() <= i)
		{
			//no line here, avoid crash
			parms.clear();
		}
		else
		{
			parms = StringTokenize(t.m_lines[(4 + i) - skipped], "=");
		}


		int index = 0;

		if (parms.size() > 1)
		{
			index = StringToInt(parms[1]);
		}


		if (index == header.firstChar + i)
		{
			//actually use this
			charData.bmpPosX = StringToInt(parms[2]);
			charData.bmpPosY = StringToInt(parms[3]);
			charData.charSizeX = StringToInt(parms[4]);
			charData.charSizeY = StringToInt(parms[5]);
			charData.charBmpOffsetX = StringToInt(parms[6]);
			charData.charBmpOffsetY = StringToInt(parms[7]);

			charData.charBmpPosU = 0;
			charData.charBmpPosV = 0;
			charData.charBmpPosU2 = 0;
			charData.charBmpPosV2 = 0;
			charData.xadvance = StringToInt(parms[8]);
		}
		else
		{
			//filler padding, we don't support this char

			charData.bmpPosX = 0;
			charData.bmpPosY = 0;
			charData.charSizeX = 0;
			charData.charSizeY = 0;
			charData.charBmpOffsetX = 0;
			charData.charBmpOffsetY = 0;

			charData.charBmpPosU = 0;
			charData.charBmpPosV = 0;
			charData.charBmpPosU2 = 0;
			charData.charBmpPosV2 = 0;
			charData.xadvance = 0;
			skipped++;
		}

		fwrite(&charData, 1, sizeof(rtfont_charData), fp);
	}

	KerningPair k;

	//also add kerning data, if applicable
	while (string(line = t.GetMultipleLineStrings("kerning first", "=")).length() > 0)
	{

		parms = StringTokenize(line, "=");
		k.first = StringToInt(parms[1]);
		k.second = StringToInt(parms[2]);
		k.amount = StringToInt(parms[3]);

		fwrite(&k, 1, sizeof(KerningPair), fp);
	}


	return true;
}

bool FontPacker::WriteHeaderBitMapFontGeneratorXML(FILE *fp, string fntFile, rtfont_header &header)
{
	unsigned int size;
	char* pData =  (char*)LoadFileIntoMemory(fntFile, &size);
	if (!pData)
	{
		return false;
	}

	xml_document<> fontDoc;
	fontDoc.parse<0>(pData);

	memcpy(header.rtFileHeader.fileTypeID, C_RTFILE_FONT_HEADER, 6);

	xml_node<> *fontNode = fontDoc.first_node("font");

	xml_node<> *commonNode = fontNode->first_node("common");
	int lineHeight = StringToInt(commonNode->first_attribute("lineHeight")->value());

	header.charSpacing = 0;
	header.lineHeight = lineHeight;
	header.lineSpacing = lineHeight;
	header.shadowXOffset = 0;
	header.shadowYOffset = 0;

	xml_node<> *kerningsNode = fontNode->first_node("kernings");
	int kerningCount = StringToInt(kerningsNode->first_attribute("count")->value());
	header.kerningPairCount = kerningCount;

	xml_node<> *charsNode = fontNode->first_node("chars");

	list<BMFGChar> charList;

	for (xml_node<> *charNode = charsNode->first_node("char"); charNode; charNode = charNode->next_sibling("char"))
	{
		BMFGChar c;
		c.id = StringToInt(charNode->first_attribute("id")->value());
		c.x = StringToInt(charNode->first_attribute("x")->value());
		c.y = StringToInt(charNode->first_attribute("y")->value());
		c.width = StringToInt(charNode->first_attribute("width")->value());
		c.height = StringToInt(charNode->first_attribute("height")->value());
		c.xoffset = StringToInt(charNode->first_attribute("xoffset")->value());
		c.yoffset = StringToInt(charNode->first_attribute("yoffset")->value());
		c.xadvance = StringToInt(charNode->first_attribute("xadvance")->value());

		// Put the chars in the list in ascending order according to their id
		list<BMFGChar>::iterator insIt(charList.begin());
		while (insIt != charList.end())
		{
			if (insIt->id > c.id)
			{
				break;
			}
			++insIt;
		}
		charList.insert(insIt, c);
	}

	header.firstChar = charList.front().id;
	header.lastChar = charList.back().id + 1;

	fwrite(&header, 1, sizeof(rtfont_header), fp);
	//add the character data
	rtfont_charData charData;

	list<BMFGChar>::iterator charIt(charList.begin());
	for (short i = header.firstChar; i < header.lastChar; ++i)
	{
		if (i == charIt->id)
		{
			//actually use this
			charData.bmpPosX = charIt->x;
			charData.bmpPosY = charIt->y;
			charData.charSizeX = charIt->width;
			charData.charSizeY = charIt->height;
			charData.charBmpOffsetX = charIt->xoffset;
			charData.charBmpOffsetY = charIt->yoffset;

			charData.charBmpPosU = 0;
			charData.charBmpPosV = 0;
			charData.charBmpPosU2 = 0;
			charData.charBmpPosV2 = 0;
			charData.xadvance = charIt->xadvance;

			++charIt;
		}
		else
		{
			//filler padding, we don't support this char

			charData.bmpPosX = 0;
			charData.bmpPosY = 0;
			charData.charSizeX = 0;
			charData.charSizeY = 0;
			charData.charBmpOffsetX = 0;
			charData.charBmpOffsetY = 0;

			charData.charBmpPosU = 0;
			charData.charBmpPosV = 0;
			charData.charBmpPosU2 = 0;
			charData.charBmpPosV2 = 0;
			charData.xadvance = 0;
		}

		fwrite(&charData, 1, sizeof(rtfont_charData), fp);
	}

	KerningPair k;

	//also add kerning data, if applicable
	for (xml_node<> *kerningNode = kerningsNode->first_node("kerning"); kerningNode; kerningNode = kerningNode->next_sibling("kerning"))
	{
		k.first = StringToInt(kerningNode->first_attribute("first")->value());
		k.second = StringToInt(kerningNode->first_attribute("second")->value());
		k.amount = StringToInt(kerningNode->first_attribute("amount")->value());

		fwrite(&k, 1, sizeof(KerningPair), fp);
	}

	SAFE_DELETE_ARRAY(pData);

	return true;
}

bool FontPacker::WriteHeader(FILE *fp, string fntFile, rtfont_header &header)
{
	TextScanner t(fntFile, false);
	if (!t.IsLoaded())
	{
		LogError("Can't find %s in the current working directory!", fntFile.c_str());
		WaitForKey();
		return false;
	}

	if (t.m_lines[0][0] == 'i' && t.m_lines[0][1] == 'n')
	{
		//"info" at the front means its a BitMap Font Generator fnt file
		return WriteHeaderBitMapFontGenerator(fp, fntFile, header);
	}
	else if (t.m_lines[0].find("<font") != string::npos)
	{
		// BitMap Font Generator XML file
		return WriteHeaderBitMapFontGeneratorXML(fp, fntFile, header);
	}

	//assume it's a Well Oiled Font file
	string fntVersion = t.m_lines[1];
	if (fntVersion != "1")
	{
		LogError("Warning: Unknown version of the .fnt file, this will probably explode");
	}

	vector<string> parms = StringTokenize(t.m_lines[3], ",");
	memcpy(header.rtFileHeader.fileTypeID, C_RTFILE_FONT_HEADER, 6);

	header.charSpacing = StringToInt(parms[0]);
	header.lineHeight = StringToInt(parms[1]);
	header.lineSpacing = StringToInt(parms[2]);
	header.shadowXOffset = StringToInt(parms[3]);
	header.shadowYOffset = StringToInt(parms[4]);
	header.kerningPairCount = 0;
	parms = StringTokenize(t.m_lines[5], ",");
	header.firstChar = StringToInt(parms[0]);
	header.lastChar = StringToInt(parms[1]);

	fwrite(&header, 1, sizeof(rtfont_header), fp);

	//add the character data
	rtfont_charData charData;

	for (int i = 0; i < header.lastChar - header.firstChar; i++)
	{
		parms = StringTokenize(t.m_lines[6 + i], ",");
		charData.bmpPosX = StringToInt(parms[0]);
		charData.bmpPosY = StringToInt(parms[1]);
		charData.charSizeX = StringToInt(parms[2]);
		charData.charSizeY = StringToInt(parms[3]);
		charData.charBmpOffsetY = StringToInt(parms[4]);
		charData.charBmpOffsetX = 0;

		charData.charBmpPosU = StringToFloat(parms[5]);
		charData.charBmpPosV = StringToFloat(parms[6]);
		charData.charBmpPosU2 = StringToFloat(parms[7]);
		charData.charBmpPosV2 = StringToFloat(parms[8]);
		charData.xadvance = 0;

		fwrite(&charData, 1, sizeof(rtfont_charData), fp);
	}

	return true;
}

bool FontPacker::WriteFontStates(FILE *fp, TextScanner &t)
{
	string line;
	while (string(line = t.GetMultipleLineStrings("add_format_color")).length() > 0)
	{
		FontState s;
		s.m_triggerChar = SeparateStringSTL(line, 1, '|')[0];
		string color = SeparateStringSTL(line, 2, '|');

		byte r = StringToInt(SeparateStringSTL(color, 0, ','));
		byte g = StringToInt(SeparateStringSTL(color, 1, ','));
		byte b = StringToInt(SeparateStringSTL(color, 2, ','));
		s.m_color = MAKE_RGB(r, g, b);
		m_fontStates.push_back(s);
	}

	//now let's write them to the file
	for (unsigned int i = 0; i < m_fontStates.size(); i++)
	{
		fwrite(&m_fontStates[i].m_color, 4, 1, fp);
		fwrite(&m_fontStates[i].m_triggerChar, 1, 1, fp);
		fwrite("\0\0\0", 3, 1, fp); //blank reserved
	}

	return true;
}

bool FontPacker::PackFont(string fileName)
{
	TextScanner t(fileName, false);

	if (!t.IsLoaded())
	{
		LogError("Can't find %s", fileName.c_str());
		WaitForKey();
		return false;
	}
	rtfont_header header;
	
	memset(&header, 0, sizeof(rtfont_header));

	string fontImage = t.GetParmString("image", 1);
	string fntFile = t.GetParmString("fnt_file", 1);
	header.blankCharWidth = StringToInt(t.GetParmString("blank_space_width", 1));

	//count how many font states we're going to have
	while (t.GetMultipleLineStrings("add_format_color").length() > 0) { header.fontStateCount++; }

	string path = RemoveLastPartOfDir(fileName);

	//first, let's compress the image file..?
	if (GetFileExtension(fontImage) != "rttex")
	{

		//this works, but cleaner to just do it inline
		//RunRTPackByCommandLine("-pvrt4444 "+RemoveLastPartOfDir(fileName)+fontImage);
		
		TexturePacker packer;
		GetApp()->SetMaxMipLevel(1);
		GetApp()->SetPixelType(pvrtexlib::OGL_RGBA_8888);
		packer.SetCompressionType(pvrtexlib::OGL_RGBA_8888);
		packer.ProcessTexture(path + fontImage);
	}

	//first we'll write the header and main data
	string outputFileName = ModifyFileExtension(fileName, "rtfont");
	FILE *fp = fopen(outputFileName.c_str(), "wb");
	WriteHeader(fp, path + fntFile, header);

	WriteFontStates(fp, t);
	//append the BMP to use next
	unsigned int size;
	byte *pData = LoadFileIntoMemory(path + ModifyFileExtension(fontImage, "rttex"), &size);

	if (!pData)
	{
		LogError("Unable to open %s", (path + ModifyFileExtension(fontImage, "rttex")).c_str());
	}
	else
	{
		fwrite(pData, size, 1, fp);
	}

	SAFE_DELETE_ARRAY(pData);

	fclose(fp);

	//let's compress the entire file
	//RunRTPackByCommandLine(outputFileName);
	CompressFile(outputFileName);

	LogMsg("Finished creating %s.  %d chars, %d font state definitions.", outputFileName.c_str(), header.lastChar - header.firstChar, header.fontStateCount);

	return true;
}
