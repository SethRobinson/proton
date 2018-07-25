#include "BaseApp.h"
#include "HTML5Utils.h"
#include <SDL.h>
//#include <GL/glfw.h>
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>

using namespace std;

bool g_bAppFinished = false;

SDL_Surface *g_screen = NULL;
SDL_Joystick *g_pSDLJoystick = NULL;
CL_Vec3f g_accelHold = CL_Vec3f(0,0,0);
bool g_bRanInit = false;
bool g_isInForeground = true;

int g_winVideoScreenX = 0;
int g_winVideoScreenY = 0;
Uint32 g_nextFrameTick = 0;
Uint32 g_frameDelayMS = 0;

int g_touchesReceived = 0;

int GetTouchesReceived() {return g_touchesReceived;}

int GetPrimaryGLX() 
{
	return g_winVideoScreenX;
}

int GetPrimaryGLY() 
{
	return g_winVideoScreenY;
}

class TouchInfo
{
public:

	TouchInfo()
	{
		m_nativeTouchID = 0;
		m_bUsed = false;
	}

	int m_nativeTouchID;
	bool m_bUsed;
};

//a little something to map random #s into #s between 0 and 12, which is how proton wants to track touches
class TouchManager
{
public:
	int OnDown(int nativeTouchID);
	int OnUp(int nativeTouchID);
	int OnMove(int nativeTouchID);
	int TouchesActive();

private:
	int FindExistingNativeTouch(int nativeTouchID);
	int AddNewTouch(int nativeTouchID);

	void Clear()
	{
		for (int i=0; i < C_MAX_TOUCHES_AT_ONCE; i++)
		{
			m_touch[i].m_bUsed = false;
			m_touch[i].m_nativeTouchID = 0;
		}
	}

	TouchInfo m_touch[C_MAX_TOUCHES_AT_ONCE];
};

int TouchManager::TouchesActive()
{

	int active = 0;
	for (int i=0; i < C_MAX_TOUCHES_AT_ONCE; i++)
	{
		if (m_touch[i].m_bUsed)
		{
			active++;	
		}
	}
	return active;
}

int TouchManager::FindExistingNativeTouch(int nativeTouchID)
{
	for (int i=0; i < C_MAX_TOUCHES_AT_ONCE; i++)
	{
		if (m_touch[i].m_bUsed && m_touch[i].m_nativeTouchID == nativeTouchID) return i;
	}
	return -1; //touch doesn't exist
}

int TouchManager::AddNewTouch(int nativeTouchID)
{
	for (int i=0; i < C_MAX_TOUCHES_AT_ONCE; i++)
	{

		if (!m_touch[i].m_bUsed)
		{
			g_touchesReceived++;
			//this will work
			m_touch[i].m_bUsed = true;
			m_touch[i].m_nativeTouchID = nativeTouchID;
			return i;
		}
	}

	LogMsg("Touch of HTML touchIDs.. we must not be getting the touch ending events somewhere.  Clearing all of them...");
	Clear();
	return AddNewTouch(nativeTouchID);
}

int TouchManager::OnDown(int nativeTouchID)
{
	int protonTouch = FindExistingNativeTouch(nativeTouchID);
	if (protonTouch != -1) 
	{
		LogMsg("Why does touch %d already exist?", nativeTouchID);
		return protonTouch;
	}
	//make new touch
	return AddNewTouch(nativeTouchID);
}

int TouchManager::OnUp(int nativeTouchID)
{
	int protonTouch = FindExistingNativeTouch(nativeTouchID);
	if (protonTouch != -1) 
	{
		m_touch[protonTouch].m_bUsed = false;
		return protonTouch;
	}

	LogMsg("Couldn't remove touch %d", nativeTouchID);
	return 0; //couldn't find it
}

int TouchManager::OnMove(int nativeTouchID)
{
	int protonTouch = FindExistingNativeTouch(nativeTouchID);
	if (protonTouch != -1) 
	{
		return protonTouch; //found it
	}

	LogMsg("Can't find touch %d to move.. faking it", nativeTouchID);
	return 0; //couldn't find it
}


TouchManager g_touchManager;


EM_BOOL on_canvassize_changed(int eventType, const void *reserved, void *userData)
{
	//old way
	/*
	int w, h, fs;
	emscripten_get_canvas_size(&w, &h, &fs);
	*/
	
	//new way
	int w, h, fs;
	EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", &w, &h); 
	if (r != EMSCRIPTEN_RESULT_SUCCESS) 
	{
		LogMsg("Error getting canvas size of #canvas");
		return 0;
		/* handle error */ 
	}

	/*
	//iOS won't allow this I think
	EmscriptenFullscreenChangeEvent e; 
	r = emscripten_get_fullscreen_status(&e); 
	if (r != EMSCRIPTEN_RESULT_SUCCESS)
	{
		LogMsg("Error with emscripten_get_fullscreen_status");

	}
	fs = e.isFullscreen; 
	*/

	
	double cssW, cssH;
	r = emscripten_get_element_css_size("#canvas", &cssW, &cssH);
	LogMsg("Resized: RTT: %dx%d, CSS : %02gx%02g\n", w, h, cssW, cssH);

	if (r != EMSCRIPTEN_RESULT_SUCCESS)
	{
		LogMsg("Error emscripten_get_element_css_size #canvas");
		/* handle error */
	}

	/*
	r = emscripten_set_canvas_element_size("#canvas",
		int(cssW), int(cssH));
		*/
	
	LogMsg("on_canvassize_changed Changing size to %d, %d", g_winVideoScreenX, g_winVideoScreenY);
	
	//Um, g_winVideoScreenX aren't actually being set here, isn't that bug? Looks like it's set elsewhere though
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	return 0;
}

void requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
	#ifndef RT_HTML5_USE_CUSTOM_MAIN
	EmscriptenFullscreenStrategy s;
	memset(&s, 0, sizeof(s));
	s.scaleMode = scaleMode;
	s.canvasResolutionScaleMode = canvasResolutionScaleMode;
	s.filteringMode = filteringMode;
	s.canvasResizedCallback = on_canvassize_changed;
	int ret = emscripten_request_fullscreen_strategy(0, 1, &s);
	#endif

}

void enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
	LogMsg("*** Entering enterSoftFullscreen");
	EmscriptenFullscreenStrategy s;
	memset(&s, 0, sizeof(s));
	s.scaleMode = scaleMode;
	s.canvasResolutionScaleMode = canvasResolutionScaleMode;
	s.filteringMode = filteringMode;
	
	#ifndef RT_HTML5_USE_CUSTOM_MAIN
	s.canvasResizedCallback = on_canvassize_changed;
	#endif
	int ret = emscripten_enter_soft_fullscreen("#canvas", &s);
}

bool InitSDLScreen()
{
	//Uint32 videoFlags = SDL_HWSURFACE;
	//SDL_OPENGL;
	//Uint32 videoFlags = SDL_SWSURFACE;
	Uint32 videoFlags = SDL_OPENGL | SDL_RESIZABLE;

	g_screen = SDL_SetVideoMode(GetPrimaryGLX() , GetPrimaryGLY(), 32, videoFlags);

	if (g_screen == NULL)
	{
		// couldn't make that screen
		LogMsg("error making screen");
		return false;
	}

	LogMsg("Setting up GLES to %d by %d.", g_screen->w, g_screen->h);
	return true;
}

int initSDL_GLES()
{
	// used to get the result value of SDL operations
	int result;

	// init SDL. This function is all it takes to init both
	// the audio and video.
	LogMsg("Initting SDL bits for emscripten GL/input to work");
	
	//requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT);
    
	
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);

	atexit(SDL_Quit);

	if (result != 0)
	{
		LogMsg("Error initting SDL");
		return result;
	}

#if SDL_VERSION_ATLEAST(1, 3, 0)
	// we need to set up OGL to be using the version of OGL we want. This
	// example uses OGL 1.0. 2.0 is a completely different animal. If you fail
	// to speficy what OGL you want to use, it will be pure luck which one you
	// get. Don't leave that up to chance. 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);	// Force this to be version 1
#endif

	InitSDLScreen();
		
	// This sets the "caption" for whatever the window is. On windows, it's the window
	// title. On the palm, this functionality does not exist and the function is ignored.
	SDL_WM_SetCaption(GetAppName(), NULL);

	// Like SDL, we will have the standard of always returning 0 for
	// success, and nonzero for failure. If we're here, it means success!
	return 0;
}

bool InitSDL()
{
	LogMsg("Initting Proton SDK by Robinson Technologies ( protonsdk.com )");

	int result = initSDL_GLES(); 
	if (result != 0)
	{
		LogMsg("Error initting SDL: %s", SDL_GetError());
		return false;
	}
	
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	return true;
}

void ChangeEmulationOrientationIfPossible(int desiredX, int desiredY, eOrientationMode desiredOrienation)
{
	if (GetForcedOrientation() != ORIENTATION_DONT_CARE) {
		LogMsg("Can't change orientation because SetForcedOrientation() is set.");
	} else {
		SetupScreenInfo(desiredX, desiredY, desiredOrienation);
	}
}

int ConvertSDLKeycodeToProtonVirtualKey(SDLKey sdlkey)
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
			} else if (ev.active.gain == 1)
			{
				g_isInForeground = true;
				GetBaseApp()->OnEnterForeground();
				LogMsg("In foreground");
			}
#endif
			break;
		
/*
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			{
				if ((int)ev.tfinger.touchId == -1)
				{
					//not a real touch.  Ignoring
					LogMsg("Not a real touch (%d), ignoring", (int)ev.tfinger.touchId);
					continue;

				}
				float xPos = (float)GetPrimaryGLX()*ev.tfinger.x;
				float yPos = (float)GetPrimaryGLY()*ev.tfinger.y;
			    
				int touchID = (int)ev.tfinger.fingerId;

				ConvertCoordinatesIfRequired(xPos, yPos);

				if (ev.type == SDL_FINGERDOWN)
				{
					touchID = g_touchManager.OnDown(touchID);

					LogMsg("The Finger %d (real touchID: %d, Proton touch %d) down at %.2f, %.2f", (int)ev.tfinger.fingerId, (int)ev.tfinger.touchId, touchID, xPos, yPos);
					GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, touchID);
				} else if (ev.type == SDL_FINGERUP)
				{
					touchID = g_touchManager.OnUp(touchID);

					LogMsg("The Finger %d (real touchID: %d, Proton touch %d) up at %.2f, %.2f", (int)ev.tfinger.fingerId, (int)ev.tfinger.touchId, touchID, xPos, yPos);
					GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, touchID);
				} else if (ev.type == SDL_FINGERMOTION)
				{
					touchID = g_touchManager.OnMove(touchID);

					LogMsg("The Finger %d (real touchID: %d, Proton touch %d) move at %.2f, %.2f", (int)ev.tfinger.fingerId, (int)ev.tfinger.touchId, touchID, xPos, yPos);
					GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, touchID);
				}
				break;
			}


		case SDL_MOUSEBUTTONDOWN:
		{
		
			LogMsg("Mouse down %d - : "+(int)ev.button.which);
			if (ev.button.which == SDL_TOUCH_MOUSEID) continue; //don't care, this will get handled by the touch handler
			LogMsg("Mouse down handled.");
			float xPos = ev.button.x;
			float yPos = ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, ev.button.button);
			break;
		}
		
		case SDL_MOUSEBUTTONUP:
		{
			if (ev.button.which == SDL_TOUCH_MOUSEID) continue; //don't care, this will get handled by the touch handler
			LogMsg("Mouse up handled.");
			float xPos = ev.button.x;
			float yPos = ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, ev.button.button);
			break;
		}
	
		case SDL_MOUSEMOTION:
		{
			if (ev.motion.which == SDL_TOUCH_MOUSEID) continue; //don't care, this will get handled by the touch handler
		
			LogMsg("Mouse move handled. (which is %d)", (int)ev.motion.which);

			float xPos = ev.motion.x;
			float yPos = ev.motion.y;
			ConvertCoordinatesIfRequired(xPos, yPos);

			// Always pass the same pointer id here since it's not possible to track multiple pointing devices with SDL_MOUSEMOTION
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, SDL_BUTTON_LEFT);
			break;
		}
*/

		case SDL_KEYDOWN:
		{
			if (ev.key.keysym.mod & KMOD_CTRL)
			{
				/*
				switch (ev.key.keysym.sym)
				{
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
				*/

			}

			switch (ev.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				// Escape forces us to quit the app
				// this is also sent when the user makes a back gesture
				g_bAppFinished = true;
				break;
			}

			int vKey = ConvertSDLKeycodeToProtonVirtualKey(ev.key.keysym.sym);
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)vKey, (float)VIRTUAL_KEY_PRESS);

			if ( (vKey >= SDLK_SPACE && vKey <= SDLK_DELETE) || vKey == SDLK_BACKSPACE || vKey == SDLK_RETURN)
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

void Sleep(long ms)
{
	#ifdef RT_EMTERPRETER_ENABLED
	emscripten_sleep(ms);
	#endif
}

void MainEventLoop()
{

	static int oldWidth = GetScreenSizeX();
	static int oldHeight = GetScreenSizeY();
	
	int width, height, isfs;
	
	
	EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", &width, &height); 
	if (r != EMSCRIPTEN_RESULT_SUCCESS) 
	{
		LogMsg("Error getting canvas size of #canvas");
	}

	if (oldWidth != width || oldHeight != height)
	{
		LogMsg("MainEventLoop: Canvas: %d, %d, %d", width, height, isfs);
		oldWidth = width;
		oldHeight = height;

		g_winVideoScreenX = width;
		g_winVideoScreenY = height;
	}

	if (g_bAppFinished)
	{
		//break; //uhhh...
	}

	SDLEventLoop();

	if (!g_bRanInit && IsStillLoadingPersistentData())
	{
		LogMsg("waiting...");
	
	}
	else
	{

		if (!g_bRanInit)
		{
			LogMsg("Initting BaseApp");
			if (!GetBaseApp()->Init())
			{
				LogError("Couldn't initialize game. Yeah.\n\nDid everything unzip right?");
				
			}
			g_bRanInit = true;
		}


		GetBaseApp()->Update();
		GetBaseApp()->Draw();
	}

	while (!GetBaseApp()->GetOSMessages()->empty())
	{
		OSMessage m = GetBaseApp()->GetOSMessages()->front();
		GetBaseApp()->GetOSMessages()->pop_front();

		switch (m.m_type)
		{
		case OSMessage::MESSAGE_CHECK_CONNECTION:
			//pretend we did it
			GetMessageManager()->SendGUI(MESSAGE_TYPE_OS_CONNECTION_CHECKED, (float)RT_kCFStreamEventOpenCompleted, 0.0f);	
			break;
			 
		case OSMessage::MESSAGE_OPEN_TEXT_BOX:
			break;

		case OSMessage::MESSAGE_CLOSE_TEXT_BOX:
			break;

		case OSMessage::MESSAGE_SET_FPS_LIMIT:
			
			g_frameDelayMS = int(m.m_x);
			
			break;

		case OSMessage::MESSAGE_SET_VIDEO_MODE:
			{
				LogMsg("Got set video mode message");
				GetBaseApp()->KillOSMessagesByType(OSMessage::MESSAGE_SET_VIDEO_MODE);
				//GetBaseApp()->SetVideoMode(Width, Height, false, 0);
				//int isInFullscreen = EM_ASM_INT_V(return !!(document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement));
				int width, height, isFullscreen;
				
				/*
				emscripten_get_canvas_size(&width, &height, &isFullscreen);
				*/

				EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", &width, &height); 
				if (r != EMSCRIPTEN_RESULT_SUCCESS) 
				{
					LogMsg("Error getting canvas size of #canvas");

					/* handle error */ 
				}
				EmscriptenFullscreenChangeEvent e; 
				r = emscripten_get_fullscreen_status(&e); 
				if (r != EMSCRIPTEN_RESULT_SUCCESS) /* handle error */ 
				{
					LogMsg("Error with emscripten_get_fullscreen_status");

				}
				isFullscreen = e.isFullscreen; 

				LogMsg("Is full screen is %d - Width: %d Height: %d", isFullscreen, width, height);

				static bool bIsSoftFullscreen = false;

				if (bIsSoftFullscreen)
				{
					LogMsg("Leaving fullscreen mode");
					emscripten_exit_soft_fullscreen();
					bIsSoftFullscreen = false;
				} else
				{
					enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST);
					//emscripten_run_script("Module[\"requestFullScreen\"](false,false);"); for security reasons browsers won't let you do this from a non-button
					bIsSoftFullscreen = true;
				}

				LogMsg("Post setting: Is full screen is %d - Width: %d Height: %d", isFullscreen, width, height);

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
	static float fpsTimer = 0;


	if (g_frameDelayMS != 0)
	{
		
	
		while (fpsTimer > GetSystemTimeAccurate())
		{
			//emscripten_sleep(1);
			//Sleep(0);
		}

		//this should be 1000 not lower, but without this SetFPS(60) results in 55
		fpsTimer = float(GetSystemTimeAccurate()) + (1000.0f / (float(g_frameDelayMS)));
		
	}


	SDL_GL_SwapBuffers();
}

void CheckWindowsMessages()
{
	SDLEventLoop();
}

static inline const char *emscripten_event_type_to_string(int eventType) {
	const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize", 
		"scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange", 
		"visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload", 
		"batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout", "(invalid)" };
	++eventType;
	if (eventType < 0) eventType = 0;
	if (eventType >= sizeof(events)/sizeof(events[0])) eventType = sizeof(events)/sizeof(events[0])-1;
	return events[eventType];
}

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result)
{
	if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
	if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
	if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
	if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
	if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
	if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
	if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
	if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
	if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
	return "Unknown EMSCRIPTEN_RESULT!";
}

void UpdateHTML5Screen()
{
	SDL_GL_SwapBuffers();
	LogMsg("Did SDL_GL_SwapBuffers()");
}

EM_BOOL uievent_callback(int eventType, const EmscriptenUiEvent *e, void *userData)
{
	printf("%s, detail: %ld, document.body.client size: (%d,%d), window.inner size: (%d,%d), scrollPos: (%d, %d)\n",
		emscripten_event_type_to_string(eventType), e->detail, e->documentBodyClientWidth, e->documentBodyClientHeight,
		e->windowInnerWidth, e->windowInnerHeight, e->scrollTop, e->scrollLeft);

#ifdef RT_HTML5_USE_CUSTOM_MAIN
	double cssW, cssH;
	emscripten_get_element_css_size("#canvas", &cssW, &cssH);
	//ResetOrthoFlag();

	g_winVideoScreenX = cssW;
	g_winVideoScreenY = cssH;
	emscripten_set_canvas_element_size("#canvas",
		int(cssW), int(cssH));
	
	/*
	emscripten_set_canvas_element_size(0,
		int(cssW), int(cssH));

	
	InitSDLScreen();
	
	//emscripten_set_canvas_size(int(g_winVideoScreenX), int(g_winVideoScreenY));

	//emscripten_webgl_get_drawing_buffer_size
	//emscripten_set_resize_callback(nullptr, nullptr, false, uievent_callback);
	if (emscripten_webgl_get_current_context() == 0)
	{
		LogMsg("There is no context");
}
	else
	{
		EMSCRIPTEN_RESULT res;
		int drawingBufferWidth = -1;
		int drawingBufferHeight = -1;
		res = emscripten_webgl_get_drawing_buffer_size(emscripten_webgl_get_current_context(), &drawingBufferWidth, &drawingBufferHeight);
		LogMsg("WebGL drawingBuffer size is %d, %d", drawingBufferWidth, drawingBufferHeight);
	}

	LogMsg("Changing size to %d, %d", g_winVideoScreenX, g_winVideoScreenY);
	//GetBaseApp()->InitializeGLDefaults();
	*/

	glViewport(0, 0, GetPrimaryGLX(), GetPrimaryGLY());
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	GetBaseApp()->OnScreenSizeChange();
#endif
	
	return 0;
}


void FirstClickUnlock()
{
	LogMsg("Unlocking audio");
	//this kill and init is only needed for FMOD

	GetAudioManager()->ReinitForHTML5();
	GetAudioManager()->Play("audio/blank.wav");

}
#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{

	/*
	LogMsg("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, movement: (%ld,%ld), canvas: (%ld,%ld), target: (%ld, %ld)\n",
		emscripten_event_type_to_string(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
		e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
		e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY, e->targetX, e->targetY);

		*/

	switch (eventType)
	{

	case EMSCRIPTEN_EVENT_MOUSEDOWN:
	case EMSCRIPTEN_EVENT_MOUSEUP:
	case EMSCRIPTEN_EVENT_MOUSEMOVE:

		float xPos = e->canvasX;
		float yPos = e->canvasY;
		ConvertCoordinatesIfRequired(xPos, yPos);

		switch (eventType)
		{
		case EMSCRIPTEN_EVENT_MOUSEDOWN:
			{
				static bool bFirstTime = true;
				if (bFirstTime)
				{
					//unlock audio on iOS
					FirstClickUnlock();
					bFirstTime = false;
				} 
			}

			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, e->button);
			break;
		case EMSCRIPTEN_EVENT_MOUSEUP:
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, e->button);
			break;
		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, e->button);
			break;

		}
		break;

	}
	
	return 0;
}

EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
/*
	LogMsg("%s, screen: (%ld,%ld), client: (%ld,%ld),%s%s%s%s button: %hu, buttons: %hu, canvas: (%ld,%ld), target: (%ld, %ld), delta:(%g,%g,%g), deltaMode:%lu\n",
		emscripten_event_type_to_string(eventType), e->mouse.screenX, e->mouse.screenY, e->mouse.clientX, e->mouse.clientY,
		e->mouse.ctrlKey ? " CTRL" : "", e->mouse.shiftKey ? " SHIFT" : "", e->mouse.altKey ? " ALT" : "", e->mouse.metaKey ? " META" : "", 
		e->mouse.button, e->mouse.buttons, e->mouse.canvasX, e->mouse.canvasY, e->mouse.targetX, e->mouse.targetY,
		(float)e->deltaX, (float)e->deltaY, (float)e->deltaZ, e->deltaMode);
		*/

	GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_MOUSEWHEEL, (float)e->deltaY, 0, 0, 0); //last parm is "winkey modifers"...

	return 0;
}

EM_BOOL focus_callback(int eventType, const EmscriptenFocusEvent* event, void* userData)
{
	
	if (event)
	{
		switch (eventType)
		{
			case EMSCRIPTEN_EVENT_BLUR:
			{
				LogMsg("Got blur");
				GetBaseApp()->OnEnterBackground();
				//s_ctx.m_eventQueue.postSuspendEvent(s_defaultWindow, Suspend::DidSuspend);
				return true;
			}
			case EMSCRIPTEN_EVENT_FOCUS:
			{
				LogMsg("Got generic focus");
				GetBaseApp()->OnEnterForeground();

				return true;
			}
			case EMSCRIPTEN_EVENT_FOCUSIN:
			{
				LogMsg("focus in");
				return true;
			}
			case EMSCRIPTEN_EVENT_FOCUSOUT:
			{
				LogMsg("focus out");
				return true;
			}
		}
	}

	return false;
}


EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{

	/*
	LogMsg("%s, numTouches: %d %s%s%s%s\n",
		emscripten_event_type_to_string(eventType), e->numTouches,
		e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "");
	
	*/

	for(int i = 0; i < e->numTouches; ++i)
	{

		const EmscriptenTouchPoint *t = &e->touches[i];
		
		/*
		LogMsg("  %ld: screen: (%ld,%ld), client: (%ld,%ld), page: (%ld,%ld), isChanged: %d, onTarget: %d, canvas: (%ld, %ld)\n",
			t->identifier, t->screenX, t->screenY, t->clientX, t->clientY, t->pageX, t->pageY, t->isChanged, t->onTarget, t->canvasX, t->canvasY);

			*/

		float xPos = (float)t->canvasX;
		float yPos = (float)t->canvasY;

		int touchID = (int)t->identifier;

		ConvertCoordinatesIfRequired(xPos, yPos);

		switch(eventType)
		{

		case EMSCRIPTEN_EVENT_TOUCHSTART:

			static bool bFirstTime = true;
			if (bFirstTime)
			{
				FirstClickUnlock();
				bFirstTime = false;
			} 

			touchID = g_touchManager.OnDown(touchID);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_START, xPos, yPos, touchID);
			//LogMsg("Sending touch down");
			break;


		case EMSCRIPTEN_EVENT_TOUCHEND:
			//LogMsg("Got touch up");
			touchID = g_touchManager.OnUp(touchID);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, touchID);
			break;

		case EMSCRIPTEN_EVENT_TOUCHMOVE:
			touchID = g_touchManager.OnMove(touchID);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_MOVE, xPos, yPos, touchID);
			break;

		case EMSCRIPTEN_EVENT_TOUCHCANCEL:

			LogMsg("Got touch cancel?");
			touchID = g_touchManager.OnUp(touchID);
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, xPos, yPos, touchID);
			break;

		}
	}

	return 0;
}

void ForceEmscriptenResize()
{
	double cssW, cssH;
	EMSCRIPTEN_RESULT r;
	emscripten_get_element_css_size(0, &cssW, &cssH);
}

bool IsStillLoadingPersistentData()
{
	return EM_ASM_INT({ 
		return Module.waitingFileReadSync;
		}) == 1;
}

bool IsStillSavingPersistentData()
{
	return EM_ASM_INT({
		return Module.waitingFileWriteSync;
		}) == 1;
}

void SyncPersistentData()
{
	if (IsStillLoadingPersistentData())
	{
		LogMsg("Ignoring SyncUserSaveData(), we're still loading our initial persistent data!");
		return;
	}

#ifdef _DEBUG
	LogMsg("Syncing data...");
#endif
	EM_ASM(
		//Module.print("Start File write sync..");
	Module.waitingFileWriteSync = 1;
	FS.syncfs(false, function(err) 
	{
		if (err)
		{
			Module.print("Uh oh, SyncPersistentData error: " + err);
		}
		else
		{
			Module.print("Synced save files.");
		}
		Module.waitingFileWriteSync = 0;
	});
	);
}

void LoadInitialSyncData()
{
	EM_ASM(
	//Module.print("start file load sync..");
	Module.waitingFileReadSync = 1;

	//populate our persistent data directories with existing persistent source data 
	//first parameter = "true" mean synchronize from Indexed Db to 
	//Emscripten file system,
	// "false" mean synchronize from Emscripten file system to Indexed Db
	//second parameter = function called when data are synchronized

	FS.syncfs(true, function(err)
	{
		if (err)
		{
			Module.print("Uh oh, syncfs error: " + err);
		}
		//Module.print("end file load sync..");
		Module.waitingFileReadSync = 0;
	});

	);
}

/*
For this to work, you need https://github.com/eligrey/FileSaver.js

 and this in your main HTML:

 //stuff for file saving
 <script src="FileSaver.js"> </script>

 <script>
 function saveFileFromMemoryFSToDisk(memoryFSname,localFSname)     // This can be called by C++ code
 {
	 var data=FS.readFile(memoryFSname);
	 var blob;
	 var isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
	 if(isSafari) {
	 blob = new Blob([data.buffer], {type: "application/octet-stream"});
	 } else {
	 blob = new Blob([data.buffer], {type: "application/octet-binary"});
	 }
	 saveAs(blob, localFSname);
 }
 </script>

*/


#ifdef _DEBUG
///#define RT_FAKE_PERSISTENCE
#endif
void AddPersistentFileFolder(string folderName) //should start with a slash
{
	//remove ending slash
	if (folderName[folderName.length() - 1] == '/')
	{
		folderName = folderName.substr(0, folderName.length() - 1);
	}

#ifdef RT_FAKE_PERSISTENCE
	CreateDirectoryRecursively("", folderName);
	LogMsg("RT_FAKE_PERSISTENCE is defined, creating fake folder %s", folderName.c_str());
#else
	EM_ASM_({

		var pSaveDir = Pointer_stringify($0);
	//Module.print("Creating save dir "+pSaveDir);

	//create your directory where we keep our persistent data
	FS.mkdir(pSaveDir);
	//mount persistent directory as IDBFS
	FS.mount(IDBFS, {}, pSaveDir);
	}, folderName.c_str());

#endif
}

void mainHTML()
{
	
	int w, h, fs;
	srand( (unsigned)time(NULL) );
	RemoveFile("log.txt", false);
	double cssW, cssH;
	EMSCRIPTEN_RESULT r;

	r = emscripten_get_element_css_size("#canvas", &cssW, &cssH);
	if (r != EMSCRIPTEN_RESULT_SUCCESS)
	{
		LogMsg("Error emscripten_get_element_css_size #canvas");
		/* handle error */
	}

	
	r = emscripten_get_canvas_element_size("#canvas", &w, &h); 
	if (r != EMSCRIPTEN_RESULT_SUCCESS) 
	{
		LogMsg("Error getting canvas size of #canvas");
		goto cleanup;
		/* handle error */ 
	}
	LogMsg("Init: RTT size: %dx%d, CS: %02gx%02g\n", w, h, cssW, cssH);
#ifdef RT_HTML5_USE_CUSTOM_MAIN
	g_winVideoScreenX = cssW;
	g_winVideoScreenY = cssH;
	//enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST);
#else
	g_winVideoScreenX = 480;
	g_winVideoScreenY = 320;

#endif
	//emscripten_run_script("Module[\"requestFullScreen\"](false,false);"); for security reasons browsers won't let you do this from a non-button
	//bIsSoftFullscreen = true;

	EmscriptenFullscreenChangeEvent e; 
	
	GetBaseApp()->OnPreInitVideo(); //gives the app level code a chance to override any of these parms if it wants to
	SetForcedOrientation(ORIENTATION_DONT_CARE);

	LogMsg("Setting up joystick");

	SDL_JoystickEventState(SDL_IGNORE);
	
	if (!InitSDL())
	{
		LogMsg("Couldn't init SDL: %s", SDL_GetError());
		goto cleanup;
	}

	LogMsg("Setting up screen");
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	
	LogMsg("Setting up persistent file system");
	
	//setup some java vars
	EM_ASM(
		Module.waitingFileReadSync = 0;
	Module.waitingFileWriteSync = 0;

	);

	
	
	
	AddPersistentFileFolder(GetSavePath());
	AddPersistentFileFolder(GetAppCachePath());
	
	
	LoadInitialSyncData();

#ifdef RT_HTML5_USE_CUSTOM_MAIN
	//enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST);

	//re-get this stuff, it's likely changed slightly

	emscripten_get_element_css_size("#canvas", &cssW, &cssH);
	
	LogMsg("Detected canvas size of %d, %d", (int)cssW, (int)cssH);
	g_winVideoScreenX = cssW;
	g_winVideoScreenY = cssH;

	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);

#endif

	r = emscripten_get_fullscreen_status(&e); 
	if (r != EMSCRIPTEN_RESULT_SUCCESS) /* handle error */ 
	{
		LogMsg("Error with emscripten_get_fullscreen_status");
	}

	fs = e.isFullscreen; 

	EMSCRIPTEN_RESULT ret;
	ret = emscripten_set_resize_callback(0, 0, 1, uievent_callback);


	ret = emscripten_set_mousedown_callback(0, 0, 1, mouse_callback);
	TEST_RESULT(emscripten_set_mousedown_callback);
	ret = emscripten_set_mouseup_callback(0, 0, 1, mouse_callback);
	TEST_RESULT(emscripten_set_mouseup_callback);
	ret = emscripten_set_mousemove_callback(0, 0, 1, mouse_callback);
	TEST_RESULT(emscripten_set_mousemove_callback);
	ret = emscripten_set_wheel_callback(0, 0, 1, wheel_callback);
	TEST_RESULT(emscripten_set_wheel_callback);


	//touch events
	ret = emscripten_set_touchstart_callback(0, 0, 1, touch_callback);
	TEST_RESULT(emscripten_set_touchstart_callback);
	ret = emscripten_set_touchend_callback(0, 0, 1, touch_callback);
	TEST_RESULT(emscripten_set_touchend_callback);
	ret = emscripten_set_touchmove_callback(0, 0, 1, touch_callback);
	TEST_RESULT(emscripten_set_touchmove_callback);
	ret = emscripten_set_touchcancel_callback(0, 0, 1, touch_callback);
	TEST_RESULT(emscripten_set_touchcancel_callback);


	//focus events
	ret = emscripten_set_blur_callback(0, 0, true, focus_callback);
	TEST_RESULT(emscripten_set_blur_callback);

	ret = emscripten_set_focus_callback(0, 0, true, focus_callback);
	TEST_RESULT(emscripten_set_focus_callback);

	ret = emscripten_set_focusin_callback(0, 0, true, focus_callback);
	TEST_RESULT(emscripten_set_focusin_callback);
	ret = emscripten_set_focusout_callback(0, 0, true, focus_callback);
	TEST_RESULT(emscripten_set_focusout_callback);


/*
	ret = emscripten_set_click_callback(0, 0, 1, mouse_callback);
	TEST_RESULT(emscripten_set_click_callback);
	ret = emscripten_set_dblclick_callback(0, 0, 1, mouse_callback);
	TEST_RESULT(emscripten_set_dblclick_callback);

	
*/

#ifndef RT_EMTERPRETER_ENABLED
	//warning, FPS limiting isn't going to respect what is set in SetFPS later...
	emscripten_set_main_loop(MainEventLoop, 0, 1);
#else

	
	while(1)
	{
		//our main loop
		static float fpsTimer = 0;

		
		if (!g_bRanInit && IsStillLoadingPersistentData())
		{
			LogMsg("waiting...");
			emscripten_sleep(100);
			
		}
		else
		{
			if (!g_bRanInit)
			{
				LogMsg("Initting BaseApp");
				if (!GetBaseApp()->Init())
				{
					LogError("Couldn't initialize game. Yeah.\n\nDid everything unzip right?");
					goto cleanup;
				}
				g_bRanInit = true;
			}

			MainEventLoop();


			//respect the FPS limit delay if needed
			bool bRanSleep = false;

			if (g_frameDelayMS != 0)
			{
				while (fpsTimer > GetSystemTimeAccurate())
				{
					bRanSleep = true;
					emscripten_sleep(1);
					//Sleep(0);
				}

				//this should be 1000 not lower, but without this SetFPS(60) results in 55
				fpsTimer = float(GetSystemTimeAccurate()) + (850.0f / (float(g_frameDelayMS)));
			}

			if (!bRanSleep)
			{
				//this must be run at least once per frame, even if we don't need the delay
				emscripten_sleep(1);
			}

		}
		//emscripten_sleep_with_yield(1); //not compatible unless the ASYNC stuff is enabled
	}
#endif

	LogMsg("Finished running");
	GetBaseApp()->Kill();

cleanup:

	if (g_pSDLJoystick)
	{
		SDL_JoystickClose(g_pSDLJoystick);
		g_pSDLJoystick = NULL;
	}

	SDL_Quit();
}


//old way
//int main(int argc, char *argv[])


#ifdef RT_HTML5_USE_CUSTOM_MAIN

extern "C" 
{
//new way that gives more flexibility on when it starts, but must now be manually called by the html.  See RTBareBones example's html or https://lyceum-allotments.github.io/2016/06/emscripten-and-sdl2-tutorial-part-7-get-naked-owl/
	void mainf()
	{
		mainHTML();
	}
}

#else
int main()
{
	mainHTML();
	return 0;
}
#endif


