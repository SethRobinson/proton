#include "PlatformPrecomp.h"
#include "BaseApp.h"
#include "Renderer/RTGLESExt.h"
#include "Renderer/SoftSurface.h"
#include "Renderer/RenderPipeline.h"

#ifdef PLATFORM_HTML5
#include <emscripten/emscripten.h>
#endif

#ifdef PLATFORM_OSX
#include "OSX/OSXUtils.h"
#endif

#ifdef _IRR_STATIC_LIB_
#include "Irrlicht/IrrlichtManager.h"
#endif

Entity * GetEntityRoot() 
{
	assert(IsBaseAppInitted() && "Base app should be initted before calling this");
	return GetBaseApp()->GetEntityRoot();
}

RenderBatcher g_globalBatcher;
bool g_isLoggerInitted = false;
bool g_isBaseAppInitted = false;
bool g_bUseBorderlessFullscreenOnWindows = true; //extern and change yourself if needed, done this way to stay compatible with old stuff.  (true was old default behavior)
bool g_defaultSmoothing = true;

bool IsBaseAppInitted()
{
	return g_isBaseAppInitted;
}

BaseApp::BaseApp()
{
		m_bDisableSubPixelBlits = false;
		m_bCheatMode = false;
		m_memUsed = m_texMemUsed = 0;	
		g_isLoggerInitted = true;	
		m_bInitted = false;
		m_bConsoleVisible = false;
		m_bManualRotation = false;
		SetFPSVisible(false);
		m_bIsInBackground = false;
		SetInputMode(INPUT_MODE_NORMAL);
		m_version = "No default Version"; // this is over written by network messages that come from IOS and Android. For other platforms (like windows), it will remain this.
		m_autoScreenshotParmsChecked = false;
		m_autoScreenshotAtMS = 0;
		m_autoScreenshotQuit = false;
		m_autoScreenshotFrames = 0;
		m_autoScreenshotStartWallMS = 0;
		m_autoScreenshotUpdateStampMS = 0;
		m_autoScreenshotEngineMS = 0;
		
		m_touchTracker.resize(C_MAX_TOUCHES_AT_ONCE);
		ClearError();
		g_isBaseAppInitted = true;
}

BaseApp::~BaseApp()
{
	m_entityRoot.RemoveAllEntities();
	m_resourceManager.KillAllResources();
	m_commandLineParms.clear();
	g_isLoggerInitted = false;
}

void BaseApp::Kill()
{
	g_isBaseAppInitted = false;
	delete this;
}

eTimingSystem GetTiming()
{
	return GetBaseApp()->GetActiveTimingSystem();
}

void BaseApp::PrintGLString(const char *name, GLenum s)
{
	const char *v = (const char *) glGetString(s);
	LogMsg("GL %s = %s\n", name, v);
}

void BaseApp::InitializeGLDefaults()
{
	rtMatrixMode(GL_MODELVIEW);
	glDepthMask(GL_TRUE);
	rtEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	rtDisable(GL_ALPHA_TEST);
	glDisable( GL_BLEND );
	rtEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	rtEnableClientState(GL_VERTEX_ARRAY);	
	rtDisableClientState(GL_COLOR_ARRAY);	
	rtDisableClientState(GL_NORMAL_ARRAY);
	rtDisable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	rtColor4x(1 << 16, 1 << 16, 1 << 16, 1 << 16);
	glClearColor(0,0,0,255);
}

void OnKillKeyboard(VariantList* pVList)
{
	if (!IsBaseAppInitted()) return;

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CLOSE_TEXT_BOX;
	GetBaseApp()->AddOSMessage(o);
}

bool BaseApp::Init()
{
	
	m_gameTimer.Reset();
	CheckAutoScreenshotParms(); //after the timer reset so deterministic mode's zeroed timeline sticks
	GetEntityRoot()->SetName("root");

	if (m_bInitted)	
	{
		LogMsg("Why are we initting more than once?");
		return true;
	}

	m_bInitted = true;

	LogMsg("Proton SDK v%s", PROTON_VERSION_STRING);

	CHECK_GL_ERROR();

	PrintGLString("Version", GL_VERSION);
	PrintGLString("Vendor", GL_VENDOR);
	PrintGLString("Renderer", GL_RENDERER);
	//PrintGLString("Extensions", GL_EXTENSIONS); //this can crash systems?!?
	//Too many extensions causes a buffer overrun issue?

	InitializeGLDefaults();
	
	LogMsg("Initialized GL defaults");
	GLint depthbits;
	glGetIntegerv(GL_DEPTH_BITS, &depthbits);
	LogMsg("GL depth buffer: %d bit", depthbits);

	CHECK_GL_ERROR();

	if (GetAudioManager())
	{
		GetAudioManager()->Init();
	}
	LogMsg("Initialized Audio");

	m_gameTimer.Reset(); //another one
	

	
#ifdef PLATFORM_ANDROID
	LogMsg("Killing keyboard at launch because sometimes it opens by itself");
	VariantList vList;
	GetMessageManager()->CallStaticFunction(OnKillKeyboard, 50, &vList, TIMER_SYSTEM);
	
	//Try again for slow loaders.  This seems to be an Android OS 7 problem (?)
	GetMessageManager()->CallStaticFunction(OnKillKeyboard, 500, &vList, TIMER_SYSTEM);


#endif
	
	return true;
}

void DrawConsole()
{
	//not implemented
}

//Automation helper for the screenshot-based render regression tests (see tests/ in
//the repo root).  Launch any Proton app with "-autoscreenshot <file.bmp> <delayMS>"
//and it will write a BMP of its framebuffer once its app timer passes delayMS.  Add
//"-autoquit" to have the app close itself right after.  The parm also switches the
//engine to a deterministic mode (a locked 16ms timestep and a fixed random seed) so
//animations and particles are in identical poses every run and shots can be compared
//pixel for pixel.  Completely inert without the parms.

//set by -autoscreenshotfps: platform main loops that normally ignore the fps
//limit during captures honor it when this is true
bool g_autoScreenshotRespectFpsLimit = false;

void BaseApp::CheckAutoScreenshotParms()
{
	if (m_autoScreenshotParmsChecked) return;
	m_autoScreenshotParmsChecked = true;

	for (unsigned int i = 0; i < m_commandLineParms.size(); i++)
	{
		string parm = ToLowerCaseString(m_commandLineParms[i]);
		if (parm == "-autoscreenshot" && i + 2 < m_commandLineParms.size())
		{
			m_autoScreenshotFile = m_commandLineParms[i + 1];
			m_autoScreenshotAtMS = atoi(m_commandLineParms[i + 2].c_str());

#ifdef PLATFORM_IOS
			//on a real device the sandbox means a relative path is the only sane
			//input, so resolve it against the app's Documents dir (the simulator
			//harness passes absolute /tmp paths, which still work there)
			if (!m_autoScreenshotFile.empty() && m_autoScreenshotFile[0] != '/')
			{
				m_autoScreenshotFile = GetSavePath() + m_autoScreenshotFile;
			}
#endif
		}
		if (parm == "-autoquit")
		{
			m_autoScreenshotQuit = true;
		}

		//captures normally run uncapped (finishes fast, and the perf sidecar
		//measures real throughput), but a scenario that depends on wall-clock
		//events (e.g. RTDink's online update check finishing before the menu
		//appears) can pin its capture speed with -autoscreenshotfps <n>
		if (parm == "-autoscreenshotfps" && i + 1 < m_commandLineParms.size())
		{
			int fps = atoi(m_commandLineParms[i + 1].c_str());
			if (fps > 0)
			{
				g_autoScreenshotRespectFpsLimit = true;
				SetFPSLimit(float(fps));
				LogMsg("autoscreenshot: capture speed pinned to %d fps", fps);
			}
		}
	}

	if (!m_autoScreenshotFile.empty())
	{
		m_gameTimer.SetLockedTimestepMS(16);
		srand(31337);
		LogMsg("autoscreenshot: locked 16ms timestep and fixed rand seed for deterministic capture");
	}
}

void BaseApp::ProcessAutoScreenshot()
{
	CheckAutoScreenshotParms(); //normally already done in Init(), but some platforms deliver parms late

	if (m_autoScreenshotFile.empty()) return;

	//speed check: with the locked timestep the frame count until capture is fixed,
	//so the wall clock those frames take is a benchmark.  Count from the first
	//frame (skipping it, since it carries startup costs) to the capture.
	if (m_autoScreenshotStartWallMS == 0)
	{
		m_autoScreenshotStartWallMS = GetSystemTimeAccurate();
	}
	else
	{
		m_autoScreenshotFrames++;
		if (m_autoScreenshotUpdateStampMS != 0)
		{
			//engine time this frame: from Update() start to here (end of Draw),
			//which excludes the platform loop's swap/vsync wait
			m_autoScreenshotEngineMS += GetSystemTimeAccurate() - m_autoScreenshotUpdateStampMS;
		}
	}

	if (m_gameTimer.GetTick() < m_autoScreenshotAtMS) return;

#ifndef _CONSOLE
	//SoftSurface::BlitFromScreenFixed doesn't exist in console builds
	SoftSurface s;
	if (s.Init(GetScreenSizeX(), GetScreenSizeY(), SoftSurface::SURFACE_RGBA))
	{
		s.BlitFromScreenFixed(0, 0, 0, 0, GetScreenSizeX(), GetScreenSizeY());
		s.WriteBMPOut(m_autoScreenshotFile);
		LogMsg("Wrote autoscreenshot to %s at tick %u", m_autoScreenshotFile.c_str(), m_gameTimer.GetTick());

		//the perf sidecar the test harness reads to catch "everything got slower" bugs.
		//fps is wall-clock (display/browser caps apply); engineMS is the average
		//Update+Draw cost per frame, which vsync can't hide.
		double elapsedMS = GetSystemTimeAccurate() - m_autoScreenshotStartWallMS;
		float fps = (elapsedMS > 0) ? float(double(m_autoScreenshotFrames) * 1000.0 / elapsedMS) : 0.0f;
		float engineMS = (m_autoScreenshotFrames > 0) ? float(m_autoScreenshotEngineMS / double(m_autoScreenshotFrames)) : 0.0f;
		string perfFile = m_autoScreenshotFile + ".perf.txt";
		FILE *fp = fopen(perfFile.c_str(), "wb");
		if (fp)
		{
			fprintf(fp, "frames=%d elapsedMS=%d fps=%.1f engineMS=%.3f\n", m_autoScreenshotFrames, int(elapsedMS), fps, engineMS);
			fclose(fp);
		}
		LogMsg("autoscreenshot perf: %d frames in %d ms = %.1f fps, %.3f engine ms/frame", m_autoScreenshotFrames, int(elapsedMS), fps, engineMS);

#ifdef PLATFORM_HTML5
		//on the web the files only exist in MEMFS, so hand them to whoever served
		//the page (the test harness runs a tiny local server that accepts these).
		//The perf sidecar goes first; the BMP upload is the harness's "done" signal.
		EM_ASM({
			try {
				var name = UTF8ToString($0);
				var perfName = name + '.perf.txt';
				fetch('autoscreenshot_upload?name=' + encodeURIComponent(perfName), { method: 'POST', body: FS.readFile(perfName) })
					.finally(function() {
						fetch('autoscreenshot_upload?name=' + encodeURIComponent(name), { method: 'POST', body: FS.readFile(name) });
					});
			} catch(e) { console.log('autoscreenshot upload failed: ' + e); }
		}, m_autoScreenshotFile.c_str());
#endif
	}
#endif
	m_autoScreenshotFile.clear(); //only fire once

	if (m_autoScreenshotQuit)
	{
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_FINISH_APP;
		AddOSMessage(o);

#ifdef PLATFORM_IOS
		//iOS ignores MESSAGE_FINISH_APP (apps aren't supposed to quit
		//themselves), which left every harness capture running on the device
		//forever - sitting on screen in locked-timestep mode with the FPS
		//display pinned at 62, repeatedly mistaken for a real fps problem.
		//The capture is written and the harness is done with us: exit.
		LogMsg("autoscreenshot: capture done, exiting (iOS has no app-quit message)");
		exit(0);
#endif
	}
}

//where the FPS/debug overlay starts.  On iOS the physical screen's rounded
//corners clip the top-left, so nudge it inward (20,5 in logical pixels,
//scaled so it's the same physical distance on retina displays)
static float GetDebugOverlayX()
{
#ifdef PLATFORM_IOS
	return 2.0f + 20.0f * GetProtonPixelScaleFactor();
#else
	return 2.0f;
#endif
}

static float GetDebugOverlayY()
{
#ifdef PLATFORM_IOS
	return 2.0f + 5.0f * GetProtonPixelScaleFactor();
#else
	return 2.0f;
#endif
}

void BaseApp::Draw()
{

#ifdef _DEBUG
//LogMsg("**********FRAME START");
#endif
    VariantList vList(Variant(0,0));
    
	m_sig_render(&vList);

	if (GetFPSVisible())
	{
		char stTemp[256];
		snprintf(stTemp, sizeof(stTemp), "fps: %d - M: %.2f, T: %.2f A: %.2f F: %.2f", m_gameTimer.GetFPS(),  (float(m_memUsed)/1024)/1024, (float(m_texMemUsed)/1024)/1024,  float(GetAudioManager()->GetMemoryUsed()/1024)/ 1024, float(GetFreeMemory()/1024)/ 1024);
	
#ifdef _IRR_STATIC_LIB_
		int prims = 0;
		if (GetIrrlichtManager()->GetDriver())
		{
			prims = GetIrrlichtManager()->GetDriver()->getPrimitiveCountDrawn();
		}
		char stExtra[256];
		sprintf(stExtra, " Prims: %d", prims);
		strcat(stTemp, stExtra);
#endif		
	
#ifdef PLATFORM_FLASH
		char stExtra[256];
		sprintf(stExtra, " Flash: %.2f", float(GetNativeMemoryUsed())/1024/1024);
		strcat(stTemp, stExtra);

#endif

#ifdef PLATFORM_ANDROID
		char stExtra[256];
		sprintf(stExtra, " MemUsed: %.2f", float(GetNativeMemoryUsed()) / 1024 / 1024);
		strcat(stTemp, stExtra);

#endif

		if (GetFont(FONT_SMALL)->IsLoaded())
		{
			GetFont(FONT_SMALL)->DrawScaled(GetDebugOverlayX(), GetDebugOverlayY(), stTemp, 0.7f);
		}
	}

	//draw the console messages?
	if (GetConsoleVisible())
	{
		DrawConsole();
	}

	switch (GetLastError())
	{
	case ERROR_MEM:
		GetFont(FONT_SMALL)->DrawScaled(GetDebugOverlayX(), GetDebugOverlayY()+12, "LOW MEM!", 0.7f);
		break;

	case ERROR_SPACE:
		GetFont(FONT_SMALL)->DrawScaled(GetDebugOverlayX(), GetDebugOverlayY()+12, "LOW STORAGE SPACE!", 0.7f);
		break;
            
        case ERROR_NONE:
            
        break;
	}

	SetupOrtho();
	g_globalBatcher.Flush();
	if (GetForceAspectRatio() != 0)
	{
		if (GetForceAspectRatio() > 1.0f)
		{
			//need to draw bars on top and bottom
			DrawFilledRect(0, -(GetScreenSizeYf()), GetScreenSizeXf(), GetScreenSizeYf(), MAKE_RGBA(0, 0, 0, 255));
			DrawFilledRect(0, GetScreenSizeYf()-1, GetScreenSizeXf(), GetScreenSizeYf()*3, MAKE_RGBA(0, 0, 0, 255));
		}
		else
		{
			//need to draw bars on left/right
			DrawFilledRect(-(GetScreenSizeXf()), 0, GetScreenSizeXf(), GetScreenSizeYf(), MAKE_RGBA(0, 0, 0, 255));
			DrawFilledRect(GetScreenSizeXf()-1, 0, GetScreenSizeXf(), GetScreenSizeYf(), MAKE_RGBA(0, 0, 0, 255));
		}
	}

	ProcessAutoScreenshot(); //inert unless the -autoscreenshot command line parm was used
}

#ifdef RT_RUN_STATIC_UPDATE
void RunStaticUpdateThing();
#endif

void BaseApp::Update()
{
	if (!m_autoScreenshotFile.empty())
	{
		m_autoScreenshotUpdateStampMS = GetSystemTimeAccurate(); //perf sidecar: frame's engine work starts here
	}

	m_gameTimer.Update();
#ifdef RT_RUN_STATIC_UPDATE
//Don't ask, for Seth
	RunStaticUpdateThing();
#endif

	if (GetMessageManager()) GetMessageManager()->Update();
	if (GetAudioManager()) GetAudioManager()->Update();
	m_sig_update(NULL);
}

void BaseApp::OnScreenSizeChange()
{
	
#ifdef _DEBUG
	LogMsg("Changing screen-size to %d, %d, %d", GetScreenSizeX(), GetScreenSizeY(), GetOrientation());
#endif
	
	GenerateSetPerspectiveFOV(C_APP_FOV, GetScreenSizeXf()/ GetScreenSizeYf(),0.1f,500.0f);
	m_sig_onScreenSizeChanged();
}

void BaseApp::SetConsoleVisible( bool bNew )
{
	m_bConsoleVisible = bNew;
}

void BaseApp::OnMessage(Message &m)
{
	static VariantList v;
	
	v.Reset();
	
	switch (m.GetClass())
	{
		case MESSAGE_CLASS_GUI:
			switch (m.GetType())
			{
			
			case MESSAGE_TYPE_GUI_CLICK_START:
			case MESSAGE_TYPE_GUI_CLICK_END:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(float(m.GetParm1()), float(m.GetParm2()) );
					v.Get(2).Set(uint32(m.GetParm3()));
					v.Get(3).Set(m.GetParm4());
			
					if (m.GetType() == MESSAGE_TYPE_GUI_CLICK_START)
					{
					//	LogMsg("Clicked finger %d, down is %d", m.GetParm3(), (int)m_touchTracker[m.GetParm3()].IsDown());
						m_touchTracker[m.GetParm3()].SetIsDown(true);
						m_touchTracker[m.GetParm3()].SetPos(v.Get(1).GetVector2());
						m_touchTracker[m.GetParm3()].SetWasHandled(false);
						m_touchTracker[m.GetParm3()].SetWasPreHandled(false);
					} else
					{
					//	LogMsg("Released finger %d, down is %d", m.GetParm3(), (int)m_touchTracker[m.GetParm3()].IsDown());
						m_touchTracker[m.GetParm3()].SetIsDown(false);
					}


					m_sig_input(&v);
					break;
				}
			
			case MESSAGE_TYPE_GUI_CLICK_MOVE:
			case MESSAGE_TYPE_GUI_CLICK_MOVE_RAW:
				{
				
				if (!IsDesktop())
				{
					if (!m_touchTracker[m.GetParm3()].IsDown())
					{
						//ignore this, we don't want a move message from something that isn't fricken' down.  At least
						//one known HP device is known to do this...
						break;
					}
				}

					v.Get(0).Set(float(m.GetType()));
					v.Get(1).Set(float(m.GetParm1()), float(m.GetParm2()) );
					v.Get(2).Set(uint32(m.GetParm3()));
					v.Get(3).Set(m.GetParm4());

					if (m.GetType() == MESSAGE_TYPE_GUI_CLICK_MOVE)
					{
						m_touchTracker[m.GetParm3()].SetPos(v.Get(1).GetVector2());
					}

					if (m_inputMode == INPUT_MODE_NORMAL)
					{
						m_sig_input(&v);
					} else
					{
						m_sig_input_move(&v);
					}

					break;
				}
			
			case MESSAGE_TYPE_GUI_ACCELEROMETER:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(m.Get().GetVector3());
					m_sig_accel(&v);				
				}
				break;
			//like MESSAGE_TYPE_GUI_CHAR, but handles up AND down events, and ignores things like key-repeat, better for
			//arcade action
			case MESSAGE_TYPE_GUI_CHAR_RAW:
				{
					v.Get(0).Set(uint32(m.GetParm1()));
					v.Get(1).Set(uint32(m.GetParm2()));
					v.Get(2).Set(uint32(m.GetParm3()));
					v.Get(3).Set(m.GetParm4());
					m_sig_raw_keyboard(&v);
				}
				break;

			//usually used for text input
			case MESSAGE_TYPE_GUI_CHAR:
				{
#ifdef _DEBUG
					//LogMsg("Got char: %c (%d)", (char)m.GetParm1(), int(m.GetParm1()));
#endif
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(0,0);
					v.Get(2).Set(uint32(m.GetParm1()));
					v.Get(3).Set(m.GetParm4());
					m_sig_input(&v);
				}
				break;

			case MESSAGE_TYPE_GUI_PASTE:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(0,0);
					v.Get(2).Set(m.Get());
					m_sig_input(&v);
					break;
				}
	
			case MESSAGE_TYPE_GUI_TRACKBALL:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(m.Get().GetVector3());
					m_sig_trackball(&v);
					break;
				}

			case MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_SHOW:
			case MESSAGE_TYPE_HW_TOUCH_KEYBOARD_WILL_HIDE:
				{
					v.Get(0).Set((float)m.GetType());
					m_sig_hardware(&v);
					break;
				}


			case MESSAGE_TYPE_OS_CONNECTION_CHECKED:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(float(m.GetParm1()), float(m.GetParm2()) );
					m_sig_os(&v);
				}
				break;
			
			case MESSAGE_TYPE_GUI_TOGGLE_FULLSCREEN:
				{
					OnFullscreenToggleRequest();
				}
				break;

			case MESSAGE_TYPE_APP_VERSION:
				{
					m_version = m.GetStringParm();
					break;
				}
		
			case MESSAGE_TYPE_GUI_MOUSEWHEEL:
				{
					v.Get(0).Set((float)m.GetType());
					v.Get(1).Set(float(m.GetParm1()), float(m.GetParm2()) );
					v.Get(2).Set(uint32(m.GetParm3()));
					v.Get(3).Set(m.GetParm4());
					v.Get(4).Set(m.GetParm1()); //total hack, because using parm1 causes problems because it's treated as x/y and modified when used with GUI objects.
					//So use Get(4) instead

					m_sig_input(&v);
					break;
				}

			default:
				break; //the game can subscribe to these messages itself, we don't need to do anything
			}

	break;

	case MESSAGE_CLASS_GAME:

		switch (m.GetType())
		{
		case MESSAGE_TYPE_PLAY_SOUND:
			if (GetAudioManager())
			{
				GetAudioManager()->Play(m.GetVarName());
			}
			break;
		case MESSAGE_TYPE_SET_SOUND_ENABLED:
			GetAudioManager()->SetSoundEnabled(m.Get().GetUINT32() != 0);
			break;
		case MESSAGE_TYPE_PRELOAD_SOUND:
			if (GetAudioManager())
			{
				GetAudioManager()->Preload(ReplaceMP3(m.GetVarName()));
			}
			break;

		case MESSAGE_TYPE_PLAY_MUSIC:
			if (GetAudioManager())
			{
				GetAudioManager()->Play(ReplaceMP3(m.GetVarName()), true, true);
			}
			break;
		
		case MESSAGE_TYPE_PLAY_MUSIC_FORCE_STREAMING:
			if (GetAudioManager())
			{
				GetAudioManager()->Play(ReplaceMP3(m.GetVarName()), true, true, true, true);
			}
			break;

		case MESSAGE_TYPE_VIBRATE:

			if (GetAudioManager())
			{
				GetAudioManager()->Vibrate(m.Get().GetUINT32());
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void BaseApp::AddOSMessage( OSMessage &m )
{
	assert(IsBaseAppInitted() && "Base app should be initted before calling AddOSMessage");

	m_OSMessages.push_back(m);
}

void BaseApp::KillOSMessagesByType(OSMessage::eMessageType type)
{
	//It's a deque, making it tricky to delete stuff from the middle.  I'll do it this way, speed isn't important
	//as this is used rarely.

	deque <OSMessage>::iterator itor = m_OSMessages.begin();

	deque <OSMessage> temp;

	for (;itor != m_OSMessages.end(); itor++)
	{
		if (itor->m_type != type)
		{
			temp.push_back(*itor);
		}
	}

	m_OSMessages = temp;
}

unsigned int BaseApp::GetGameTick()
{
	return m_gameTimer.GetGameTick();
}

eTimingSystem BaseApp::GetActiveTimingSystem()
{
	if (GetGameTickPause()) return TIMER_SYSTEM;
	return TIMER_GAME;
}

unsigned int BaseApp::GetTickTimingSystem( eTimingSystem timingSystem )
{
	if (timingSystem == TIMER_SYSTEM) return m_gameTimer.GetTick();

	assert(timingSystem == TIMER_GAME);
	return m_gameTimer.GetGameTick();
}

int BaseApp::GetDeltaTick()
{
	return m_gameTimer.GetDeltaTick();
}

void LogError ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
#ifdef WIN32
	vsnprintf_s( buffer, logSize, logSize, traceStr, argsVA );
#else
	vsnprintf( buffer, logSize, traceStr, argsVA );
#endif
	va_end( argsVA );
	LogMsg("ERROR: %s", buffer);

	GetBaseApp()->SetConsoleVisible(true); //make sure we see the error
	//assert(!"Got an error, your majesty...");
}

unsigned int GetTick( eTimingSystem timingSystem )
{
	return GetBaseApp()->GetTickTimingSystem(timingSystem);
}

ResourceManager * GetResourceManager()
{
	return GetBaseApp()->GetResourceManager();
}

void BaseApp::SetManualRotationMode( bool bRotation )
{
	//if (GetPlatformID() == PLATFORM_ID_BBX) bRotation = false; //on BBX we never have to do that
	LogMsg("AppManualRotation set to %d", int(bRotation));
	m_bManualRotation = bRotation;
}


void BaseApp::OnMemoryWarning()
{
	LogMsg("Got memory warning");
}

void BaseApp::OnEnterBackground()
{
	if (!m_bIsInBackground)
	{
		m_bIsInBackground = true;
#ifdef _DEBUG	
		LogMsg("Entering background");
#endif
	 
	#ifndef PLATFORM_ANDROID
		if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
		m_sig_unloadSurfaces();	
	#endif
	
		
	if (GetPlatformID() != PLATFORM_ID_ANDROID)
	{
		//android will do it elsewhere, but for other platforms we fake this message here
		m_sig_pre_enterbackground(NULL); 
	}

		m_sig_enterbackground(NULL);
	}

	GetAudioManager()->Suspend();
	//ResetTouches(); //Turns out we don't need this
    
}

void BaseApp::OnEnterForeground()
{
	GetAudioManager()->Resume();

	if (m_bIsInBackground)
	{
		m_bIsInBackground = false;
#ifdef _DEBUG
		LogMsg("Entering foreground");
#endif
	
	#ifndef PLATFORM_ANDROID  //wtf?!
		if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
		{
			GetBaseApp()->m_sig_loadSurfaces(); //for anyone who cares
		}
	#endif


		m_sig_enterforeground(NULL);
	}
}

void BaseApp::AddCommandLineParm( string parm )
{
	m_commandLineParms.push_back(parm);

#ifdef RT_SHADER_PIPELINE_AVAILABLE
	//the shader pipeline is the default in builds that compile it; -shaderpipeline
	//is kept as an explicit (now redundant) opt-in so old launch scripts still work
	if (ToLowerCaseString(parm) == "-shaderpipeline")
	{
		SP_ResetState();
		g_bShaderPipelineActive = true;
		LogMsg("Shader pipeline requested via command line parm");
	}

	//escape hatch for A/B comparison against the legacy fixed-function path.
	//Flip as early as possible so the very first GL default setup routes correctly.
	if (ToLowerCaseString(parm) == "-fixedpipeline")
	{
#ifdef RT_SHADER_PIPELINE_ONLY
		LogMsg("-fixedpipeline ignored: this build compiled out the fixed-function path");
#else
		g_bShaderPipelineActive = false;
		LogMsg("Fixed-function pipeline requested via command line parm");
#endif
	}
#endif
}

vector<string> BaseApp::GetCommandLineParms()
{
	return m_commandLineParms;
}

void BaseApp::SetAccelerometerUpdateHz(float hz) //another way to think of hz is "how many times per second to update"
{
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_SET_ACCELEROMETER_UPDATE_HZ;
	o.m_x = hz;
	GetBaseApp()->AddOSMessage(o);
}

void BaseApp::SetAllowScreenDimming(bool bAllowDimming) 
{
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_ALLOW_SCREEN_DIMMING;
    if (bAllowDimming)
    {
        o.m_x = 1;
    } else
    {
        o.m_x = 0;
    }
    
	GetBaseApp()->AddOSMessage(o);
}

void BaseApp::SetFPSLimit(float fps) 
{
	if (fps >= 0.0f)
	{
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_SET_FPS_LIMIT;
		o.m_x = fps;
		GetBaseApp()->AddOSMessage(o);
	}
}
 
void BaseApp::SetVideoMode(int width, int height, bool bFullScreen, float aspectRatio) //aspectRatio should be 0 to ignore
{
	//this message is only going to be processed by platforms that can change size during runtime and have such a thing as fullscreen
	
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_SET_VIDEO_MODE;
	o.m_x =(float) width;
	o.m_y = (float) height;
	o.m_fullscreen = bFullScreen;	
	o.m_fontSize = aspectRatio;
	GetBaseApp()->AddOSMessage(o);
}



#ifdef _WINDOWS_

//yes, hacky.  Will cleanup when I add the OSX support for this
extern bool g_bIsFullScreen;
#endif

bool BaseApp::OnPreInitVideo()
{
	//only called for desktop systems
	//override in App.* if you want to do something here.  You'd have to
	//extern these vars from main.cpp to change them...
	
	//SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);
	
#if defined(PLATFORM_WINDOWS) || defined (PLATFORM_OSX) || defined (PLATFORM_LINUX) 
	SetPrimaryScreenSize(1024, 768);

#endif

	return true; //no error
}


void BaseApp::OnFullscreenToggleRequest()
{
#ifdef _WINDOWS_
	
	static int savex =0;
	static int savey =0;

	if (g_bIsFullScreen)
	{
		if (savex == 0)
		{
			savex = GetPrimaryGLX();
			savey = GetPrimaryGLY();
		}

		GetBaseApp()->SetVideoMode(savex, savey, false);

	} else
	{
		if (!g_bUseBorderlessFullscreenOnWindows)
		{
			//use current rez and fullscreen it

			savex = GetPrimaryGLX();
			savey = GetPrimaryGLY();
			GetBaseApp()->SetVideoMode(savex, savey, true);

		}
		else
		{
			//fake fullscreen using borderless window of current resolution (more compatible)

			savex = GetPrimaryGLX();
			savey = GetPrimaryGLY();

			GetBaseApp()->SetVideoMode(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), true);

		}

	}
#endif

#ifdef PLATFORM_OSX
	OSXToggleFullscreen();
#endif
}


void BaseApp::ResetTouches()
{
	for (int i=0; i < C_MAX_TOUCHES_AT_ONCE; i++)
	{
		if (m_touchTracker[i].IsDown())
		{
			LogMsg("Finger %d is down, sending fake release", i);
			//release it with a fake message
			GetMessageManager()->SendGUIEx(MESSAGE_TYPE_GUI_CLICK_END, m_touchTracker[i].GetPos().x, m_touchTracker[i].GetPos().y, i);
		}
	}

}

TouchTrackInfo * BaseApp::GetTouch( int index )
{
//legacy null safety check, technically UB (a modern optimizer is free to delete it) but keeping behavior as-is
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#endif
	if (this == 0) return NULL;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	if (index >= C_MAX_TOUCHES_AT_ONCE)
	{
		assert(!"Uh no");
		return &m_touchTracker[C_MAX_TOUCHES_AT_ONCE-1];
	}

	return &m_touchTracker[index];
}

int BaseApp::GetTotalActiveTouches()
{
	int count = 0;
	
	//why am I doing C_MAX_TOUCHES_AT_ONCE-1 instead of C_MAX_TOUCHES_AT_ONCE? Well, it's because SendFakeInputMessageToEntity()
	//uses the last touch to send fake mouse presses and should always be ignored. - Seth

	for (int i=0; i < C_MAX_TOUCHES_AT_ONCE-1; i++)
	{
		if (m_touchTracker[i].IsDown())	
		{
			count++;
		}
	}

	return count;
}

string BaseApp::GetAppVersion()
{
	return m_version;
}

void TouchTrackInfo::SetWasHandled( bool bNew, Entity *pEntity )
{
	m_pEntityThatHandledIt = pEntity;
	m_bHandled = bNew;
}

void TouchTrackInfo::SetWasPreHandled( bool bNew, Entity *pEntity /*= NULL*/ )
{
	m_bPreHandled = bNew;
	m_pEntityThatPreHandledIt = pEntity;
}

bool GetDefaultSmoothing()
{
	return g_defaultSmoothing;
}
void SetDefaultSmoothing(bool bNew)
{
	g_defaultSmoothing = bNew;
}
