//  ***************************************************************
//  SoftSurface - Creation date: 12/07/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef SoftSurface_h__
#define SoftSurface_h__

//#include "RenderBatcher.h"

#ifdef _CONSOLE
#include "Renderer/GL/gl.h"
#include "Surface.h"
#endif

#include "bitmap.h"

#ifdef RT_PNG_SUPPORT
#include "../../Irrlicht/source/Irrlicht/libpng/png.h"
#endif


const int C_MAX_PALETTE_SIZE=256;

//Normally this isn't used, but it's useful for handling raw pixel data including palletized stuff.
//Note:  We keep palletized surfaces in the same format as bmp, ie, upside down

class SoftSurface
{

public:

	SoftSurface();
	virtual ~SoftSurface();

	enum eSurfaceType
	{
		SURFACE_NONE,
		SURFACE_PALETTE_8BIT,
		SURFACE_RGBA,
		SURFACE_RGB
	};

	enum eColorKeyType
	{
		COLOR_KEY_NONE,
		COLOR_KEY_BLACK,
		COLOR_KEY_WHITE,
		COLOR_KEY_MAGENTA,
		COLOR_KEY_CUSTOM
	};

	eSurfaceType GetSurfaceType() {return m_surfaceType;}
	int GetWidth(){return m_width;}
	int GetHeight() {return m_height;}
	int GetOriginalWidth() {return m_originalWidth;} //applicable to rttex images only
	int GetOriginalHeight() {return m_originalHeight;} //applicable to rttex images only
	int GetBytesPerPixel() {return m_bytesPerPixel;}

	bool IsActive() {return m_surfaceType != SURFACE_NONE;}

	bool Init(int sizex, int sizey, eSurfaceType type, bool bRememberOriginalData = false);
	void Kill();
	void FillColor(glColorBytes color);
	uint8 * GetPixelData() {return m_pPixels;}
	bool LoadFile(string fName, eColorKeyType colorKey = SoftSurface::COLOR_KEY_NONE, bool addBasePath = true, bool bApplyCheckerboardFix = false);
	bool LoadFileFromMemory( uint8 *pMem, eColorKeyType colorKey, int inputSize = 0, bool bAddAlphaChannelIfNotPowerOfTwo =false, bool bApplyCheckerboardFix =false );
	void Blit(int dstX, int dstY, SoftSurface *pSrc, int srcX = 0, int srcY = 0, int srcWidth = 0, int srcHeight = 0); //paste an image over ours
	void SetCustomColorKey(glColorBytes color) { m_customColorKey = color; }

	Surface * CreateGLTexture();
	void UpdateGLTexture(Surface* pSurf);
	unsigned int GetSizeInBytes() {return m_memUsed;}
	void SetAutoPremultiplyAlpha(bool bYes) {m_bAutoPremultiplyAlpha = bYes;} //will convert to pre multiplied alpha ASAP, during the next copy to 32 bit surface in the case of 8 bit images
	bool GetAutoPremultiplyAlpha() {return m_bAutoPremultiplyAlpha;}
	void SetHasPremultipliedAlpha(bool bYes) {m_bHasPremultipliedAlpha = bYes;}; //current state of premultiplied alpha
	bool GetHasPremultipliedAlpha() {return m_bHasPremultipliedAlpha;}
	
	void SetPixel( int x, int y, glColorBytes color )
	{
		if (m_surfaceType == SURFACE_RGBA)
		{
			memcpy(m_pPixels+(y*m_usedPitch+x*4), &color.r, 4);
		} else assert(!"uh");
	}

	void SetPixelSafe(int x, int y, glColorBytes color)
	{
		if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
		SetPixel(x, y, color);
	}

	void DrawCircleSafe(int centerX, int centerY, glColorBytes color, int radius)
	{
		int xStart = centerX - radius;
		int yStart = centerY - radius;
		int diameter = 2 * radius;

		for (int x = 0; x <= diameter; x++)
		{
			for (int y = 0; y <= diameter; y++)
			{
				int dx = radius - x;
				int dy = radius - y;
				if (dx * dx + dy * dy <= radius * radius)
				{
					SetPixelSafe(centerX + dx, centerY + dy, color);
				}
			}
		}
	}

	
	void SetPixel( int x, int y, uint8 color )
	{
		assert(m_surfaceType == SURFACE_PALETTE_8BIT);
		assert(x < m_width && y < m_height);
		m_pPixels[ ( (y)*(m_usedPitch+m_pitchOffset)+x)] = color;
	}

	glColorBytes GetPixel( int x, int y )
	{
		switch (m_surfaceType)
		{
		case SURFACE_PALETTE_8BIT:
			return m_palette[m_pPixels[ ( ((m_height-1)-y)*(m_usedPitch+m_pitchOffset)+x)]];
			break;

		case SURFACE_RGBA:
			return *((glColorBytes*)((m_pPixels+(y*m_usedPitch+x*4))));
			break;

		case SURFACE_RGB:
		{
			glColorBytes temp;
			memcpy(&temp, (glColorBytes*)((m_pPixels + (y*m_usedPitch + x * 3))), 3);
			temp.a = 255;
			return temp;
		}
			break;

		default:
			assert(!"Unhandled pixel type... SSSSEEEEETTTHH!");
		}

		return glColorBytes(0,0,0,255);
	}

	bool GetUsesAlpha() {return m_bUsesAlpha;}
	void SetUsesAlpha(bool bNew) {m_bUsesAlpha = bNew;}

	int GetPitch() {return m_usedPitch+m_pitchOffset;}
	bool SetPaletteFromBMP(const string fName, eColorKeyType colorKey);
	void SetColorKeyType(eColorKeyType colorKey);
	eColorKeyType GetColorKeyType() {return m_colorKeyType;}
	glColorBytes GetColorKeyColor();
	int GetColorKeyPaletteIndex() { return m_colorKeyPaletteIndex; } //used for 8 bit palettes that have a color key
	glColorBytes * GetPalette() {return m_palette;}
	int GetPaletteColorCount() {return m_paletteColors;}
	bool IsPaletteTheSame(glColorBytes *palette, int colorCount);
	bool GetModified() {return m_bModified;}
	void SetModified(bool bNew) {m_bModified = bNew;}
	void FlipY();
	void FlipX(); 
	void Rotate90Degrees(bool bRotateLeft);
	BMPImageHeader BuildBitmapHeader();
	BMPImageHeader BuildBitmapHeader8bit();
	void BlitFromScreen(int dstX, int dstY, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/); //deprecated
	void BlitFromScreenFixed(int dstX, int dstY, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/);
	void WriteRawDataOut(string fileName);
	void WriteBMPOut(string fileName);
	uint8* WritePNGToMemory(int compressionLevel, int &outSize); //NOTE: You need to delete[] the data when done with it!
	void WritePNGOut(string fileName, int compressionLevel);
	void FillAlphaBit(unsigned char alpha);
	void Scale(int newX, int newY); //does simple linear scaling
	void SetForceBlackAndWhiteOnBmpPalettes(bool bNew) { m_bForceBlackAndWhiteOnBmpPalettes = bNew; }
	float GetAverageLumaFromRect(const CL_Vec2i vAreaPos, const CL_Vec2i vAreaSize); //send the upper left, and the width/height you want.  Measures brightness of pixels
	float GetAverageComplexityFromRect(const CL_Vec2i vAreaPos, const CL_Vec2i vAreaSize); //measures variation between pixels
	void FlipRedAndBlue(); //I needed this to fix colors before sending camera capture to an OGL surface

	void RemoveTrueBlack(uint8 minimumLuma);

private:

	SoftSurface(const SoftSurface&); //don't allow copy operation.  Use Blit or Surface::CreateFromSoftSurface instead

	void CheckDinkColorKey();
	void BlitRGBAFrom8Bit( int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/ );
	void BlitRGBAFromRGBA( int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/ );
	
	glColorBytes GetFinalRGBAColorWithColorKey(glColorBytes color)
	{
		if (m_colorKeyType == COLOR_KEY_NONE) return color;
		
		if (color.Compare(GetColorKeyColor())) return glColorBytes(0,0,0,0);

		return color;
	}
	uint8 * GetPointerToPixel(int x, int y)
	{
		switch(m_surfaceType)
		{
		case SURFACE_PALETTE_8BIT:
			return m_pPixels + ( ((m_height-1)-y)*(m_usedPitch+m_pitchOffset)+x);

		case SURFACE_RGBA:
		case SURFACE_RGB:
			return m_pPixels + m_usedPitch*y+(x*m_bytesPerPixel);
		
		default:
			assert(!"Unknown pixel type");
		}
		return 0;
	}

	void Blit8BitFrom8Bit( int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/ );
	void LoadPaletteDataFromBMPMemory(uint8 *pPaletteData, int colors);
	void IncreaseMemCounter(int mem);
	void Blit8BitFromRGBA( int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/ );
	int RGBAToPalette(const glColorBytes &color);
	bool RLE8BitDecompress(uint8 *pDst, uint8 *pSrc, int dstSize, int srcSize);
	void PreMultiplyAlpha();
	void ConvertCheckboardToAlpha(glColorBytes * pImg);
	bool IsCheckerboardAlphaShadowPixel(const glColorBytes * aPixel);
	bool IsCheckerboardSolidShadowPixel(glColorBytes * pImg, int x, int y, const glColorBytes & aPixel);
	void FadeCheckerboardAlphaPixel(glColorBytes * aDestination, const glColorBytes& aSource);
	bool LoadBMPTexture(uint8 *pMem);
	bool LoadBMPTextureCheckerBoardFix(uint8 *pMem);
#ifdef RT_PNG_SUPPORT
	bool LoadPNGTexture(uint8 *pMem, int inputSize, bool bApplyCheckerBoardFix);
	void LoadPaletteDataFromPNG(png_structp png_ptr, png_infop info_ptr);
	void ParseRGBA(const png_structp& png_ptr, const png_infop& info_ptr);
#endif
	
	bool LoadRTTexture(uint8 *pMem);
	void BlitRGBFromRGBA( int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/ );
	void BlitRGBAFromRGB(int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/);
	void BlitRGBFromRGB(int dstX, int dstY, SoftSurface *pSrc, int srcX /*= 0*/, int srcY /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/);

	eSurfaceType m_surfaceType;
	int m_width, m_height;
	uint8 *m_pPixels;
	int m_bytesPerPixel;
	int m_usedPitch; //read this many per line
	int m_pitchOffset; //filler to add to pitch for 32 bit byte boundries, bmp's need to know this, 0 if unused
	bool m_bUsesAlpha; //hint for the GL renderer
	eColorKeyType m_colorKeyType;
	glColorBytes m_palette[C_MAX_PALETTE_SIZE];
	int m_paletteColors;
	int m_colorKeyPaletteIndex;
	bool m_bModified;
	int m_memUsed;
	bool m_bAutoPremultiplyAlpha;
	bool m_bHasPremultipliedAlpha;
	glColorBytes m_customColorKey;
	bool m_bForceBlackAndWhiteOnBmpPalettes; //applicable to 8 bit bmp loading only

	//used only for RTTEX textures
	int m_originalWidth,m_originalHeight;
};

#endif // SoftSurface_h__