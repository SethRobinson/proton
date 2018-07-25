#include "PlatformSetup.h"
#include "BaseApp.h"

#include <SDL2/SDL.h>

const char * GetAppName();

using namespace std;

bool g_bAppFinished = false;
bool g_bIsFullScreen = true;
SDL_Surface *g_screen = NULL;
SDL_Joystick *g_pSDLJoystick = NULL;
CL_Vec3f g_accelHold = CL_Vec3f(0,0,0);
bool g_leftMouseButtonDown = false; //to help emulate how an iphone works
bool g_rightMouseButtonDown = false; //to help emulate how an iphone works

bool g_isInForeground = true;

int g_winVideoScreenX = 0;
int g_winVideoScreenY = 0;
SDL_GLContext g_glcontext = NULL;
SDL_Window *g_window = NULL;

#ifndef C_GL_MODE
#include "EGL/egl.h"
#include "bcm_host.h"
EGLDisplay			g_eglDisplay	= 0;
EGLSurface			g_eglSurface	= 0;
EGLConfig eglConfig = 0;
EGLContext			g_eglContext	= 0;
static EGL_DISPMANX_WINDOW_T native_window; //probably specific to raspi
#endif


#ifdef WINAPI

void AddText(const char *tex ,const char *filename)
{
	FILE *          fp;
	if (strlen(tex) < 1) return;

	if (FileExists(filename) == false)
	{

		fp = fopen( filename, "wb");
		if (!fp) return;
		fwrite( tex, strlen(tex), 1, fp);      
		fclose(fp);
		return;
	} else
	{
		fp = fopen(filename, "ab");
		if (!fp) return;
		fwrite( tex, strlen(tex), 1, fp);      
		fclose(fp);
	}
}


void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 1024*10;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf_s( buffer, logSize, logSize, traceStr, argsVA );
	va_end( argsVA );


	OutputDebugString(buffer);
	OutputDebugString("\n");

	if (IsBaseAppInitted())
	{
		GetBaseApp()->GetConsole()->AddLine(buffer);
		strcat(buffer, "\r\n");
		//OutputDebugString( (string("writing to ")+GetSavePath()+"log.txt\n").c_str());
		AddText(buffer, (GetSavePath()+"log.txt").c_str());
	}

}
#endif

int GetPrimaryGLX() 
{
	return g_winVideoScreenX;
}

int GetPrimaryGLY() 
{
	return g_winVideoScreenY;
}

void SetPrimaryScreenSize(int width, int height)
{
	g_winVideoScreenX = width;
	g_winVideoScreenY = height;
}

bool initSDL_GLES()
{
	// used to get the result value of SDL operations
	
	uint32 initFlags = SDL_INIT_TIMER| SDL_INIT_JOYSTICK | SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER|SDL_INIT_VIDEO;

#ifdef RT_USE_SDL_AUDIO
	initFlags = initFlags|SDL_INIT_AUDIO;
#endif

	SDL_Init(initFlags);              // Initialize SDL2

	
	uint32 videoFlags = SDL_WINDOW_OPENGL;


	//for fulscreen:

	//videoFlags = videoFlags|SDL_WINDOW_FULLSCREEN_DESKTOP;
	
	
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0) 
	{
		SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());

	} else
	{
		LogMsg("Real screen size: %d, %d", dm.w, dm.h);
	}

	//SetPrimaryScreenSize(GetScreenSizeX() , GetScreenSizeY());

	//if (GetPrimaryGLX() == GetScreenSizeX() && GetPrimaryGLY() == GetScreenSizeY())
	{
		//let's assume they want a real fullscreen
#ifdef _DEBUG

		//videoFlags = videoFlags|SDL_WINDOW_FULLSCREEN_DESKTOP; //handles debugging better

#else
		//videoFlags = videoFlags|SDL_WINDOW_SHOWN;

#endif
	}

	LogMsg("Primary: %d, %d, opening window sized %d, %d", GetPrimaryGLX(), GetPrimaryGLY(), GetScreenSizeX(), GetScreenSizeY());


#ifndef C_GL_MODE
	LogMsg("Attempting to get a GLES1.1 context");
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1); 

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); 

	//actually, we don't need a context.  We'll do it raw our own way
	return true;
#else

	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

#endif

	
	// Create an application window with the following settings:
	g_window = SDL_CreateWindow(
		GetAppName(),                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		GetScreenSizeX(),                               // width, in pixels
		GetScreenSizeY(),                               // height, in pixels
		videoFlags                
		);


	// Check that the window was successfully created
	if (g_window == NULL) 
	{
		// In the case that the window could not be made...
		LogError("Could not create window: %s\n", SDL_GetError());
		return false;
	}
	LogMsg("SDL Window created");


	//Note: For both GLES and GL we allow SDL to create the window.  But on Raspberry pi, we have to use EGL to create our own context or we
	//always get a GLES 2.0 instead of GLES 1.1 (SDL bug I think as it works right on other platforms)

	return true;
}


bool InitGLESContextWithSDL()
{

	g_glcontext = SDL_GL_CreateContext(g_window);

	if (g_glcontext == NULL) 
	{
		// In the case that the window could not be made...
		LogError("Could not create GL context: %s\n", SDL_GetError());
		return false;
	}

	LogMsg("SDL context created");

	//don't need a renderer

	/*

	SDL_Renderer* renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer)
	{
		LogMsg("Unable to create SDL renderer");
		return false;
	}

	LogMsg("SDL renderer created");
	*/

	return true;
}

#ifndef C_GL_MODE


bool InitGLESContextWithEGL()
{
	g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if (g_eglDisplay == EGL_NO_DISPLAY)
	{
		LogError("Can't get EGL display");
		return false;
	}

	LogMsg("Initted EGL display");

	int rc = eglInitialize(g_eglDisplay, NULL, NULL);
	if (rc != EGL_TRUE) 
	{
		LogError("eglInitialize failed");
		return false;
	}

	rc = eglBindAPI(EGL_OPENGL_ES_API);
	if (rc != EGL_TRUE) 
	{
		LogError("eglBindAPI failed");
		return false;
	}

	
	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(g_eglDisplay, &iMajorVersion, &iMinorVersion))
	{

		LogError("eglinitialize failed");
		return false;
	}

	
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;


	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = GetPrimaryGLX();
	dst_rect.height = GetPrimaryGLY();

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = dst_rect.width<< 16;
	src_rect.height = dst_rect.height<< 16;    

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );

	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
		0/*layer*/, &dst_rect, 0/*src*/,
		&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

	native_window.element = dispman_element;
	native_window.width = dst_rect.width;
	native_window.height = dst_rect.height;

	
	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLint num_config;

	
	rc = eglChooseConfig(g_eglDisplay, attribute_list, &eglConfig, 1, &num_config);

	if (rc == EGL_FALSE)
	{
		LogError("eglChooseConfig failed");
		return false;
	}

	g_eglContext = eglCreateContext(g_eglDisplay, eglConfig, NULL, NULL);
	if (g_eglContext == EGL_NO_CONTEXT)
	{
		LogError("Error creating egl context");
		return false;
	}


	vc_dispmanx_update_submit_sync( dispman_update );

	g_eglSurface = eglCreateWindowSurface(g_eglDisplay, eglConfig, &native_window, NULL);

	if(g_eglSurface == EGL_NO_SURFACE)
	{
		eglGetError(); // Clear error
		LogMsg("Eror creating egl window surface");
		return false;
	}

	


	LogMsg("EGL context created");
	
	if (eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext) != EGL_TRUE)
	{
		LogError("eglMakeCurrent");
		return false;
	}

	LogMsg("Initting EGL at %d, %d",GetPrimaryGLX(), GetPrimaryGLY());

	return true;
}
#endif

bool InitSDL()
{
	LogMsg("initting SDL");
	if (!initSDL_GLES())
	{
		LogMsg("Error initting SDL: %s", SDL_GetError());
		return false;
	}

	
#ifdef C_GL_MODE
	InitGLESContextWithSDL(); //doesn't work to get v1.1 gles context, but fine for opengl
#else
	InitGLESContextWithEGL(); //gles v1.1 mode for raspberry pi
#endif

	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	return true;
}

void ChangeEmulationOrientationIfPossible(int desiredX, int desiredY, eOrientationMode desiredOrienation)
{
	if (GetForcedOrientation() != ORIENTATION_DONT_CARE)
	{
		LogMsg("Can't change orientation because SetForcedOrientation() is set.");
	} else
	{
		SetupScreenInfo(desiredX, desiredY, desiredOrienation);
	}
}

int ConvertSDLKeycodeToProtonVirtualKey(SDL_Keycode sdlkey)
{
	int keycode = sdlkey;
	
	switch (sdlkey)
	{
	case SDLK_LEFT: keycode = VIRTUAL_KEY_DIR_LEFT; break;
	case SDLK_RIGHT: keycode = VIRTUAL_KEY_DIR_RIGHT; break;
	case SDLK_UP: keycode = VIRTUAL_KEY_DIR_UP; break;
	case SDLK_DOWN: keycode = VIRTUAL_KEY_DIR_DOWN; break;
	
	case SDLK_RSHIFT:
	case SDLK_LSHIFT: keycode = VIRTUAL_KEY_SHIFT; break;
	case SDLK_RCTRL:
	case SDLK_LCTRL: keycode = VIRTUAL_KEY_CONTROL; break;
	
	case SDLK_ESCAPE: keycode = VIRTUAL_KEY_BACK; break;

	default:
		if (sdlkey >= SDLK_F1 && sdlkey <= SDLK_F15)
		{
				keycode = VIRTUAL_KEY_F1 + (sdlkey - SDLK_F1);
		}
	}

	return keycode;
}

uint32 GetLinuxKeyModifiers()
{
	uint32 mods=0;

	uint32 sdlMods = SDL_GetModState();

	if (sdlMods &KMOD_SHIFT)
	{
		mods = mods|VIRTUAL_KEY_MODIFIER_SHIFT;
	}


	if (sdlMods &KMOD_CTRL)
	{
		mods = mods|VIRTUAL_KEY_MODIFIER_CONTROL;
	}

	if (sdlMods &KMOD_ALT)
	{
		mods = mods|VIRTUAL_KEY_MODIFIER_ALT;
	}

	return mods;
}


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


		case SDL_WINDOWEVENT:
			{

			
			switch (ev.window.event) 
			{
		case SDL_WINDOWEVENT_SHOWN:
			SDL_Log("Window %d shown", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			g_isInForeground = false;
			LogMsg("In background");
			GetBaseApp()->OnEnterBackground();
			SDL_Log("Window %d hidden", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			g_isInForeground = true;
			GetBaseApp()->OnEnterForeground();
			LogMsg("In foreground");
			SDL_Log("Window %d exposed", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_MOVED:
			SDL_Log("Window %d moved to %d,%d",
				ev.window.windowID, ev.window.data1,
				ev.window.data2);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			SDL_Log("Window %d resized to %dx%d",
				ev.window.windowID, ev.window.data1,
				ev.window.data2);
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SDL_Log("Window %d size changed to %dx%d",
				ev.window.windowID, ev.window.data1,
				ev.window.data2);
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			g_isInForeground = false;
			LogMsg("In background");
			GetBaseApp()->OnEnterBackground();
			SDL_Log("Window %d minimized", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			SDL_Log("Window %d maximized", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_RESTORED:
			g_isInForeground = true;
			GetBaseApp()->OnEnterForeground();
			LogMsg("In foreground");
			SDL_Log("Window %d restored", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_ENTER:
			SDL_Log("Mouse entered window %d",
				ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_LEAVE:
			SDL_Log("Mouse left window %d", ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			SDL_Log("Window %d gained keyboard focus",
				ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			SDL_Log("Window %d lost keyboard focus",
				ev.window.windowID);
			break;
		case SDL_WINDOWEVENT_CLOSE:
			SDL_Log("Window %d closed", ev.window.windowID);
			g_bAppFinished = true;
			break;
		default:
			SDL_Log("Window %d got unknown event %d",
				ev.window.windowID, ev.window.event);
			break;
			}
			break;

			}
		
		case SDL_MOUSEBUTTONDOWN:
		{
			int buttonToToSendToProton = ev.button.button;

			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				buttonToToSendToProton = 0; //other platforms always have 0 as the left mouse button
				g_leftMouseButtonDown = true;
			}
			if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				buttonToToSendToProton = 1; //other platforms always have 1 as the right mouse button
				g_rightMouseButtonDown = true;
			}

			if (ev.button.button == SDL_BUTTON_MIDDLE)
			{
				buttonToToSendToProton = 2;
			}


			float xPos = (float)ev.button.x;
			float yPos = (float)ev.button.y;
			
#ifdef _DEBUG
LogMsg("Tapped %.2f, %.2f", xPos, yPos);
#endif
			ConvertCoordinatesIfRequired(xPos, yPos);
			//gPointerEventHandler->handlePointerDownEvent(xPos, yPos, ev.button.button);
			
			GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_START, (float)xPos, (float)yPos, buttonToToSendToProton, GetLinuxKeyModifiers());

			break;
		}
		
		case SDL_MOUSEBUTTONUP:
		{
			int buttonToToSendToProton = ev.button.button;

			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				buttonToToSendToProton = 0; //other platforms always have 0 as the left mouse button
				g_leftMouseButtonDown = false;
			}
			if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				buttonToToSendToProton = 1; //other platforms always have 1 as the right mouse button
				g_rightMouseButtonDown = false;
			}

			if (ev.button.button == SDL_BUTTON_MIDDLE)
			{
				buttonToToSendToProton = 2;
			}


			float xPos = (float)ev.button.x;
			float yPos = (float)ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			//gPointerEventHandler->handlePointerUpEvent(xPos, yPos, ev.button.button);
			GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_END, (float)xPos, (float)yPos, buttonToToSendToProton, GetLinuxKeyModifiers());
			break;
		}
	
		case SDL_MOUSEMOTION:
		{
			float xPos = (float)ev.button.x;
			float yPos = (float)ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);

			if (g_leftMouseButtonDown)
			{
				GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, 0, GetLinuxKeyModifiers());
			} 

			if (g_rightMouseButtonDown)
			{
				GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, 1, GetLinuxKeyModifiers());
			} 

			GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_MOVE_RAW, xPos, yPos, 0, GetLinuxKeyModifiers());
	
			break;
		}

		case SDL_KEYDOWN:
		{
			if (ev.key.keysym.mod & KMOD_CTRL)
			{
				switch (ev.key.keysym.sym)
				{
				case SDLK_q:
					LogMsg("Ctrl-Q detected, quitting app");
					g_bAppFinished = true;
					break;
					case SDLK_l: // Left landscape mode
						ChangeEmulationOrientationIfPossible(GetPrimaryGLY(), GetPrimaryGLX(), ORIENTATION_LANDSCAPE_LEFT);
						break;

					case SDLK_r: // Right landscape mode
						ChangeEmulationOrientationIfPossible(GetPrimaryGLY(), GetPrimaryGLX(), ORIENTATION_LANDSCAPE_RIGHT);
						break;

					case SDLK_p: // Portrait mode
						ChangeEmulationOrientationIfPossible(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
						break;

					case SDLK_u: // Upside down portrait mode
						ChangeEmulationOrientationIfPossible(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT_UPSIDE_DOWN);
						break;
				}
			}

			int vKey = ConvertSDLKeycodeToProtonVirtualKey(ev.key.keysym.sym);
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)vKey, (float)VIRTUAL_KEY_PRESS);

			if (vKey >= SDLK_SPACE && vKey <= SDLK_DELETE || vKey == SDLK_BACKSPACE || vKey == SDLK_RETURN)
			{
				signed char key = vKey;

				if (ev.key.keysym.mod & KMOD_SHIFT || ev.key.keysym.mod & KMOD_CAPS)
				{
					key = toupper(key);
				}

				GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR, (float)key, (float)VIRTUAL_KEY_PRESS);
			}
		}
		break;
			
		case SDL_KEYUP:
		{
			int vKey = ConvertSDLKeycodeToProtonVirtualKey(ev.key.keysym.sym);
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)vKey, (float)VIRTUAL_KEY_RELEASE);
		}
		break;
		}
	}
}

#ifdef PLATFORM_LINUX

//linux doesn't have a SDL2_main lib I guess?
int main(int argc, char *argv[])

#else
int SDL_main(int argc, char *argv[])
#endif
{

	uint32 width = 1920;
	uint32 height = 1080;


	

	for (int l = 0; argv[l]; l++)
	{
		
		vector<string> parms = StringTokenize(argv[l], " ");

		for (unsigned int i = 0; i < parms.size(); i++)
		{
			GetBaseApp()->AddCommandLineParm(parms[i]);
		}
	}


#ifndef C_GL_MODE
 bcm_host_init();
 graphics_get_display_size(0 /* LCD */, &width, &height);
 LogMsg("Pulled %d, %d as screen size.", width, height);
#else

#endif

		
	    SetPrimaryScreenSize(width, height);

		SetEmulatedPlatformID(PLATFORM_ID_LINUX);
		SetForcedOrientation(ORIENTATION_DONT_CARE);

		SetupScreenInfo(g_winVideoScreenX,g_winVideoScreenY, ORIENTATION_DONT_CARE);
	
		GetBaseApp()->OnPreInitVideo(); //gives the app level code a chance to override any of these parms if it wants to

	if (!InitSDL())
	{
		LogMsg("Couldn't init SDL: %s", SDL_GetError());
		goto cleanup;
	}

	SDL_JoystickEventState(SDL_IGNORE);
	
	//SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	
	if (!GetBaseApp()->Init())
	{
		LogError("Couldn't initialize game. Yeah.\n\nDid everything unzip right?");
		goto cleanup;
	}

	GetBaseApp()->OnScreenSizeChange();

	static Uint32 nextFrameTick = 0;
	static Uint32 frameDelayMS = 0;

	while(1)
	{
		if (g_bAppFinished)
		{
			break;
		}
#ifndef C_GL_MODE
		if (eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext) != EGL_TRUE)
		{
			LogError("eglMakeCurrent");
			
		}
#endif
		SDLEventLoop();

		if (g_isInForeground) 
		{
			GetBaseApp()->Update();
		}

		GetBaseApp()->Draw();
 
		while (!GetBaseApp()->GetOSMessages()->empty())
		{
			OSMessage m = GetBaseApp()->GetOSMessages()->front();
			GetBaseApp()->GetOSMessages()->pop_front();

			switch (m.m_type)
			{

			case OSMessage::MESSAGE_FINISH_APP:
			case OSMessage::MESSAGE_SUSPEND_TO_HOME_SCREEN:

				g_bAppFinished = true;
				break;

				case OSMessage::MESSAGE_SET_VIDEO_MODE:
					LogMsg("Video mode set, ignoring");
					break;
				case OSMessage::MESSAGE_CHECK_CONNECTION:
					//pretend we did it
					GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);	
					break;
			
				case OSMessage::MESSAGE_OPEN_TEXT_BOX:
					break;
				
				case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
					break;
					
				case OSMessage::MESSAGE_SET_FPS_LIMIT:
					if (m.m_x == 0.0f)
					{
						frameDelayMS = 0;
					} else
					{
						frameDelayMS = (int)(1000.0f / m.m_x);
					}
					break;

				case OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ:
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

#ifndef C_GL_MODE
		//LogMsg("Tick %d", Random(20000));
		glFlush();
		eglSwapBuffers(g_eglDisplay, g_eglSurface);

#else
		
		SDL_GL_SwapWindow(g_window);
#endif

		// Control FPS and give some time for other processes too
		Uint32 sleepTime = 1;

		if (frameDelayMS != 0)
		{
			Uint32 ticksNow = SDL_GetTicks();

			if (nextFrameTick != 0 && nextFrameTick > ticksNow)
			{
				sleepTime = nextFrameTick - ticksNow;
			}
		}

		SDL_Delay(sleepTime);
		nextFrameTick = SDL_GetTicks() + frameDelayMS;
	}
	
	GetBaseApp()->Kill();

cleanup:
	if (g_pSDLJoystick)
	{
		SDL_JoystickClose(g_pSDLJoystick);
		g_pSDLJoystick = NULL;
	}
	
	SDL_Quit();

	LogMsg("Cleanly quitting");
#ifndef C_GL_MODE
	eglMakeCurrent( g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface( g_eglDisplay, g_eglSurface );
	eglDestroyContext( g_eglDisplay, g_eglContext );
	eglTerminate( g_eglDisplay );
#endif
	return 0;
}


void ForceVideoUpdate()
{
	g_globalBatcher.Flush();

	SDL_GL_SwapWindow(g_window);
	LogMsg("Did SDL_GL_SwapWindow");
}

