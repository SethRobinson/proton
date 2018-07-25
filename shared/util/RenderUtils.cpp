/*
 *  RenderUtils.cpp
 *  
 *
 *  Created by Seth Robinson on 3/18/09.
 *  Copyright 2009 Robinson Technologies. All rights reserved.
 *
 */

#include "PlatformPrecomp.h"
#include "RenderUtils.h"
#include "BaseApp.h"

//defaults
int g_screenSizeY = GetPrimaryGLY();
int g_screenSizeX = GetPrimaryGLX();
int g_fakePrimaryScreenSizeX = 0;
int g_fakePrimaryScreenSizeY = 0;
int g_originalScreenSizeX = 0;
int g_originalScreenSizeY = 0;
float g_deviceDiagonalSizeInInches = 0;


float g_protonPixelScaleFactor = 1.0f; //only used by iOS

eOrientationMode g_forcedOrientation = ORIENTATION_DONT_CARE;
int g_orientation = ORIENTATION_PORTRAIT;
bool g_lockedLandscape = false;

eOrientationMode GetForcedOrientation() {return g_forcedOrientation;}
void SetForcedOrientation(eOrientationMode orientation) {g_forcedOrientation = orientation;}

void SetProtonPixelScaleFactor(float scale)
{
	#ifdef RT_DISABLE_RETINA_ON_IPAD
		if (IsIPAD() && scale == 2.0f)
		{
			//it's an ipad3 actually.  Do we want retina or not?
			scale = 1.0f;
		}
	#endif
    g_protonPixelScaleFactor = scale;
}

float GetProtonPixelScaleFactor()
{
    return g_protonPixelScaleFactor;
}

void InitBaseScreenSizeFromPrimary()
{
    //to be called after GetPrimaryGLY() is valid (Only needed on iOS?)
    g_screenSizeY = GetPrimaryGLY();
    g_screenSizeX = GetPrimaryGLX();
}

float GetDeviceSizeDiagonalInInches()
{
	return g_deviceDiagonalSizeInInches;
}

void SetDeviceSizeDiagonalInInches(float sizeInInches)
{
	g_deviceDiagonalSizeInInches = sizeInInches;
}

int GetDevicePixelsPerInchDiagonal()
{
	static int cachedPixelsPerInch = 0;

	if (cachedPixelsPerInch == 0)
	{
		//one time look up
		
		//First, if SetDeviceSizeDiagonalInInches() was used earlier by the native side, we're good to go.  If not, we'll
		//guess.

		if (g_deviceDiagonalSizeInInches == 0)
		{
			//guess.
			if (IsIphone4Size || IsIphone5Size) cachedPixelsPerInch = 326;
			if (IsIPADSize)	cachedPixelsPerInch = 132;
			if (IsIPADRetinaSize) cachedPixelsPerInch = 264;
			if (IsIphoneSize) cachedPixelsPerInch = 163;
			if (IsXoomSize) cachedPixelsPerInch = 149;
			if (IsDroidSize) cachedPixelsPerInch = 265;
			if (IsNexusOneSize) cachedPixelsPerInch = 235;
			if (IsPlaybookSize) cachedPixelsPerInch = 169;
			if (IsNexus7B || IsNexus7BActual) cachedPixelsPerInch = 273;
			if (IsOptimusHDSize)  cachedPixelsPerInch = 326;
			if (IsNexus7) cachedPixelsPerInch = 216;
			if (IsNexus5Actual) cachedPixelsPerInch = 445;
			if (IsGalaxyNoteActual) cachedPixelsPerInch = 550;//guessing
			if (IsLGG4Actual) cachedPixelsPerInch = 600;//guessing

			if (IsHDSize && !IsDesktop()) cachedPixelsPerInch = 445; //guessing
			
			if (cachedPixelsPerInch == 0)
			{
				//we really have no clue
				cachedPixelsPerInch = 163;

				if (!IsDesktop() && GetScreenSizeXf() >= 1920)
				{
					cachedPixelsPerInch = 445;
				}
			}
		} else
		{
			cachedPixelsPerInch = (int) (GetScreenSize().length() / g_deviceDiagonalSizeInInches);
		}
	}

	return cachedPixelsPerInch;	
}

void RenderGLTriangle()
{
	//let's draw a simple triangle 
	/*
	 2
	0 1
	*/

	const float triSize = 0.4f;

	GLfloat vVertices[] = 
	{
		-triSize, -triSize, 0.0f,
		triSize, -triSize, 0.0f,
		0.0f, triSize, 0.0f
	};
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT,  0, vVertices);

	unsigned int rgba = MAKE_RGBA(255,0,0,255);
	glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glDrawArrays(GL_TRIANGLES, 0, 3);
	//fix the color back to white
	glColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);


	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnable(GL_TEXTURE_2D);
	CHECK_GL_ERROR();
}


void RenderSpinningTriangle()
{

	glPushMatrix();
	
    glLoadIdentity();
	glTranslatef(0,0,-2);
	glRotatef(float( (GetBaseApp()->GetGameTick()/10) %360) , 0, 1, 0); //rotate it
	glDisable(GL_CULL_FACE); //so we can see the back of the triangle too
	RenderGLTriangle();
	glEnable(GL_CULL_FACE); //so we can see the back of the triangle too
	glPopMatrix();
	CHECK_GL_ERROR();
}

void RenderTexturedGLTriangle()
{


	glPushMatrix();
	//glLoadIdentity();
	//glTranslatef(0,0,-2);
	glRotatef(float( (GetBaseApp()->GetGameTick()/10) %360) , 0, 1, 0); //rotate it
	glDisable(GL_CULL_FACE); //so we can see the back of the triangle too



	//let's draw a simple triangle 
	/*
	 2
	0 1
	*/

	const float triSize = 0.4f;

	GLfloat vVertices[] = 
	{
		-triSize, -triSize, 0.0f,
		triSize, -triSize, 0.0f,
		0.0f, triSize, 0.0f
	};

	GLfloat vTexCoords[] = 
	{
		0,0,
		1, 0,
		0.5,1
	};
	//unsigned int rgba = MAKE_RGBA(255,255,255,255);
	//glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);
	
	glVertexPointer(3, GL_FLOAT,  0, vVertices);
	glTexCoordPointer(2, GL_FLOAT,  0, vTexCoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE); //so we can see the back of the triangle too

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	CHECK_GL_ERROR();

	glEnable(GL_CULL_FACE); //so we can see the back of the triangle too
	glPopMatrix();
	CHECK_GL_ERROR();
}



void RenderTexturedGLTriangleWithDrawElements()
{

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0,0,-2);
	glRotatef(float( (GetBaseApp()->GetGameTick()/10) %360) , 0, 1, 0); //rotate it
	glDisable(GL_CULL_FACE); //so we can see the back of the triangle too


	//let's draw a simple triangle 
	/*
	 2
	0 1
	*/

	const float triSize = 0.4f;

	GLfloat vVertices[] = 
	{
		4000,4000,4000,
		4000,4000,4000,

		-triSize, -triSize, 0.0f,
		triSize, -triSize, 0.0f,
		0.0f, triSize, 0.0f
	};

	GLfloat vTexCoords[] = 
	{
		0,0,
		0,0,
		0,0,
		1, 0,
		0.5,1
	};
	//unsigned int rgba = MAKE_RGBA(255,255,255,255);
	//glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);
	
#ifdef RT_GLES_ADAPTOR_MODE
	//glVertexPointerEx(3, GL_FLOAT,  0, vVertices, 5);

	//make sure doing it the dumb way still works too
	glVertexPointer(3, GL_FLOAT,  0, vVertices);
#else
	glVertexPointer(3, GL_FLOAT,  0, vVertices);
#endif
	glTexCoordPointer(2, GL_FLOAT,  0, vTexCoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE); //so we can see the back of the triangle too

	//glDrawArrays(GL_TRIANGLES, 0, 3);
	
	uint16 index[5];
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
	index[3] = 3;
	index[4] = 4;

	//setup to start at an offset, needed this to test something
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, &index[2]);
	
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	CHECK_GL_ERROR();

	glEnable(GL_CULL_FACE); //so we can see the back of the triangle too
	glPopMatrix();
	CHECK_GL_ERROR();
}

void RenderTexturedRectangle(float RectSize)
{
	//let's draw a simple rectangle
	/*
	3 2
	0 1
	*/

	GLfloat vVertices[] = 
	{
		-RectSize, -RectSize, 0.0f,
		RectSize, -RectSize, 0.0f,
		RectSize, RectSize, 0.0f,
		-RectSize, RectSize, 0.0f
	};

	GLfloat vTexCoords[] = 
	{
		0,0,
		1,0,
		1, 1,
		0, 1
	};
	//unsigned int rgba = MAKE_RGBA(255,255,255,255);
	//glColor4x( (rgba >>8 & 0xFF)*256,  (rgba>>16& 0xFF)*256, (rgba>>24& 0xFF)*256, (rgba&0xFF)*256);
	glVertexPointer(3, GL_FLOAT,  0, vVertices);
	glTexCoordPointer(2, GL_FLOAT,  0, vTexCoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}


void SetupFakePrimaryScreenSize(int x, int y)
{
	g_fakePrimaryScreenSizeX = x;
	g_fakePrimaryScreenSizeY = y;
	
	//recompute it
	SetupScreenInfo(GetScreenSizeX(), GetScreenSizeY(), GetOrientation());
};


void SetupOriginalScreenSize(int x, int y)
{
	g_originalScreenSizeX = x;
	g_originalScreenSizeY = y;

};

int GetOriginalScreenSizeX()
{
	return g_originalScreenSizeX;
}

int GetOriginalScreenSizeY()
{
	return g_originalScreenSizeY;
}

int GetFakePrimaryScreenSizeX()
{
	return g_fakePrimaryScreenSizeX;
}

int GetFakePrimaryScreenSizeY()
{
	return g_fakePrimaryScreenSizeY;
}

int g_undoFakeScreenSizeX = 0;
int g_undoFakeScreenSizeY = 0;

bool NeedToUseFakeScreenSize()
{
	if (g_fakePrimaryScreenSizeX == 0) return false;
	if (g_fakePrimaryScreenSizeX == GetPrimaryGLX()
		&& g_fakePrimaryScreenSizeY == GetPrimaryGLY())	 return false;
	return true;
}
void UndoFakeScreenSize()
{
	if (GetFakePrimaryScreenSizeX() == 0) return; //not used

	g_undoFakeScreenSizeX = g_fakePrimaryScreenSizeX;
	g_undoFakeScreenSizeY = g_fakePrimaryScreenSizeY;

	g_fakePrimaryScreenSizeX = 0;
	g_fakePrimaryScreenSizeY = 0;

	g_screenSizeX = GetOriginalScreenSizeX();
	g_screenSizeY = GetOriginalScreenSizeY();
	
	SetupOriginalScreenSize(0,0);

	SetupScreenInfo(g_screenSizeX,g_screenSizeY, GetOrientation());
	PrepareForGL(); //make sure any ortho settings are also updated
}


void RedoFakeScreenSize()
{

	if (g_undoFakeScreenSizeX)
	{
		SetupFakePrimaryScreenSize(g_undoFakeScreenSizeX, g_undoFakeScreenSizeY);
		SetupScreenInfo(GetOriginalScreenSizeX(), GetOriginalScreenSizeY(), GetOrientation());

		g_undoFakeScreenSizeX = 0;
		g_undoFakeScreenSizeY = 0;
	
		PrepareForGL(); //make sure any ortho settings are also updated
	}

}


int GetScreenSizeY()
{
	return g_screenSizeY;
}

int GetScreenSizeX()
{
	return g_screenSizeX;
}

float GetScreenSizeYf()
{
	return (float)g_screenSizeY;
}

float GetScreenSizeXf()
{
	return (float)g_screenSizeX;
}

CL_Vec2f GetScreenSize()
{
	return CL_Vec2f((float)g_screenSizeX, (float)g_screenSizeY);
}

CL_Rectf GetScreenRect()
{
	return CL_Rectf(0,0, (float)g_screenSizeX, (float)g_screenSizeY);
}

//true for anything larger than an iPhone
bool IsLargeScreen()
{
	return g_screenSizeX>480 || g_screenSizeY > 480;
}

bool IsTabletSize()
{
	if (g_screenSizeX>=1024 || g_screenSizeY >= 1024)
	{
		//it's big, but if the pixel density is high enough, it still counts
		//as small
		if (GetDevicePixelsPerInchDiagonal() > 273) //comon, even the ipad3 doesn't have that, must be a phone
		{
			return false;
		}
		return true;
	}
	return false;

}

int GetOrientation() {return g_orientation;}

bool GetLockedLandscape() {return g_lockedLandscape;}

void SetLockedLandscape(bool bNew)
{

	if (g_lockedLandscape != bNew)
	{
		g_lockedLandscape = bNew;
		
        
        if (!GetBaseApp()->GetManualRotationMode())
        {
            //the system handles rotation, we don't need to get fancy here
            return;
        }
        
        if (GetOrientation() == ORIENTATION_PORTRAIT || GetOrientation() == ORIENTATION_PORTRAIT_UPSIDE_DOWN)
		{

            if (GetPrimaryGLX() == 0) return; //not initted yet
			if (GetForcedOrientation() != ORIENTATION_DONT_CARE)
			{
				if (GetForcedOrientation() == ORIENTATION_PORTRAIT || GetForcedOrientation() == ORIENTATION_PORTRAIT_UPSIDE_DOWN )
				{
					//can't lock landscape, pretending we're portrait mode, hack for emulating screens that require we manually
					//rotate (usually for speed on certain devices)
					SetupScreenInfo(GetPrimaryGLX(),GetPrimaryGLY(), ORIENTATION_LANDSCAPE_LEFT);
					return;

				}
			}

			//flip 'er around
			SetupScreenInfo(GetPrimaryGLY(),GetPrimaryGLX(), ORIENTATION_LANDSCAPE_LEFT);
		}
	}
}

bool g_usingNativeUI = false;
Entity *g_pGUIWithGUIFocus = NULL;

Entity * GetEntityWithNativeUIFocus()
{
	return g_pGUIWithGUIFocus;
}

void SetEntityWithNativeUIFocus(Entity *pEnt, bool bSendLosingFocusMessage)
{
	
	if (pEnt == g_pGUIWithGUIFocus) return;

	if (bSendLosingFocusMessage && pEnt)
	{
		if (g_pGUIWithGUIFocus)
		{
			g_pGUIWithGUIFocus->GetShared()->CallFunctionIfExists("OnLosingNativeGUIFocus", NULL);
		}
	}
	
	g_pGUIWithGUIFocus = pEnt;
}

bool GetIsUsingNativeUI() {return g_usingNativeUI;}

void SetIsUsingNativeUI(bool bNew) 
{
	if (bNew == g_usingNativeUI) return;
	g_usingNativeUI = bNew;

	//broadcast it via a signal incase anybody cares
	VariantList v((uint32)bNew);
	GetBaseApp()->m_sig_native_input_state_changed(&v);
}


bool CanRotateTo(int orientation)
{
	if (orientation ==  4 || orientation == 3)
	{
		return true;
	}
	if (GetLockedLandscape()) return false;

	return true;
}

bool SetupScreenInfoIPhone(int interfaceOrientation)
{
	
	if (GetForcedOrientation() != ORIENTATION_DONT_CARE)
	{
		interfaceOrientation = GetForcedOrientation();
	}
	//note, this is slightly different from our usual orientation (UIDeviceOrientation) so let's convert it first
	int orientation = interfaceOrientation;
	
	if (!CanRotateTo(orientation)) return false;
	
	SetupScreenInfo(g_screenSizeX,g_screenSizeY,orientation); 	
	
	return false;
}

void SetupScreenInfo(int x, int y, int orientation)
{
	if (GetForcedOrientation() != ORIENTATION_DONT_CARE)
	{
		orientation = GetForcedOrientation();
	}

	SetupOriginalScreenSize(GetPrimaryGLX(), GetPrimaryGLY());

#ifdef _DEBUG
	//	LogMsg("Setting screen info to %d, %d, mode %d.  Original is %d, %d. Fake X is %d", x, y, orientation, GetOriginalScreenSizeX(), GetOriginalScreenSizeY(),
	//		GetFakePrimaryScreenSizeX());
#endif

		g_screenSizeX = x;
		g_screenSizeY = y;
		g_orientation = orientation;
	
		if (GetFakePrimaryScreenSizeX())
		{
			//recompute it using our fake information
			x = GetFakePrimaryScreenSizeX();
			y = GetFakePrimaryScreenSizeY();
		
			g_screenSizeX = x;
			g_screenSizeY = y;

		}

		NotifyOSOfOrientationPreference(eOrientationMode(orientation));
		if (IsBaseAppInitted())
		{
			GetBaseApp()->OnScreenSizeChange();
		}
}

void ConvertCoordinatesIfRequired(int &xPos, int &yPos)
{
	float x = (float)xPos;
	float y = (float)yPos;
	ConvertCoordinatesIfRequired(x, y);
	xPos = (int)x;
	yPos = (int)y;
}

void ConvertCoordinatesIfRequired(double &xPos, double &yPos)
{
    float x = (float)xPos;
    float y = (float)yPos;
    ConvertCoordinatesIfRequired(x, y);
    xPos = (int)x;
    yPos = (int)y;
}

void ConvertCoordinatesIfRequired(float &xPos, float &yPos)
{
//	LogMsg("Before converting, coords are %d, %d", int(xPos), int(yPos));	

	xPos *= GetProtonPixelScaleFactor();
	yPos *= GetProtonPixelScaleFactor();

	if (GetBaseApp()->GetManualRotationMode())
	{
		switch (GetOrientation())
		{

		case ORIENTATION_PORTRAIT:
			break;

		case ORIENTATION_PORTRAIT_UPSIDE_DOWN:
			yPos = float(GetOriginalScreenSizeY())-yPos;
			xPos = float(GetOriginalScreenSizeX())-xPos;
			break;

		case ORIENTATION_LANDSCAPE_LEFT:
			swap(xPos, yPos);
			yPos = float(GetOriginalScreenSizeX())-yPos;
			break;

		case ORIENTATION_LANDSCAPE_RIGHT:
			yPos = float(GetOriginalScreenSizeY())-yPos;
			swap(xPos, yPos);
			break;

		}
	}
	if (GetFakePrimaryScreenSizeX() != 0)
	{
		//remap to correct values
		//LogMsg("CurY: %.2f - old y: %.2f",  GetScreenSizeYf(), float(GetOriginalScreenSizeY()));
		//LogMsg("CurX: %.2f - old x: %.2f",  GetScreenSizeXf(), float(GetOriginalScreenSizeX()) );
		
		float OriginalX = (float)GetOriginalScreenSizeX();
		float OriginalY = (float)GetOriginalScreenSizeY();
		
		if (GetBaseApp()->GetManualRotationMode())
        {
            if (InLandscapeGUIMode())
            {
                swap(OriginalX, OriginalY);
            }
        }
		
		xPos = (float(xPos) * (GetScreenSizeXf()/OriginalX));
		yPos = (float(yPos) * (GetScreenSizeYf()/OriginalY));
	}
	
//	LogMsg("Converted coords to %d, %d", int(xPos), int(yPos));
}

bool g_needsOrthoModeSet = true;

bool NeedsOrthoSet() 
{
	return g_needsOrthoModeSet;
}

void ResetOrthoFlag()
{
	
	g_needsOrthoModeSet = true;
}

void SetOrthoModeFlag()
{
	g_needsOrthoModeSet = false;
}

bool InLandscapeGUIMode()
{
	return GetOrientation() == 3 || GetOrientation() == 4;
}

//returns a range of -1 to 1 with the cycle matching the MS sent in, based on a sin wave
float SinPulseByMS(int ms)
{
	int tick = GetBaseApp()->GetTick()%ms;
	return (float)(sin (   (float(tick)/float(ms))  *M_PI*2   ));
}

float SinPulseByCustomTimerMS(int ms, unsigned int timerMS)
{
	int tick = timerMS%ms;
	return (float)(sin (   (float(tick)/float(ms))  *M_PI*2   ));
}

float SinGamePulseByMS(int ms)
{
	int tick = GetBaseApp()->GetGameTick()%ms;
	return (float)(sin (   (float(tick)/float(ms))  *M_PI*2   ));
}

//a helper to use with the above functions when you just want 0-1 range
float SinToZeroToOneRange(float sinIn)
{
	return (sinIn+1.0f)/2;
}

//just makes a random brightish color
uint32 GetBrightColor()
{
	int color[3];

	int num = Random(2);

	if (Random(3))
	{
		color[num%3] = Random(80)+ (255-80); 

	} else
	{
		color[num%3] = Random(150)+ (255-150); 

	}
	color[(num+1)%3] = rt_min(255, Random(355 - (color[num%3])));
	color[(num+2)%3] =  rt_min(255,Random(455 - (color[num%3] +color[(num+1)%3] )));
	return MAKE_RGBA(color[0], color[1], color[2], 255);
}

//mix between c1 and c2 based on progress, which should be 0 to 1
uint32 ColorCombineMix(uint32 c1, uint32 c2, float progress)
{
	int r,g,b,a;

	//OPTIMIZE oh come on, optimize this - Seth

	r = int (float(GET_RED(c1)) + ((  float(GET_RED(c2)) - float(GET_RED(c1))  ) * progress));
	g = int (float(GET_GREEN(c1)) + ((  float(GET_GREEN(c2)) - float(GET_GREEN(c1))  ) * progress));
	b = int (float(GET_BLUE(c1)) + ((  float(GET_BLUE(c2)) - float(GET_BLUE(c1))  ) * progress));
	a = int (float(GET_ALPHA(c1)) + ((  float(GET_ALPHA(c2)) - float(GET_ALPHA(c1))  ) * progress));


	//LogMsg(PrintColor(MAKE_RGBA(r,g,b,a)).c_str());
	return MAKE_RGBA(r,g,b,a);
}

uint32 ColorCombine(uint32 c1, uint32 c2, float alphaMod)
{
	int r,g,b,a;

	if (c2 != MAKE_RGBA(255,255,255,255))
	{
		r = GET_RED(c1) - (255- GET_RED(c2));
		g = GET_GREEN(c1) - (255-  GET_GREEN(c2));
		b = GET_BLUE(c1) - (255-  GET_BLUE(c2));
		a = GET_ALPHA(c1) - (255-  GET_ALPHA(c2));

		r = rt_max(0, r); r = rt_min(255,r);
		g = rt_max(0, g); g = rt_min(255,g);
		b = rt_max(0, b); b = rt_min(255,b);
		a = rt_max(0, a); a = rt_min(255,a);
	} else
	{
		if (alphaMod == 1) return c1;
		r = GET_RED(c1);
		g = GET_GREEN(c1);
		b = GET_BLUE(c1); 
		a = GET_ALPHA(c1);
	}

	//LogMsg("%s and %s make %s", PrintColor(c1).c_str(), PrintColor(c2).c_str(), PrintColor(MAKE_RGBA(r,g,b, byte(float(a)*alphaMod))).c_str());
	return MAKE_RGBA(r,g,b, byte(float(a)*alphaMod)) ;
}

void  ScreenToWorld(CL_Vec2f pt, CL_Vec3f *pReturnA, float dist)
{
	float aspect = GetScreenSizeYf()/GetScreenSizeXf();

	const float tanffov =  tanf((float(C_APP_FOV) * M_PI) / 360.0f);

	float x =  pt.x / (GetScreenSizeXf()/2)-1.0f;
	float y = 1.0f- (pt.y / (GetScreenSizeYf()/2));

	x = tanffov * (x) / aspect;
	y =  tanffov * y;

	CL_Mat4f mat16, projMat16;
	glGetFloatv(GL_MODELVIEW_MATRIX, mat16);
	glGetFloatv(GL_PROJECTION_MATRIX, projMat16);

	mat16.inverse();
	
	projMat16.inverse();

	*pReturnA = CL_Vec3f(x,y,dist);
	
	CL_Vec4f vTemp4 = CL_Vec4f(    pReturnA->x,    pReturnA->y,  pReturnA->z, 1)*mat16;
	vTemp4 = vTemp4*projMat16;

	*pReturnA = CL_Vec3f(  vTemp4.x,  vTemp4.y, vTemp4.z);
}

CL_Vec3f GetOGLPos(int x, int y, float z, CL_Vec3f *pNormalOut, CL_Mat4f *pModelMatrix, CL_Mat4f *pModelProjectionMatrix)
{
		
	//this xmod/ymod handles some things we do with auto screen stretching if "fake screensize" is used

	float xmod = (float(GetOriginalScreenSizeX())/GetScreenSizeXf());
	float ymod = (float(GetOriginalScreenSizeY())/GetScreenSizeYf());

	if (GetBaseApp()->GetManualRotationMode())
	{
		
		switch (GetOrientation())
		{
		case ORIENTATION_LANDSCAPE_LEFT:
			y = GetScreenSizeY()-y;
			swap(x,y);
			swap(xmod,ymod);
			break;

		case ORIENTATION_LANDSCAPE_RIGHT:
			x = GetScreenSizeX()-x;
			swap(x,y);
			swap(xmod,ymod);
			break;

		case ORIENTATION_PORTRAIT:
			break;

		case ORIENTATION_PORTRAIT_UPSIDE_DOWN:
			y = GetScreenSizeY()-y;
			x = GetScreenSizeX()-x;
			break;
		}
	}
	
	//convert back into real screen coordinates if needed


	
	if (GetFakePrimaryScreenSizeX() != 0)
	{
		//remap to correct values
		x = (int) (float(x) * xmod);
		y = (int) (float(y) * ymod);

		if (GetBaseApp()->GetManualRotationMode())
		{

			switch (GetOrientation())
			{
			case ORIENTATION_LANDSCAPE_LEFT:
			case ORIENTATION_LANDSCAPE_RIGHT:
				//I have no idea what I'm doing but this fixes a problem with fake scaling and rotation
				swap(x,y);
				break;
			}
		}

	}
	


	GLint viewport[4];
	GLfloat modelview[16];
	GLfloat projection[16];
	GLfloat winX, winY;
	GLfloat posX, posY, posZ;

	*((CL_Mat4f*)modelview) = *pModelMatrix;
	
	if (pModelProjectionMatrix)
	{
		*((CL_Mat4f*)projection) = *pModelProjectionMatrix;
	} else
	{
		*((CL_Mat4f*)projection) = *GetBaseApp()->GetProjectionMatrix();
	}
	//glGetFloatv( GL_PROJECTION_MATRIX, projection );
	
	
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = GetPrimaryGLX();
	viewport[3] = GetPrimaryGLY();
	//glGetIntegerv( GL_VIEWPORT, viewport ); //doesn't work on android

	//Read the window z co-ordinate 
	//(the z value on that point in unit cube)		
	
	CL_Vec3f a,b;
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);
	a = CL_Vec3f(posX, posY, posZ);

	gluUnProject( winX, winY, 1, modelview, projection, viewport, &posX, &posY, &posZ);
	b = CL_Vec3f(posX, posY, posZ);

	CL_Vec3f n = b-a;
	n.normalize();
	if (pNormalOut)
	{
		*pNormalOut = n;
	}
	return a + (n* z);
}


//I see I'm truncating the floats to ints here, but I can't remember if I had a decent reason.  Oh well, this stuff
//is rather old don't use it

float iPhoneMapY( float y )
{
	return (int)(y*GetScreenSizeYf())/320.0f; //I cast to int because it helps with "middle of the pixel" jaggies
}

float iPhoneMapX( float x )
{
	return (int)(x*GetScreenSizeXf())/480.0f;  //I cast to int because it helps with "middle of the pixel" jaggies
}

CL_Vec2f iPhoneMap( CL_Vec2f vPos )
{
	return CL_Vec2f((float)(int) (vPos.x*GetScreenSizeXf()/480.0f), (float)(int) (vPos.y*GetScreenSizeYf()/320.0f));
}


CL_Vec2f iPhoneMap( float x, float y )
{
	return CL_Vec2f((float)(int) (x*GetScreenSizeXf()/480.0f), (float)(int) (y*GetScreenSizeYf()/320.0f));
}

float iPadMapY( float y )
{
	return (float)(int)(y*GetScreenSizeYf())/768.0f; //I cast to int because it helps with "middle of the pixel" jaggies
}

float iPadMapX( float x )
{
	return(float) (int)(x*GetScreenSizeXf())/1024.0f;  //I cast to int because it helps with "middle of the pixel" jaggies
}

CL_Vec2f iPadMap( CL_Vec2f vPos )
{
	return CL_Vec2f((float)(int) (vPos.x*GetScreenSizeXf()/1024.0f), (float)(int) (vPos.y*GetScreenSizeYf()/768.0f));
}

CL_Vec2f iPadMap( float x, float y )
{
	return CL_Vec2f((float)(int) (x*GetScreenSizeXf()/1024.0f), (float)(int) (y*GetScreenSizeYf()/768.0f));
}


float iPhoneMapX2X( float x )
{
	if (!IsLargeScreen()) return x;
	return (float)(int)(x*960)/480;
}


float iPhoneMapY2X( float y )
{
	if (!IsLargeScreen()) return y;
	return (float)(int)(y*649)/320;
}

CL_Vec2f iPhoneMap2X( CL_Vec2f vPos )
{
	if (!IsLargeScreen()) return vPos;
	
	return CL_Vec2f((float)(int) (vPos.x*960/480), (float)(int) (vPos.y*640/320));
}

CL_Vec2f iPhoneMap2X( float x, float y )
{
	if (!IsLargeScreen()) return CL_Vec2f(x, y);
	return CL_Vec2f((float)(int) (x*960/480), (float)(int) (y*640/320));
}

/*
rtRectf ConvertFakeScreenRectToReal(rtRectf r)
{
	if (GetFakePrimaryScreenSizeX() == 0) return r;

	float ratioy = ((float)GetPrimaryGLY()/(float)GetFakePrimaryScreenSizeY());
	float ratiox = ((float)GetPrimaryGLX()/(float)GetFakePrimaryScreenSizeX());

	float widthHold = r.GetWidth();
	float heightHold = r.GetHeight();

	r.top *= ratioy;
	r.left *= ratiox;

	r.right = widthHold*ratiox+r.left;
	r.bottom = heightHold*ratioy+r.top;

	return r;
}
*/

//old version.. looks wrong!



rtRectf ConvertFakeScreenRectToReal(rtRectf r)
{
	if (GetFakePrimaryScreenSizeX() == 0) return r;

	float primaryY = (float)GetPrimaryGLY();
	float primaryX = (float)GetPrimaryGLX();

	float fakeX = (float)GetFakePrimaryScreenSizeX();
	float fakeY = (float)GetFakePrimaryScreenSizeY();

#ifdef _DEBUG
//	LogMsg("ConvertFakeScreenRectToReal:  Primary: %d, %d, Fake: %d, %d", GetPrimaryGLX(), GetPrimaryGLY(),
//		GetFakePrimaryScreenSizeX(), GetFakePrimaryScreenSizeY());
#endif

	if(GetBaseApp()->GetManualRotationMode() && InLandscapeGUIMode())
	{
		swap(primaryX , primaryY);
	}
	
    float ratiox = (primaryX/fakeX);
	float ratioy = (primaryY/fakeY);

	float widthHold = r.GetWidth();
	float heightHold = r.GetHeight();

	r.top *= ratioy;
	r.left *= ratiox;

	r.right = widthHold*ratiox+r.left;
	r.bottom = heightHold*ratioy+r.top;

	return r;
}

rtRectf ConvertFakeScreenRectToReal(rtRectf r, float aspectRatioModX, float aspectRatioModY)
{
	if (GetFakePrimaryScreenSizeX() == 0) return r;

	float primaryY = (float)GetPrimaryGLY();
	float primaryX = (float)GetPrimaryGLX();

	float fakeX = (float)GetFakePrimaryScreenSizeX();
	float fakeY = (float)GetFakePrimaryScreenSizeY();

	if(GetBaseApp()->GetManualRotationMode() && InLandscapeGUIMode())
	{
		swap(primaryX , primaryY);
	}

	float ratiox = (primaryX/fakeX);
	float ratioy = (primaryY/fakeY);

	float widthHold = r.GetWidth();
	float heightHold = r.GetHeight();

	r.top *= ratioy;
	r.left *= ratiox;

	r.right = widthHold*ratiox+r.left;
	r.bottom = heightHold*ratioy+r.top;


	r.right *= aspectRatioModX;
	r.bottom *= aspectRatioModY;
	return r;
}


//a helper for calculating fadein/fadeout times

float GetFadeAlphaFromTime(int curTimeMS, int totalTimeMS, int fadeInMS, int fadeOutMS)
{
	//LogMsg("GetFadeAlpha: curTime: %d, total %d", curTimeMS, totalTimeMS);

	if (curTimeMS <= 0 || curTimeMS >= totalTimeMS) return 0;

	if (curTimeMS > totalTimeMS-fadeOutMS)
	{
		return 1.0f - ((float)( curTimeMS-(totalTimeMS-fadeOutMS))/(float)fadeOutMS);
	}

	if (curTimeMS < fadeInMS)
	{
		return (float)curTimeMS/(float)fadeInMS;		
	}

	return 1;
}


