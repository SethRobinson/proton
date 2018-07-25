/*
 *  RenderUtils.h
 *  
 *
 *  Created by Seth Robinson on 3/18/09.
 *  Copyright 2009 Robinson Technologies. All rights reserved.
 *
 */

#ifndef RenderUtils_h__
#define RenderUtils_h__
#include "Math/rtRect.h"
#include "util/MathUtils.h"

class Surface;
class Entity;


#define CLEAR_GL_ERRORS() while (glGetError());


#ifdef _DEBUG
#define CALL_THE_QGL_VERSION_OF_glGetError glGetError
#define CHECK_GL_ERROR() \
{ \
	GLenum __error = glGetError(); \
	if(__error) { \
		LogError("OpenGL error 0x%04X (%d) in %s file %s:%d\n", __error,__error, __FUNCTION__, __FILE__, __LINE__); \
	} \
}

#else
#define CHECK_GL_ERROR() 

#endif


#define C_APP_FOV 45.0f


void RenderGLTriangle();
void RenderTexturedGLTriangle();
void RenderTexturedRectangle(float RectSize=1.0f);
void RenderSpinningTriangle();

/**
 * Returns the current height of the scene viewport. Usually this is the same as the height
 * of the application window. But it can be also different if the fake screen size mode
 * is used.
 */
int GetScreenSizeY();
/**
 * Returns the current width of the scene viewport. Usually this is the same as the width
 * of the application window. But it can be also different if the fake screen size mode
 * is used.
 */
int GetScreenSizeX();
/**
 * Returns the same as \c GetScreenSizeY() but as a float.
 */
float GetScreenSizeYf();
/**
 * Returns the same as \c GetScreenSizeX() but as a float.
 */
float GetScreenSizeXf();
/**
 * Constructs a vector from the values returned by \c GetScreenSizeXf() and \c GetScreenSizeYf().
 */
CL_Vec2f GetScreenSize();

void SetupScreenInfo(int x, int y, int orientation);
bool SetupScreenInfoIPhone(int interfaceOrientation);

void ConvertCoordinatesIfRequired(int &xPos, int &yPos);
void ConvertCoordinatesIfRequired(float &xPos, float &yPos);
void ConvertCoordinatesIfRequired(double &xPos, double &yPos); //needed for 64 bit iOS builds
bool NeedsOrthoSet();
void ResetOrthoFlag();
void SetOrthoModeFlag();

bool InLandscapeGUIMode(); //use this for things solely about screen size, so windows fake landscape modes can test GUI things
int GetOrientation(); //if three, iPhone is in landscape mode and things will be rotated
float SinPulseByMS(int ms); //returns -1 to 1 over the millsecond interval given, good for quickie effects of things bobbing or rotating
//when you don't want to setup a real timer
float SinGamePulseByMS(int ms); //like above, but will pause when the game is paused
float SinToZeroToOneRange(float sinIn); //a helper to use with the above functions when you just want 0-1 range.  (Do SinToZeroToOneRange(SinGamePulseByMS(4000)); for example

uint32 ColorCombineMix(uint32 c1, uint32 c2, float progress);

/**
 * Performs a "linear burn" operation for the colors \a c1 and \a c2.
 * This is achieved by summing the color component values together and
 * subtracting 255 from the result. The result of this is then clamped
 * to the accepted range of [0, 255]. Note that this operation is also
 * performed for the alpha channel.
 * 
 * Finally the alpha value is multiplied with \a alphaMod.
 * 
 * http://en.wikipedia.org/wiki/Blend_modes#Dodge_and_burn
 * 
 * \return The final resulting color of the operation.
 */
uint32 ColorCombine(uint32 c1, uint32 c2, float alphaMod = 1.0f);
void SetLockedLandscape(bool bNew);
bool GetLockedLandscape();
void  ScreenToWorld(CL_Vec2f pt, CL_Vec3f *pReturnA, float dist);

float GetProtonPixelScaleFactor(); //for iOS's weird scale factor thing, so we can convert the rez to normal pixels right
void SetProtonPixelScaleFactor(float scale);

//if you send in a NULL projection matrix, then Proton will guess, which is usually right unless you've setup your own somewhere when
//doing the 3D
CL_Vec3f GetOGLPos(int x, int y, float z, CL_Vec3f *pNormalOut, CL_Mat4f *pModelMatrix, CL_Mat4f *pModelProjectionMatrix = NULL);
bool CanRotateTo(int orientation);
bool GetIsRotating();
void SetIsRotating(bool bNew) ;
bool GetIsUsingNativeUI();
void SetIsUsingNativeUI(bool bNew);
Entity * GetEntityWithNativeUIFocus();
void SetEntityWithNativeUIFocus(Entity *pEnt, bool bSendLosingFocusMessage = true);
uint32 GetBrightColor();
bool IsLargeScreen();
float iPhoneMapX(float x);//enter coords for iPhone, if iPad, will convert to relative position (makes porting easier)
float iPhoneMapY(float y); //enter coords for iPhone, if iPad, will convert to relative position (makes porting easier)
CL_Vec2f iPhoneMap( CL_Vec2f vPos ); //enter coords for iPhone, if iPad, will convert to relative position (makes porting easier)
CL_Vec2f iPhoneMap( float x, float y ); //enter coords for iPhone, if iPad, will convert to relative position (makes porting easier)

float iPadMapY( float y );
float iPadMapX( float x );
CL_Vec2f iPadMap( CL_Vec2f vPos );
CL_Vec2f iPadMap( float x, float y );

//like above, but force iPad's/large screens to return numbers 2x of iPhone, useful for sharing the 2x media between iPhone4 and ipad
float iPhoneMapX2X( float x );
float iPhoneMapY2X( float y );
CL_Vec2f iPhoneMap2X( CL_Vec2f vPos );
CL_Vec2f iPhoneMap2X( float x, float y );
void InitBaseScreenSizeFromPrimary(); //to be called after GetPrimaryGLY() is valid (Only needed on iOS?)

void SetupFakePrimaryScreenSize(int x, int y);
int GetFakePrimaryScreenSizeX();
int GetFakePrimaryScreenSizeY();
int GetOriginalScreenSizeY();
int GetOriginalScreenSizeX();
void UndoFakeScreenSize();
void RedoFakeScreenSize();
bool IsTabletSize();
rtRectf ConvertFakeScreenRectToReal(rtRectf r);
rtRectf ConvertFakeScreenRectToReal(rtRectf r, float aspectRatioModX, float aspectRatioModY);
eOrientationMode GetForcedOrientation();
void SetForcedOrientation(eOrientationMode orientation);
bool NeedToUseFakeScreenSize(); //true if using a fake (stretched to device) screen and it's a different size than our primary screen
CL_Rectf GetScreenRect();
float SinPulseByCustomTimerMS(int ms, unsigned int timerMS);
void RenderTexturedGLTriangleWithDrawElements();
float GetFadeAlphaFromTime(int curTimeMS, int totalTimeMS, int fadeInMS, int fadeOutMS);
float GetDeviceSizeDiagonalInInches(); //returns 0 if unknown
void SetDeviceSizeDiagonalInInches(float sizeInInches); //guesses if unknown.  If GetDeviceSizeDiagonalInInches() == 0, it guessed.
int GetDevicePixelsPerInchDiagonal();

#endif // RenderUtils_h__
