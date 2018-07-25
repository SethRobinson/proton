#ifndef main_h__
#define main_h__

#pragma once

//a windows only tool to turn any image into a .rttex or .rtfont file

#include "PlatformSetup.h"
#include "util/MiscUtils.h"
#include "util/ResourceUtils.h"
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include "PVRTexLib/PVRTexLib.h"
#include "util/RTFileFormat.h"
#include "ClanlibUtils.h"

class App
{
public:

	enum eOutput
	{
		RTTEX,
		PVR,
		PNG,
		JPG
	};

	App()
	{
		m_bForceAlpha = false;
		m_bNoPowerOf2 = false;
		m_output = RTTEX;
		SetPixelType(pvrtexlib::PixelType(0));
		SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::PixelType(0));
		SetMaxMipLevel(1);
		SetStretchImage(false);
		SetForceSquare(false);
		SetFlipV(false);
		m_ultraCompressQuality= 0;
	}

	bool ParmExistsWithData(std::string parm, std::string *parmData);
	bool ParmExists(std::string parm); //not case sensitive
	std::vector<std::string> m_parms;
	std::string GetLastParm();
	eOutput GetOutput() {return m_output;}
	void SetOutput(eOutput o) {m_output = o;}
	void SetPixelType(pvrtexlib::PixelType ptype );
	pvrtexlib::PixelType GetPixelType() {return m_pixelType;}
	void SetPixelTypeText(std::string s);
	std::string GetPixelTypeText();
	
	void SetMaxMipLevel(int maxMip) {m_maxMipLevel = maxMip;}
	int GetMaxMipLevel() {return m_maxMipLevel;}

	void SetStretchImage(bool bNew) {m_stretchImage = bNew;}
	bool GetStretchImage() {return m_stretchImage;}

	void SetForceSquare(bool bNew) {m_forceSquare = bNew;}
	bool GetForceSquare() {return m_forceSquare;}

	void SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::PixelType ptype) {m_pixelTypeIfNotSquareOrTooBig = ptype;}
	pvrtexlib::PixelType GetPixelTypeIfNotSquareOrTooBig() {return m_pixelTypeIfNotSquareOrTooBig;}

	void SetFlipV(bool bNew) {m_bFlipV = bNew;}
	bool GetFlipV() {return m_bFlipV;}
	bool GetForceAlpha() {return m_bForceAlpha;}
    void SetForceAlpha(bool bNew) {m_bForceAlpha = bNew;}
	bool GetNoPowerOfTwo() {return m_bNoPowerOf2;}
	void SetNoPowerOfTwo(bool bNew) {m_bNoPowerOf2 = bNew;}
	void SetUltraCompressQuality(int quality) {m_ultraCompressQuality = quality;}
	int GetUltraCompressQuality() {return m_ultraCompressQuality;}

private:
	
	eOutput m_output;
	pvrtexlib::PixelType m_pixelType;
	pvrtexlib::PixelType m_pixelTypeIfNotSquareOrTooBig;
	std::string m_pixelTypeText;
	int m_maxMipLevel;
	bool m_stretchImage;
	bool m_forceSquare;
	bool m_bFlipV;
	bool m_bForceAlpha;
	bool m_bNoPowerOf2;
	int m_ultraCompressQuality; //0 if disabled

};

App * GetApp();


#ifndef _WIN32
#define ZeroMemory(pntr, s) memset((pntr), 0, (s))
#endif

#endif // #define main_h__
