#include "PlatformPrecomp.h"
#ifdef RT_WEBOS
#include "SDLMain.h"
#include "PDL.h"
#include "../win/app/main.h"
#include "BaseApp.h"
#include "../util/cJSON.h" //used for IAP
#include "../Manager/IAPManager.h"
#include "../util/cJSON.c" //Because I don't want to update all the projects with this file..., yes, Bad Seth

SDL_Surface *g_screen = NULL;
SDL_Joystick *g_pSDLJoystick = NULL;
CL_Vec3f g_accelHold = CL_Vec3f(0,0,0);

const char * GetBundleName();
const char * GetBundlePrefix();

 
bool g_isInForeground = true;
#ifdef RT_WEBOS_ARM

//need these for the real compile
#include <syslog.h>
#include <unistd.h>


void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );
	syslog(LOG_ERR, buffer, 0, LOG_USER);
	if (IsBaseAppInitted())
	{
		GetBaseApp()->GetConsole()->AddLine(buffer);
	}
}

#endif

#ifdef _WIN32

#include <time.h>

void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize,  traceStr, argsVA );
	va_end( argsVA );
	OutputDebugString(buffer);
	OutputDebugString("\n");
	if (IsBaseAppInitted())
	{
		GetBaseApp()->GetConsole()->AddLine(buffer);
	}
}
#endif


bool parseNullsepList(const char *nullsepList, int listBufferSize, char **arrayToFill, int numElements);

void IAPBuyItem(string m_item)
{
#ifdef _DEBUG
	LogMsg("Want to buy %s",m_item.c_str());
#endif

	if (GetDeviceOSVersion() < 3.0f)
	{
		//avoid a crash on older devices
		GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_SERVICE_UNAVAILABLE,0.0f);
		return;
	}
	PDL_ItemReceipt *itemReceipt = PDL_PurchaseItem(m_item.c_str(), 1, m_item.c_str());

	const char *itemReceiptJSON = NULL;
	if (itemReceipt)
	{
		itemReceiptJSON = PDL_GetItemReceiptJSON(itemReceipt);
	}

	if (!itemReceipt || !itemReceiptJSON)
	{
		LogMsg(("itemReceipt was null"));
		GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_DEVELOPER_ERROR,0.0f);
		return;
	}

	//after a long wait, control comes back here, and we examine our results
#ifdef _DEBUG
	LogMsg("IAP packet returned:");
	LogMsg(itemReceiptJSON);
#endif

	char *receiptStatus = NULL;
	cJSON *cJSONReceipt = cJSON_Parse(itemReceiptJSON);
	
	if (cJSONReceipt)
	{
		receiptStatus = cJSON_GetObjectItem(cJSONReceipt, "receiptStatus")->valuestring;
	}

	if (!receiptStatus || !cJSONReceipt)
	{
		//Service not available?
		GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_SERVICE_UNAVAILABLE,0.0f);
		//clean up if needed
		cJSON_Delete(cJSONReceipt);
		return;
	}
	
#ifdef _DEBUG
	LogMsg("Receipt status: %s", receiptStatus);
#endif
	//CStrChar receipt(receiptStatus);
	if (CaseInsensitiveCompare(receiptStatus, "Charged"))
	{
		int orderNum =0;
		cJSON *pOrderNum = cJSON_GetObjectItem(cJSONReceipt, "orderNo");
		if (pOrderNum) 
		{
			orderNum = pOrderNum->valueint;
		}

		string extra = toString(orderNum);
		GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_OK,0.0f,0, extra);
	} else
	{
		GetMessageManager()->SendGUI(MESSAGE_TYPE_IAP_RESULT,(float)IAPManager::RESULT_USER_CANCELED,0.0f);
	}

	//either way, clean up
	cJSON_Delete(cJSONReceipt);
}


// When running under windows, it is necessary to make additional OGL
// setup calls. We forward declare them here. Note that in the palm version
// this all gets #ifdef'd out.
// also, if we're not using OGL at all, this doesn't get compiled either. 
#if WIN32
// the forward declaration of the library loder. This is necessary to actually
// load up the OGL lib. You can copy-paste this directly into your code. 
// You'll never need to change it.
extern "C" 
{
	GL_API int GL_APIENTRY _dgles_load_library(void *, void *(*)(void *, const char *));
};

// this function is necessary for the call to _dgles_load_library. Internally,
// it is a callback that the OGL lib calls to find out where various functions are.
// You won't have to change this. Copy-paste is your friend.
static void *proc_loader(void *h, const char *name)
{
	(void) h; // make the compiler not complain about unused formal parameters
	return SDL_GL_GetProcAddress(name);
}
#endif // USE_OGL & WIN32



int initSDL_GLES()
{
	// used to get the result value of SDL operations
	int result;

	/*
	char buffer[1024];
	sprintf(buffer, "test:bar:bas");
	buffer[4] = 0;
	buffer[8] = 0;
	char *names[3];
	parseNullsepList(buffer, 1024, names, 3);
	*/

	PDL_ScreenTimeoutEnable(PDL_FALSE);

	// init SDL. This function is all it takes to init both
	// the audio and video.
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK );
	if ( result < 0 )
	{
		return result;
	}

	// On the actual device, you do not need to do anything special
	// to use OpenGL. but on windows, you need to load the library
	// in order to use it. So this code block is ifdefed only for windows.
	// if we're not using OGL, this is unnecessary
#if USE_OGL & WIN32
	{
		result = SDL_GL_LoadLibrary("opengl32.dll");
		if (result != 0)
		{
			return result;
		}
	}
#endif

	Uint32 videoFlags = SDL_OPENGL;
	// we need to set up OGL to be using the version of OGL we want. This
	// example uses OGL 1.0. 2.0 is a completely different animal. If you fail
	// to speficy what OGL you want to use, it will be pure luck which one you
	// get. Don't leave that up to chance. 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);	// Force this to be version 1


#ifdef RT_WEBOS_ARM
	

#ifdef FORCE_IPHONE_SIZE
	//if defined, this is useful for letting us make the touchpad handle the zooming for us
	g_winVideoScreenX = 320;
	g_winVideoScreenY = 480;

	LogMsg("Forcing GL to be %d, %d so the Touchpad will scale it up for us", g_winVideoScreenX, g_winVideoScreenY);
	
#endif
	
	g_screen = SDL_SetVideoMode(g_winVideoScreenX, g_winVideoScreenY, 0, videoFlags);
	g_winVideoScreenX = g_screen->w;
	g_winVideoScreenY = g_screen->h;

#else
	g_screen = SDL_SetVideoMode(GetPrimaryGLX() , GetPrimaryGLY(), 0, videoFlags);
#endif



	LogMsg("Setting up GLES to %d by %d.  PDK version is %d",g_screen->w, g_screen->h, PDL_GetPDKVersion());
	
	if ( g_screen == NULL )
	{
		// couldn't make that screen
		LogMsg("error making screen");
		return 1;
	}

	// time for some windows-specific initalization again. This time, it's
	// the call necessary to load up the OGL library. Again, this is windows
	// specific, and not needed on the palm side. You may wonder where _dgles_load_library
	// lives. It's in the OGL static lib associated with the dll. that's why we 
	// forward declared it at the top of this file, because none of our headers have
	// heard of it. 
	// And again, if we're not using OGL, this is unnecessary
#if WIN32
	_dgles_load_library(NULL, proc_loader);
#endif

	LogMsg((char*)glGetString(GL_VERSION));

	// now we init graphics
	//graphics_init();

	// This sets the "caption" for whatever the window is. On windows, it's the window
	// title. On the palm, this functionality does not exist and the function is ignored.
	SDL_WM_SetCaption(GetAppName(), NULL);

	// Like SDL, we will have the standard of always returning 0 for
	// success, and nonzero for failure. If we're here, it means success!
	return 0;
}

bool parseNullsepList(const char *nullsepList, int listBufferSize, char **arrayToFill, int numElements)
{
	// parse the string
	char *p = (char *)nullsepList;
	p[listBufferSize-1] = 0; // one way or another, this buffer ends in a 0
	int pos = 0; // used to track how far into the string we are
	for ( int i=0 ; i<numElements ; i++ )
	{
		// get the current element
		char *start = p;

		// advance until we hit a 0 (buffer is guaranteed to end in a 0)
		while (*p!=0) 
		{
			p++;
			pos++;

			if ( pos >= listBufferSize ) return false;
		}

		int len = (int)(p-start);

		// make the string
		arrayToFill[i] = new char[len+1];

		// snprintf(arrayToFill[i], len+1, "%s", p); // copy over the string
		sprintf(arrayToFill[i], "%s", start); // copy over the string

		// advance past the 0
		p++;
		pos++;
	}
	return true;
}
bool g_leftMouseButtonDown = false; //to help emulate how an iphone works


void SDLEventLoop()
{
	// we'll be using this for event polling
	SDL_Event ev;

	// this is how you poll for events. You can't just
	// ignore this function and go your own way. SDL does
	// critical tasks during SDL_PollEvent. You have to call
	// it, and call it frequently. If you have some very large
	// task to perform, like loading hundreds of images, you'll
	// have to break it up in to bite sized pieces. Don't 
	// starve SDL. 
	while (SDL_PollEvent(&ev)) 
	{
		switch (ev.type) 
		{
		case SDL_QUIT:
			{
				g_bAppFinished = true;
			}
			break; 

		case SDL_JOYAXISMOTION:
			{
				int accelerometer = ev.jaxis.axis;
				float val = float(ev.jaxis.value) / 32768.0f;
				//updateAccelerometer(accelerometer, val);
				
				CL_Vec3f v = CL_Vec3f(0,0,0);

				switch (ev.jaxis.axis)
				{
				case 0: //x
					g_accelHold.x = val;
					break;
				case 1: //y
					g_accelHold.y = val;
					break;
				case 2: //z
					g_accelHold.z = val;
					break;
				}
				GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_ACCELEROMETER, Variant(g_accelHold));

				//LogMsg("Got %s",PrintVector3(v).c_str());
			}
			break;
	
	case SDL_ACTIVEEVENT:
	
#ifndef RT_RUNS_IN_BACKGROUND
		if (ev.active.gain == 0)
		{
			g_isInForeground = false;
			LogMsg("In background");
			GetBaseApp()->OnEnterBackground();
		}

		if (ev.active.gain == 1)
		{
			g_isInForeground = true;
			GetBaseApp()->OnEnterForeground();
			LogMsg("In foreground");
		}
#endif

 	break;
		case SDL_MOUSEBUTTONDOWN:
			{
				//ev.motion.which should hold which finger id

			g_leftMouseButtonDown = true;
			float xPos = ev.motion.x;
			float yPos = ev.motion.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, ev.motion.which);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			{
				//ev.motion.which should hold which finger id
				g_leftMouseButtonDown = false;
				float xPos = ev.motion.x;
				float yPos = ev.motion.y;
				ConvertCoordinatesIfRequired(xPos, yPos);
				GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, ev.motion.which);
			}
			break;
	
		case SDL_MOUSEMOTION:     
			{
				float xPos = ev.motion.x;
				float yPos = ev.motion.y;
				
				
				//ev.motion.which should hold which finger id
				ConvertCoordinatesIfRequired(xPos, yPos);

				if (g_leftMouseButtonDown || GetPlatformID() == PLATFORM_ID_WEBOS)
				{
					GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, ev.motion.which);
					break;
				} 

#ifdef WIN32
				GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE_RAW, xPos, yPos, ev.motion.which);
#endif
			}

			break;

		case SDL_KEYDOWN:
		
			switch (ev.key.keysym.sym)
			{
		
				//touchpad doesn't seem to send these capslock events at all.  Uh.. why?

			case SDLK_CAPSLOCK:
				//LogMsg("Got Capslock: %d, mod of %d, scan of %d, unicode of %d", ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.scancode, ev.key.keysym.unicode);
				break;
			case SDLK_NUMLOCK:
				//LogMsg("Got numlock: %d, mod of %d, scan of %d, unicode of %d", ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.scancode, ev.key.keysym.unicode);
				break;

			case SDLK_ESCAPE:
				// Escape forces us to quit the app
				// this is also sent when the user makes a back gesture
				g_bAppFinished = true;
				break;

			case SDLK_WORLD_70:
				//LogMsg("Got old keyboard change button");
				break;
			
			case 24:
				//LogMsg("Got new keyboard change button.. should lose focus");
				SetIsUsingNativeUI(false);
				PDL_SetKeyboardState(PDL_FALSE);

				break;

			default:
				{

				
					if (ev.key.keysym.sym >= 32 && ev.key.keysym.sym <= 128 || ev.key.keysym.sym == 8 || ev.key.keysym.sym == 13)
					{
						signed char key = ev.key.keysym.sym;

						if (ev.key.keysym.mod & KMOD_LSHIFT || ev.key.keysym.mod & KMOD_RSHIFT || ev.key.keysym.mod & KMOD_CAPS)
						{
							key = toupper(key);
						}

						GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR, (float)key, 0.0f); 

					} else
					{
						//LogMsg("Got unknown key: %d", ev.key.keysym.sym);
					}
				}

				break;
			}
		break;
		}
	}
}

bool InitSDL()
{
	LogMsg("Initting SDL bits for GL setup and input");
	int result = initSDL_GLES(); 
	if ( result != 0 )
	{
		LogMsg("Error initting SDL: %s", SDL_GetError());
		return false;
	}

	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	return true;
}

#ifdef WIN32
//needed so we don't have to set the correct working directory during windows emulator builds
#include <direct.h>

string GetExePath()
{
	// Get path to executable:
	TCHAR szDllName[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFilename[256];
	TCHAR szExt[256];
	GetModuleFileName(0, szDllName, _MAX_PATH);
	_splitpath(szDllName, szDrive, szDir, szFilename, szExt);

	return string(szDrive) + string(szDir); 
}

#endif

int main(int argc, char *argv[])
{
#ifdef WIN32
	//first make sure our working directory is the .exe dir
	_chdir(GetExePath().c_str());
#endif

#ifdef RT_WEBOS_ARM
	openlog(GetBundleName(), LOG_PID, LOG_USER); 

	//move to the correct path, my Palm Pre 1.42 seems to need this, even after installing as a real package
	
	char temp[512];
	sprintf(temp, "/media/cryptofs/apps/usr/palm/applications/%s%s", GetBundlePrefix(), GetBundleName());
	chdir(temp);

#endif

	srand( (unsigned)time(NULL) );
	RemoveFile("log.txt", false);

#ifdef WIN32
	//init winsock so we can use the internet
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	InitVideoSize();

	if (!InitSDL())
	{
		LogMsg("Couldn't init SDL: %s", SDL_GetError());
#ifdef WIN32
		MessageBox(NULL, SDL_GetError(), "Unable to load stuff", NULL);
#endif
		goto cleanup;
	}

	SDL_JoystickEventState(SDL_IGNORE);
	
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	
	if (!GetBaseApp()->Init())
	{
#ifdef WIN32
		MessageBox(NULL, "Couldn't initialize game.  Yeah.\n\nDid everything unzip right?", "Unable to load stuff", NULL);
#endif
		goto cleanup;
	}


	static unsigned int gameTimer = 0;
	static unsigned int fpsTimerLoopMS = 0;
	GetBaseApp()->OnScreenSizeChange();

	while(1)
	{
		if (g_bAppFinished) break;
		SDLEventLoop();

		if (fpsTimerLoopMS != 0)
		{
			while (gameTimer > SDL_GetTicks())
			{
				SDL_Delay(1); 
			}
			gameTimer = SDL_GetTicks()+fpsTimerLoopMS;
		}

		if (g_isInForeground) 
		{
			GetBaseApp()->Update();
		}

		GetBaseApp()->Draw();

		while (!GetBaseApp()->GetOSMessages()->empty())
		{
			OSMessage m = GetBaseApp()->GetOSMessages()->front();
			GetBaseApp()->GetOSMessages()->pop_front();
			//LogMsg("Got OS message %d, %s", m.m_type, m.m_string.c_str());

			switch (m.m_type)
			{
			case OSMessage::MESSAGE_CHECK_CONNECTION:
				//pretend we did it
				GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);	
				break;
			case OSMessage::MESSAGE_OPEN_TEXT_BOX:
				
				if (PDL_GetPDKVersion() >= 300)
				{
					//for touchpad only.  Comment this out of you aren't using the 3.0 SDK but something older...
					PDL_SetKeyboardState(PDL_TRUE);
				}
				break;
			
			case OSMessage::MESSAGE_IAP_PURCHASE:

				IAPBuyItem(m.m_string);
				break;
			case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
				if (PDL_GetPDKVersion() >= 300)
				{
					//for touchpad only.  Comment this out of you aren't using the 3.0 SDK but something older...
					PDL_SetKeyboardState(PDL_FALSE);

				}
				SetIsUsingNativeUI(false);
				break;
				
			case OSMessage::MESSAGE_SET_FPS_LIMIT:
				fpsTimerLoopMS = (int) (1000.0f/m.m_x);
				break;

			case OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ:

				//well, it's hardcoded at 30 by the pdk I guess, but we can still enable/disable it
				if( SDL_NumJoysticks() > 0 )
				{

					if (m.m_x == 0)
					{
						//disable it if needed
						if (g_pSDLJoystick)
						{
							SDL_JoystickClose(g_pSDLJoystick);
							g_pSDLJoystick = NULL;
							SDL_JoystickEventState(SDL_IGNORE);
					}
					} else
					{
						if (!g_pSDLJoystick)
						{
							g_pSDLJoystick = SDL_JoystickOpen(0);
							SDL_JoystickEventState(SDL_ENABLE);

						}
					}
				}

				break;


			}
		}

		// You should always have a little delay (recommend 10ms)
		// at the end of your event loop. This keeps SDL from
		// hogging all the CPU time. 
		glFlush();
		glFinish();
		SDL_GL_SwapBuffers();
		SDL_Delay(10); 
	}
	GetBaseApp()->Kill();

cleanup:
	if (g_pSDLJoystick)
	{
		SDL_JoystickClose(g_pSDLJoystick);
		g_pSDLJoystick = NULL;
	}

	SDL_Quit();
	

#ifdef RT_WEBOS_ARM
	closelog(); 
#endif

	#ifdef WIN32
	//done with winsock
	WSACleanup(); 
	#endif

	return 0;
}
#endif