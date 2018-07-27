//  ***************************************************************
//  App - Creation date: 04/19/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef App_h__
#define App_h__

#include "main.h"

#define RT_ENABLE_FILEMANAGER
#ifdef RT_NO_PVR
//minimum data needed so we can compile without PVRTexLib
namespace pvrtexlib
{
	enum PixelType
	{
		// OpenGL version of pixel types
		OGL_RGBA_4444 = 0x10,
		OGL_RGBA_5551,
		OGL_RGBA_8888,
		OGL_RGB_565,
		OGL_RGB_555,
		OGL_RGB_888,
	};
}

#else
#include "PVRTexLib/PVRTexLib.h"
#endif
#include "TexturePacker.h"

class FileManager;
FileManager * GetFileManager();

class App
{
public:
	App();
	virtual ~App();

	bool Init();
	bool Update();
	void Kill();

	enum eOutput
	{
		RTTEX,
		PVR,
		PNG,
		JPG,
		BMP
	};

	eOutput GetOutput() { return m_output; }
	void SetOutput(eOutput o) { m_output = o; }
	void SetPixelType(pvrtexlib::PixelType ptype);
	pvrtexlib::PixelType GetPixelType() { return m_pixelType; }
	void SetPixelTypeText(std::string s);
	std::string GetPixelTypeText();

	void SetMaxMipLevel(int maxMip) { m_maxMipLevel = maxMip; }
	int GetMaxMipLevel() { return m_maxMipLevel; }

	void SetStretchImage(bool bNew) { m_stretchImage = bNew; }
	bool GetStretchImage() { return m_stretchImage; }

	void SetForceSquare(bool bNew) { m_forceSquare = bNew; }
	bool GetForceSquare() { return m_forceSquare; }

	void SetPixelTypeIfNotSquareOrTooBig(pvrtexlib::PixelType ptype) { m_pixelTypeIfNotSquareOrTooBig = ptype; }
	pvrtexlib::PixelType GetPixelTypeIfNotSquareOrTooBig() { return m_pixelTypeIfNotSquareOrTooBig; }

	void SetFlipV(bool bNew) { m_bFlipV = bNew; }
	bool GetFlipV() { return m_bFlipV; }
	bool GetForceAlpha() { return m_bForceAlpha; }
	void SetForceAlpha(bool bNew) { m_bForceAlpha = bNew; }
	bool GetNoPowerOfTwo() { return m_bNoPowerOf2; }
	void SetNoPowerOfTwo(bool bNew) { m_bNoPowerOf2 = bNew; }
	void SetUltraCompressQuality(int quality) { m_ultraCompressQuality = quality; }
	int GetUltraCompressQuality() { return m_ultraCompressQuality; }

protected:
	
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

#endif // App_h__