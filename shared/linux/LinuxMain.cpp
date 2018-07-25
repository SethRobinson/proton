/*

NOTE:  Seth now uses the SDL2Main.cpp file instead of this for linux builds
*/


#include "BaseApp.h"
#include "LinuxUtils.h"
#include "util/VideoModeSelector.h"
#include "util/PassThroughPointerEventHandler.h"
#include "util/TouchDeviceEmulatorPointerEventHandler.h"
#include <SDL.h>

using namespace std;

bool g_bAppFinished = false;

SDL_Surface *g_screen = NULL;
SDL_Joystick *g_pSDLJoystick = NULL;
CL_Vec3f g_accelHold = CL_Vec3f(0,0,0);

bool g_isInForeground = true;

int g_winVideoScreenX = 0;
int g_winVideoScreenY = 0;

PointerEventHandler* gPointerEventHandler = NULL;

int GetPrimaryGLX() 
{
	return g_winVideoScreenX;
}

int GetPrimaryGLY() 
{
	return g_winVideoScreenY;
}

int initSDL_GLES()
{
	// used to get the result value of SDL operations
	int result;

	    const SDL_VideoInfo* info = NULL;

	// init SDL. This function is all it takes to init both
	// the audio and video.
	result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	if (result < 0)
	{
		return result;
	}


	/* Let's get some video information. */
	info = SDL_GetVideoInfo( );
	if (!info)
	{

		LogMsg("Error getting SDL video info: %s",  SDL_GetError( ));
		return -1;
	}

	LogMsg("SDL reports screen is %d, %d, setting primaries to that.", info->current_w, info->current_h);
	g_winVideoScreenX = info->current_w; //"win"? not the best naming! todo
	g_winVideoScreenY =info->current_h;


#if SDL_VERSION_ATLEAST(1, 3, 0)
	// we need to set up OGL to be using the version of OGL we want. This
	// example uses OGL 1.0. 2.0 is a completely different animal. If you fail
	// to speficy what OGL you want to use, it will be pure luck which one you
	// get. Don't leave that up to chance. 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);	// Force this to be version 1
#endif
	


	Uint32 videoFlags = SDL_OPENGL| SDL_FULLSCREEN;
	g_screen = SDL_SetVideoMode(GetPrimaryGLX() , GetPrimaryGLY(), 0, videoFlags);

	if (g_screen == NULL)
	{
		// couldn't make that screen
		LogMsg("error making screen");
		return 1;
	}

	LogMsg("Setting up GLES to %d by %d", g_screen->w, g_screen->h);
	
	
	
	LogMsg((char*)glGetString(GL_VERSION));

	// This sets the "caption" for whatever the window is. On windows, it's the window
	// title. On the palm, this functionality does not exist and the function is ignored.
	SDL_WM_SetCaption(GetAppName(), NULL);

	// Like SDL, we will have the standard of always returning 0 for
	// success, and nonzero for failure. If we're here, it means success!
	return 0;
}

bool InitSDL()
{
	LogMsg("initting SDL");
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
		
		case SDL_MOUSEBUTTONDOWN:
		{
			float xPos = ev.button.x;
			float yPos = ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			gPointerEventHandler->handlePointerDownEvent(xPos, yPos, ev.button.button);
			break;
		}
		
		case SDL_MOUSEBUTTONUP:
		{
			float xPos = ev.button.x;
			float yPos = ev.button.y;
			ConvertCoordinatesIfRequired(xPos, yPos);
			gPointerEventHandler->handlePointerUpEvent(xPos, yPos, ev.button.button);
			break;
		}
	
		case SDL_MOUSEMOTION:
		{
			float xPos = ev.motion.x;
			float yPos = ev.motion.y;
			ConvertCoordinatesIfRequired(xPos, yPos);

			// Always pass the same pointer id here since it's not possible to track multiple pointing devices with SDL_MOUSEMOTION
			gPointerEventHandler->handlePointerMoveEvent(xPos, yPos, SDL_BUTTON_LEFT);
			break;
		}

		case SDL_KEYDOWN:
		{
			if (ev.key.keysym.mod & KMOD_CTRL)
			{
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

int main(int argc, char *argv[])
{
	VideoModeSelector vms;

	string requestedVideoMode("iPhone Landscape");
	if (argc > 1) {
		if (string(argv[1]) == "-l") {
			LogMsg("%s", vms.modesAsString().c_str());
			return 0;
		}
		
		requestedVideoMode = argv[1];
	}
	
	srand( (unsigned)time(NULL) );
	RemoveFile("log.txt", false);
	
	const VideoMode *videoMode = vms.getNamedMode(requestedVideoMode);
	if (videoMode != NULL) {
		g_winVideoScreenX = videoMode->getX();
		g_winVideoScreenY = videoMode->getY();
		SetEmulatedPlatformID(videoMode->getPlatformID());
		SetForcedOrientation(videoMode->getOrientationMode());
	} else {
		LogMsg("Requested video mode '%s' not found. Setting default video mode.", requestedVideoMode.c_str());
		LogMsg("Get available video mode names by using the -l command line argument.");
		g_winVideoScreenX = 320;
		g_winVideoScreenY = 480;
		SetEmulatedPlatformID(PLATFORM_ID_LINUX);
		SetForcedOrientation(ORIENTATION_DONT_CARE);
	}

	GetBaseApp()->OnPreInitVideo(); //gives the app level code a chance to override any of these parms if it wants to

	if (!InitSDL())
	{
		LogMsg("Couldn't init SDL: %s", SDL_GetError());
		goto cleanup;
	}

	SDL_JoystickEventState(SDL_IGNORE);
	
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	
	if (!GetBaseApp()->Init())
	{
		LogError("Couldn't initialize game. Yeah.\n\nDid everything unzip right?");
		goto cleanup;
	}

	GetBaseApp()->OnScreenSizeChange();

	if (IsDesktop()) {
		gPointerEventHandler = new PassThroughPointerEventHandler;
	} else {
		gPointerEventHandler = new TouchDeviceEmulatorPointerEventHandler(SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT);
	}

	static Uint32 nextFrameTick = 0;
	static Uint32 frameDelayMS = 0;

	while(1)
	{
		if (g_bAppFinished)
		{
			break;
		}

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

		glFlush();
		glFinish();
		SDL_GL_SwapBuffers();

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

	delete gPointerEventHandler;

	SDL_Quit();

	return 0;
}
