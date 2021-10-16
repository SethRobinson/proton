#ifndef video_dx_h__
#define video_dx_h__

#include "PlatformSetup.h"
#include "Math/rtRect.h"

class SoftSurface;
class Surface;

typedef struct _DDCOLORKEY
{
	uint32       dwColorSpaceLowValue;   // low boundary of color space that is to
	// be treated as Color Key, inclusive
	uint32       dwColorSpaceHighValue;  // high boundary of color space that is
	// to be treated as Color Key, inclusive
} DDCOLORKEY;

typedef struct _DDBLTFX
{
	uint32       dwSize;                         // size of structure
	uint32       dwDDFX;                         // FX operations
	uint32       dwROP;                          // Win32 raster operations
	uint32       dwDDROP;                        // Raster operations new for DirectDraw
	uint32       dwRotationAngle;                // Rotation angle for blt
	uint32       dwZBufferOpCode;                // ZBuffer compares
	uint32       dwZBufferLow;                   // Low limit of Z buffer
	uint32       dwZBufferHigh;                  // High limit of Z buffer
	uint32       dwZBufferBaseDest;              // Destination base value
	uint32       dwZDestConstBitDepth;           // Bit depth used to specify Z constant for destination

	DDCOLORKEY  ddckDestColorkey;               // DestColorkey override
	DDCOLORKEY  ddckSrcColorkey;                // SrcColorkey override

	uint32   dwFillColor;                    // color in RGB or Palettized
	uint32   dwFillDepth;                    // depth value for z-buffer
	uint32   dwFillPixel;                    // pixel value for RGBA or RGBZ

} DDBLTFX;

typedef DDBLTFX * LPDDBLTFX;

enum eTransparencyType
{
	TRANSPARENT_NONE,
	TRANSPARENT_BLACK,
	TRANSPARENT_WHITE,
	TRANSPARENT_MAGENTA,
	TRANSPARENT_CUSTOM_COLOR //set in advance with SetCustomTransparency

};



//blt flags
#define DDBLT_KEYSRC                            0x00008000l
#define DDBLT_COLORFILL                         0x00000400l
#define DDBLT_WAIT                              0x01000000l


#define DD_OK                                   0L
#define DD_FALSE                                1L
/****************************************************************************
*
* BLTFAST FLAGS
*
****************************************************************************/

#define DDBLTFAST_NOCOLORKEY                    0x00000000
#define DDBLTFAST_SRCCOLORKEY                   0x00000001
#define DDBLTFAST_DESTCOLORKEY                  0x00000002
#define DDBLTFAST_WAIT                          0x00000010
#define DDBLTFAST_DONOTWAIT  


//my fake DD wrapper that really uses GL under the hood
class IDirectDrawSurface
{
public:

	enum eMode
	{
		MODE_NORMAL, //8 bit image only
		MODE_PRIMARY_GL, //just a dummy surface, blitting to it goes straight to the screen
		MODE_SHADOW_GL //keep an 8 bit version but cache a HW GL surface as well
	};
	
	IDirectDrawSurface();
	~IDirectDrawSurface();

 int Blt(rtRect32 *pDestRect, IDirectDrawSurface * pSrcSurf, rtRect32 *pSrcRect, uint32 flags, DDBLTFX *pFX);
 int BltFast(int x, int y, IDirectDrawSurface *pSrcSurf, rtRect32 *pSrcRect, uint32 dwTrans);;
 void UpdateLastUsedTime();
 void UpdateShadowSurface();
 Surface * GetGLSuface(); //update it and send back a valid surface if possible

 Surface *m_pGLSurf;
 eMode m_mode;
 SoftSurface *m_pSurf;
 int m_gameTickOfLastUse;


};

typedef class IDirectDrawSurface *LPDIRECTDRAWSURFACE;

IDirectDrawSurface * InitOffscreenSurface(int x, int y, IDirectDrawSurface::eMode mode = IDirectDrawSurface::MODE_NORMAL, bool bHiColor = false, SoftSurface * pSurfaceToCopyFrom = NULL);

extern LPDIRECTDRAWSURFACE lpDDSBack;

void dderror(int hErr);
bool InitializeVideoSystem();
void KillVideoEngine();
void GetSizeOfSurface(IDirectDrawSurface *pdds, int *pX, int *pY);

IDirectDrawSurface * LoadBitmapIntoSurface(const char *pName, eTransparencyType trans= TRANSPARENT_NONE, IDirectDrawSurface::eMode mode = IDirectDrawSurface::MODE_SHADOW_GL, byte *pMem = NULL, int pMemSize = 0,
	bool bUseCheckerboardFix = false);

#endif // video_dx_h__