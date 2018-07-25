#include "PlatformSetup.h"
#include <time.h>
#include <stdlib.h>
#include <AS3/AS3.h>
//#include <Flash++.h>
#include "App.h"
#include "util/MiscUtils.h"

#ifndef _CONSOLE
	#include "GLFlashAdaptor.h"
#endif
//using namespace AS3::ui;

/*
Functions beginning with Native_ are called from Flash, so don't change the names!

If you add any new functions to be called from flash, keep in mind you must edit the exports.txt file and add them
for each project.

-Seth (seth@rtsoft.com)
*/

uint32_t g_nativeScreenWidth = 0;
uint32_t g_nativeScreenHeight = 0;

#ifdef _CONSOLE

	//NOTE:  I don't think this console stuff works anymore.  If you want a "console app" in Flash, you'll have to
	//fix it and disable all the GLES stuff.

	App g_App; //the app level App.cpp that's assumed to exist that drives the whole app
	App * GetApp() {return &g_App;}

	bool IsLargeScreen()
	{
		return true; 
	}

	bool IsTabletSize()
	{
		return false;
	}

#endif

int GetPrimaryGLX() {return g_nativeScreenWidth;}
int GetPrimaryGLY() {return g_nativeScreenHeight;}

extern "C" void Native_Render()
{
	#ifndef _CONSOLE
	GetApp()->Draw();
	#endif
}

extern "C" void Native_OnGotFocus()
{
	#ifndef _CONSOLE
	GetBaseApp()->OnEnterForeground();
	#endif
}

extern "C" void Native_OnLostFocus()
{
	#ifndef _CONSOLE
	GetBaseApp()->OnEnterBackground();
	#endif
}

extern "C" void Native_SendGUIEx()
{
	
	#ifndef _CONSOLE
	int msg;
	float xPos,yPos;
	int touchID;

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"%0 = Console.current.m_parmInt1;"
		"%1 = Console.current.m_parmFloat1;"
		"%2 = Console.current.m_parmFloat2;"
		"%3 = Console.current.m_parmInt2;"
		: "=r"( msg),"=r"( xPos),"=r"( yPos), "=r"( touchID) :
	);
	
	//LogMsg("Native_SendGUIEx got %d, %d, %.2f,  %.2f", msg, touchID, xPos, yPos);
	
	//convert to proton's key layout if needed
	if (msg == MESSAGE_TYPE_GUI_CHAR_RAW)
	{
		switch (int(xPos))
		{
		case 37: xPos = VIRTUAL_KEY_DIR_LEFT; break;
		case 38: xPos = VIRTUAL_KEY_DIR_UP; break;
		case 39: xPos = VIRTUAL_KEY_DIR_RIGHT; break;
		case 40: xPos = VIRTUAL_KEY_DIR_DOWN; break;
		case 16: xPos = VIRTUAL_KEY_SHIFT; break;
		case 17: xPos = VIRTUAL_KEY_CONTROL; break;
		case 112: xPos = VIRTUAL_KEY_F1; break;
		case 113: xPos = VIRTUAL_KEY_F2; break;
		case 114: xPos = VIRTUAL_KEY_F3; break;
		case 115: xPos = VIRTUAL_KEY_F4; break;
		case 116: xPos = VIRTUAL_KEY_F5; break;
		case 117: xPos = VIRTUAL_KEY_F6; break;
		case 118: xPos = VIRTUAL_KEY_F7; break;
		case 119: xPos = VIRTUAL_KEY_F8; break;
		case 120: xPos = VIRTUAL_KEY_F9; break;
		case 121: xPos = VIRTUAL_KEY_F10; break;
		case 27: xPos = VIRTUAL_KEY_BACK; break;


		default:;
		}
	}
	

	GetMessageManager()->SendGUIEx((eMessageType)msg, xPos, yPos, touchID);
	
	#endif
}

extern "C" void Native_Update()
{
	GetApp()->Update();

#ifndef _CONSOLE
	while (!GetBaseApp()->GetOSMessages()->empty())
	{
		OSMessage m = GetBaseApp()->GetOSMessages()->front();
		GetBaseApp()->GetOSMessages()->pop_front();
#ifdef _DEBUG
		LogMsg("Got OS message %d, %s", m.m_type, m.m_string.c_str());
#endif
		switch (m.m_type)
		{
		case OSMessage::MESSAGE_CHECK_CONNECTION:
			//pretend we did it
			GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);
			break;
		case OSMessage::MESSAGE_OPEN_TEXT_BOX:
			break;

		case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
			
			SetIsUsingNativeUI(false);
			break;

		case OSMessage::MESSAGE_SET_FPS_LIMIT:
			//fpsTimerLoopMS = (int) (1000.0f/m.m_x);
			break;

		case OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ:
			{
			}
			break;
		}
	}
	#endif

}


int main(int argc,char **argv) 
{
	
#ifdef _DEBUG

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"Console.current.SetDebugMode(%0);"
		: : "r"(true)
		);

#endif

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"Console.current.SetDebugMode(%0);"
		: : "r"(true)
		);

#ifndef _CONSOLE

    int stagewidth, stageheight;

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"import com.adobe.flascc.CModule;\n"
		"import flash.display.Bitmap;\n"
		"import flash.display.BitmapData;\n"
		"%0 = Console.screenWidth;\n"
		"%1 = Console.screenHeight;\n"
		: "=r"(g_nativeScreenWidth), "=r"(g_nativeScreenHeight) :
	);
  
	LogMsg("Screensize: %d, %d", GetPrimaryGLX(), GetPrimaryGLY());
	
	
			GLFlashAdaptor_Initialize();
			SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
 #endif

	#ifndef _CONSOLE
		if (!GetBaseApp()->Init())
		{
			LogError("Unable to init app");
			return -1;
		}
		#else
		if (!GetApp()->Init())
		{
			LogError("Unable to init app");
			return -1;
		}

		GetApp()->Update();
	#endif

	
#ifndef _CONSOLE
	if (GetFakePrimaryScreenSizeX() == 0)
	{
		//without this, RTBasicApp never sets up its screen.  But RTSimpleApp does, so we don't want this happening with that.
		//Waiting to test on a real device to really nail down rotation/portait mode, it's sort of hardcoded to landscape now.. I think..
		SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	}
#endif
	


}
