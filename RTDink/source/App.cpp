/*
 *  App.cpp
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */
#include "PlatformPrecomp.h"
#include "App.h"
#include "GUI/MainMenu.h"
#include "Entity/EntityUtils.h"//create the classes that our globally library expects to exist somewhere.
#include "dink/dink.h"
#include "GUI/GameMenu.h"
#include "util/archive/TarHandler.h"
#include "Renderer/SoftSurface.h"
#include "GUI/BrowseMenu.h"
#include "Entity/SliderComponent.h"
#include "GUI/OptionsMenu.h"
#include "FileSystem/FileSystemZip.h"
#include "Entity/ArcadeInputComponent.h"
#include "GUI/ExpiredMenu.h"
#include <time.h>
#include "Gamepad/GamepadManager.h"
#include "Gamepad/GamepadProvideriCade.h"
#include "GUI/PopUpMenu.h"
#include "GUI/PauseMenu.h"

#ifdef PLATFORM_HTML5
#include "html5/HTML5Utils.h"
#include "html5/SharedJSLIB.h"
#endif

#ifdef WINAPI

extern int g_winVideoScreenX;
extern int g_winVideoScreenY;
extern bool g_bUseBorderlessFullscreenOnWindows;
void AddText(const char *tex, const char *filename);

#include "StackWalker/StackUtils.h"
extern bool g_bIsFullScreen;
#endif

extern bool g_script_debug_mode;
extern Surface g_transitionSurf;

#ifdef RT_MOGA_ENABLED
#include "Gamepad/GamepadProviderMoga.h"
#endif

#ifdef RT_CHARTBOOST_ENABLED
#include "Ad/AdProviderChartBoost.h"
#endif

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT
#include "Gamepad/GamepadProvider60Beat.h"
#endif

//#define FORCE_DMOD_SUPPORT

MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

GamepadManager g_gamepadManager;
GamepadManager * GetGamepadManager() {return &g_gamepadManager;}

#ifdef __APPLE__

#if TARGET_OS_IPHONE == 1
  //it's an iPhone or iPad
  //#include "Audio/AudioManagerOS.h"
  //AudioManagerOS g_audioManager;
	//#include "Audio/AudioManagerDenshion.h"

  //AudioManagerDenshion g_audioManager;

  #include "Audio/AudioManagerFMODStudio.h"
  AudioManagerFMOD g_audioManager;
#else
  //it's being compiled as a native OSX app
#include "Audio/AudioManagerFMODStudio.h"
  AudioManagerFMOD g_audioManager; //dummy with no sound

  //in theory, CocosDenshion should work for the Mac builds, but right now it seems to want a big chunk of
  //Cocos2d included so I'm not fiddling with it for now

  //#include "Audio/AudioManagerDenshion.h"
  //AudioManagerDenshion g_audioManager;
#endif

#else

#if defined RT_WEBOS || defined RTLINUX
#include "Audio/AudioManagerSDL.h"
  AudioManagerSDL g_audioManager; //sound in windows and WebOS
  //AudioManager g_audioManager; //to disable sound
#elif defined ANDROID_NDK
#include "Audio/AudioManagerAndroid.h"
  AudioManagerAndroid g_audioManager; //sound for android
#elif defined PLATFORM_BBX
#include "Audio/AudioManagerBBX.h"
  //AudioManager g_audioManager; //to disable sound
  AudioManagerBBX g_audioManager;
#elif defined PLATFORM_HTML5
#include "Audio/AudioManagerFMODStudio.h"
AudioManagerFMOD g_audioManager;

//AudioManager g_audioManager; //to disable sound

#elif defined PLATFORM_FLASH
  //AudioManager g_audioManager; //to disable sound
#include "Audio/AudioManagerFlash.h"
  AudioManagerFlash g_audioManager;
#else

  //in windows
  //AudioManager g_audioManager; //to disable sound

#ifdef RT_FLASH_TEST
	#include "Audio/AudioManagerFlash.h"
    AudioManagerFlash g_audioManager;
#else
	// #include "Audio/AudioManagerAudiere.h"
	// AudioManagerAudiere g_audioManager;  //Use Audiere for audio

	#include "Gamepad/GamepadProviderVita.h"
	#include "Audio/AudioManagerSDL.h"
	 AudioManagerSDL g_audioManager; //if we wanted FMOD sound in windows
#endif

#endif
#endif



#ifdef ANDROID_NDK
void SetPreferSDCardForStorage(bool bNew);
#endif

AudioManager * GetAudioManager(){return &g_audioManager;}

App *g_pApp = NULL;
BaseApp * GetBaseApp() 
{
	if (!g_pApp)
	{
		g_pApp = new App;
	}

	return g_pApp;
}

App * GetApp() 
{
	return g_pApp;
}

const char * GetAppName()
{

#ifdef WINAPI

	if (GetApp())
	{
		static char name[64];
		sprintf(name, "Dink Smallwood HD %s", GetApp()->GetVersionString().c_str());
		return name;
   }
#endif

	return "Dink Smallwood HD";


	
};


App::App()
{
	m_logFileHandle = NULL;

	//http://www.rtsoft.com

	m_bGhostMode = false;
#ifdef ANDROID_NDK
	SetPreferSDCardForStorage(true);
#endif
	
	m_bDidPostInit = false;
	m_bHasDMODSupport = true;
	//for mobiles
	m_version = 1.92f;
	m_versionString = "V1.92";
	m_build = 1;
	m_bCheatsEnabled = false;

	//for Win/mac
	m_desktopVersion = m_version;
	m_desktopVersionString = m_versionString; 
	m_desktopBuild = 1;
	m_bForceAspectRatio = true;
	
}

App::~App()
{

	assert(m_logFileHandle);
	if (m_logFileHandle)
		fclose(m_logFileHandle);


	//L_ParticleSystem::deinit();
}


void App::AddIcadeProvider()
{
	GamepadProvider * pProv = GetGamepadManager()->AddProvider(new GamepadProvideriCade); //use iCade, this actually should work with any platform...
	GetBaseApp()->SetAllowScreenDimming(false);
	if (pProv)
	{
		pProv->m_sig_failed_to_connect.connect(1, boost::bind(&App::OniCadeDisconnected, this, _1));
	}
}

bool App::GetForceAspectRatio()
{
	return m_bForceAspectRatio;
}

bool App::UseClassicEscapeMenu()
{

	if (GetEmulatedPlatformID() == PLATFORM_ID_HTML5 && !GetApp()->GetUsingTouchScreen())
	{
		return true;
	}

	if (IsDesktop())
	{
		return true;
	}

	return false;
}

void App::OniCadeDisconnected(GamepadProvider *pProvider)
{
	LogMsg("Dealing with icade disconnect");
	GetGamepadManager()->RemoveProviderByName("iCade");
	GetApp()->RemoveAndroidKeyboardKeys();

	GetApp()->GetVar("check_icade")->Set(uint32(0));

	Entity *pOptions = GetEntityRoot()->GetEntityByName("OptionsMenu");
	if (pOptions)
	{
        LogMsg("Found options");
		Entity *pCheckBox = pOptions->GetEntityByName("check_icade");
		if (pCheckBox)
		{
            LogMsg("Found checkbox");
			SetCheckBoxChecked(pCheckBox, false, true);
		}
	}
}

bool App::DoesCommandLineParmExist(string parm)
{
	vector<string> parms = GetBaseApp()->GetCommandLineParms();
	parm = ToLowerCaseString(parm);
	for (int i = 0; i < parms.size(); i++)
	{
		if (ToLowerCaseString(parms[i]) == parm) return true;
	}
	return false;
}

bool App::Init()
{

#ifdef WINAPI
	InitUnhandledExceptionFilter();
#endif


	//GetBaseApp()->SetDisableSubPixelBlits(true);
	SetDefaultButtonStyle(Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);
	SetManualRotationMode(false);

	bool bScaleScreenActive = true; //if true, we'll stretch every screen to the coords below
	int scaleToX = 480;
	int scaleToY = 320;

	//if (IsTabletSize() || IsDesktop())
	{
		scaleToX = 1024;
		scaleToY = 768;
	}


	/*
	if (IsIphoneSize || IsIphone4Size || IsIPADSize)
	{
		bScaleScreenActive = false;
	}
	*/

	switch (GetEmulatedPlatformID())
	{
		//special handling for certain platforms to tweak the video settings

	case PLATFORM_ID_WEBOS:
		//if we do this, everything will be stretched/zoomed to fit the screen
		if (IsIPADSize)
		{
			//doesn't need rotation
			SetLockedLandscape(false);  //because it's set in the app manifest, we don't have to rotate ourselves
			SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
			if (bScaleScreenActive)
				SetupFakePrimaryScreenSize(scaleToX,scaleToY); //game will think it's this size, and will be scaled up
		} 
		else
		{
			//but the phones do
			SetLockedLandscape(true); //we don't allow portrait mode for this game
			if (bScaleScreenActive)
				SetupFakePrimaryScreenSize(scaleToX,scaleToY); //game will think it's this size, and will be scaled up
		}

		break;

	case PLATFORM_ID_IOS:
		SetLockedLandscape(true); //we stay in portrait but manually rotate, gives better fps on older devices
		if (bScaleScreenActive)
			SetupFakePrimaryScreenSize(scaleToX,scaleToY); //game will think it's this size, and will be scaled up
		break;

	default:

		//Default settings for other platforms

		SetLockedLandscape(false); //we don't allow portrait mode for this game
		SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
		if (bScaleScreenActive)
			SetupFakePrimaryScreenSize(scaleToX,scaleToY); //game will think it's this size, and will be scaled up
	}


	//L_ParticleSystem::init(2000);
	SetInputMode(INPUT_MODE_SEPARATE_MOVE_TOUCHES); //this game has so much move touching, I handle them separately for performance reasons

	if (m_bInitted)	
	{
		return true;
	}
	
	if (!BaseApp::Init())
	{
		return false;
	}


	LogMsg("Initializing Dink HD %s", GetVersionString().c_str());

	//add fake parms


#ifdef PLATFORM_HTML5
	
	char *pStringTemp = JLIB_GetURL();

	string crap = pStringTemp;
	free(pStringTemp); //emscripten thing, trust me

	int n = crap.find_last_of('?');
	if (n == string::npos)
	{
		//I thought maybe this would be useful later to use # instead of ? to prevent caching? Dunno, doesn't hurt to add though
		n = crap.find_last_of('#');
	}
	if (n != string::npos)
	{
		//we should fake add whatever this command is
		string final = crap.substr(n + 1, crap.length() - n);
		GetBaseApp()->AddCommandLineParm(final);
	}


#endif
	//string crap = "http://www.rtsoft.com/web/dink/?-game http://www.rtsoft.com/web/srchmili.dmod";

	vector<string> parm = GetBaseApp()->GetCommandLineParms();

	string parms;
	for (int i = 0; i < parm.size(); i++)
	{
		if (i != 0) parms += " ";
		parms += parm[i];
	}

	if (!parm.empty())
	{
		string text = string("Run with parms: " + string(parms) + "\r\n\r\n");

#ifdef WINAPI
		OutputDebugString(text.c_str());
#endif
		AddTextToLog(text.c_str(), (GetSavePath() + "log.txt").c_str());
	}


	m_adManager.Init();

#ifdef RT_CHARTBOOST_ENABLED
	AdProviderChartBoost *pProvider = new AdProviderChartBoost;

#ifdef PLATFORM_ANDROID
	assert(!"No longer using chartboost!");

	pProvider->SetupInfo("", ""); //Dink HD Android

#else
	
	pProvider->SetupInfo("", ""); //Dink HD iOS 

#endif
	
	
	
	m_adManager.AddProvider(pProvider);
	pProvider->CacheShowInterstitial();
//	pProvider->CacheShowMoreApps();

	m_adManager.GetProviderByType(AD_PROVIDER_CHARTBOOST)->ShowInterstitial();
	//m_adManager.GetProviderByType(AD_PROVIDER_CHARTBOOST)->ShowMoreApps();
#endif

	LogMsg("Save path is %s", GetSavePath().c_str());

	if (GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		//g_dglo.m_bUsingDinkPak = true; //but we're not tho
	}



	if (g_dglo.m_bUsingDinkPak)
	{
		FileSystemZip *pFileSystem = new FileSystemZip();
		if (!pFileSystem->Init(GetBaseAppPath()+ "dink/dink.pak"))
		{
			LogMsg("Error finding APK file to load resources");
		}

		//pFileSystem->SetRootDirectory("dink");

		GetFileManager()->MountFileSystem(pFileSystem);
		
	}

	if (GetPlatformID() != PLATFORM_ID_ANDROID)
	{
	/*
			FileSystemZip *pFileSystem = new FileSystemZip();
			if (!pFileSystem->Init(GetBaseAppPath()+ "dink/dink.pak"))
			{
				LogMsg("Error finding APK file to load resources");
			}

			GetFileManager()->MountFileSystem(pFileSystem);
			*/
	/*	
			vector<string> contents = pFileSystem->GetContents();
			LogMsg("Listing all %d files.", contents.size());
			
			for (int i=0; i < contents.size(); i++)
			{
				LogMsg("%s", contents[i].c_str());
			}
	*/

	}

	
	switch (GetPlatformID())
	{
	case PLATFORM_ID_WINDOWS:
	case PLATFORM_ID_BBX:
	case PLATFORM_ID_WEBOS:
	case PLATFORM_ID_HTML5:
		CreateDirectoryRecursively(GetSavePath(), GetDMODRootPath());
		break;


	default:
		
		CreateAppCacheDirIfNeeded();
		break;
	}

	
	if (IsLargeScreen())
	{
		if (!GetFont(FONT_SMALL)->Load("interface/font_normalx2.rtfont")) return false;
		if (!GetFont(FONT_LARGE)->Load("interface/font_bigx2.rtfont")) return false;
	} else
	{
		if (!GetFont(FONT_SMALL)->Load("interface/font_normal.rtfont")) return false;
		if (!GetFont(FONT_LARGE)->Load("interface/font_big.rtfont")) return false;
	}

	//GetFont(FONT_SMALL)->SetSmoothing(false);
	#ifndef FORCE_DMOD_SUPPORT

		if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
		{
			//m_bHasDMODSupport = false;
		}
	#endif

	#ifdef _DEBUG
		GetBaseApp()->SetFPSVisible(true);
	#endif

	bool bFileExisted;
	m_varDB.Load("save.dat", &bFileExisted);
	
	GetApp()->GetVarWithDefault("smoothing",uint32(0))->GetUINT32();

	GetApp()->GetVarWithDefault("buttons",uint32(0));

	GetApp()->GetVarWithDefault("music_vol",1.0f)->GetFloat();
	GetApp()->GetVarWithDefault("gui_transparency",0.35f)->GetFloat();

  
	GetApp()->GetVarWithDefault("checkerboard_fix", uint32(1)); //default to on for Windows

	GetGamepadManager()->AddProvider(new GamepadProviderVita);

#ifdef RT_MOGA_ENABLED
	GetGamepadManager()->AddProvider( new GamepadProviderMoga);
	GetBaseApp()->SetAllowScreenDimming(false);
#endif

	if (GetVar("check_icade")->GetUINT32() != 0)
	{
		AddIcadeProvider();
	}

#if defined(PLATFORM_IOS) && defined(RT_IOS_60BEAT_GAMEPAD_SUPPORT)
	//startup the 60beat gamepad stuff.. really, we should only do this if they've checked to use it in options
	//or such because their driver may slow us down.. unsure
	if (GetVar("check_60beat")->GetUINT32() != 0)
	{
		//startup the 60beat gamepad stuff
		GetGamepadManager()->AddProvider(new GamepadProvider60Beat);
		GetBaseApp()->SetAllowScreenDimming(false);
	}
#endif



	if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS || GetEmulatedPlatformID() == PLATFORM_ID_OSX || GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		//should we draw that onscreen GUI stuff for Dink?	
		m_bUsingTouchScreen = false;
	} else
	{
		m_bUsingTouchScreen = true;
	}

	if (IsIPADSize && GetEmulatedPlatformID() != PLATFORM_ID_WEBOS)
	{
		GetApp()->GetVarWithDefault("fpsLimit", Variant(uint32(VIDEO_FPS_LIMIT_OFF)))->GetUINT32();
	}
	
	UpdateVideoSettings();
	//preload audio


if (GetEmulatedPlatformID() == PLATFORM_ID_IOS || GetEmulatedPlatformID() == PLATFORM_ID_HTML5 )
{
	//use our own DLS, as iPhone/iPad don't have any midi system
	g_audioManager.SetDLS("dink/midi/TimGM6mbTiny.dls");
}

#ifdef _WIN32

	//temporary while I make movies
	//GetApp()->SetCheatsEnabled(true);
#endif
	
	GetApp()->SetCheatsEnabled(true);

	bool bSound = m_varDB.GetVarWithDefault("sound", uint32(1))->GetUINT32() != 0;
	GetAudioManager()->SetSoundEnabled(bSound);

	//GetAudioManager()->SetMusicEnabled(!GetApp()->GetVar("musicDisabled")->GetUINT32());
	GetAudioManager()->SetMusicVol(GetApp()->GetVar("music_vol")->GetFloat());
	GetAudioManager()->Preload("audio/click.wav");
	InitDinkPaths(GetBaseAppPath(), "dink", "");
	

	GetBaseApp()->m_sig_pre_enterbackground.connect(1, boost::bind(&App::OnPreEnterBackground, this, _1));
	
	
	GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&App::OnLoadSurfaces, this));

	//when screen size changes we'll unload surfaces
	GetBaseApp()->m_sig_unloadSurfaces.connect(1, boost::bind(&App::OnUnloadSurfaces, this));
	
#ifdef WINAPI
	int videox = GetApp()->GetVarWithDefault("video_x", uint32(640))->GetUINT32();
	int videoy = GetApp()->GetVarWithDefault("video_y", uint32(480))->GetUINT32();
	int fullscreen = GetApp()->GetVarWithDefault("fullscreen", uint32(1))->GetUINT32();
	bool borderlessfullscreen = GetApp()->GetVarWithDefault("fullscreen", uint32(0))->GetUINT32();
		
	if (DoesCommandLineParmExist("-window") || DoesCommandLineParmExist("-windowed"))
	{
		fullscreen = false;
		GetApp()->GetVar("fullscreen")->Set(uint32(0));
	}

	if (DoesCommandLineParmExist("-debug") )
	{
		g_script_debug_mode = true;
	}

	if (fullscreen && g_bUseBorderlessFullscreenOnWindows)
	{
		LogMsg("Setting fullscreen...");
		//GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_TOGGLE_FULLSCREEN, 0, 0);  //lParam holds a lot of random data about the press, look it up if
		g_bIsFullScreen = false; //because we're using toggle..
		OnFullscreenToggleRequest();
	}
	else
	{
		/*
		if (videox != 0 && videoy != 0)
		{
			//remember old setup
			SetVideoMode(videox, videoy, false, 0);
		}
		*/
	}
	
#endif
	return true;
}

void App::OnPreEnterBackground(VariantList *pVList)
{
	SaveAllData();
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

void App::Kill()
{
	
	if (!IsInBackground())
	{
		SaveAllData();
	}
	
	finiObjects();
	
	BaseApp::Kill();
	g_pApp = NULL; //make sure nobody elses access this
}


void App::RemoveAndAttachAllAvailableGamepads()
{
	ArcadeInputComponent *pComp = (ArcadeInputComponent*) GetEntityRoot()->GetComponentByName("ArcadeInput");
	assert(pComp);

	for (int i=0; i < GetGamepadManager()->GetGamepadCount(); i++)
	{
		Gamepad *pPad = GetGamepadManager()->GetGamepad((eGamepadID)i);
		pPad->ConnectToArcadeComponent(pComp, true, true);

		//if we cared about the analog sticks too, we'd do this:
		//pPad->m_sig_left_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
		//pPad->m_sig_right_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
	}
}

void App::RemoveAndroidKeyboardKeys()
{
	ArcadeInputComponent *pComp = (ArcadeInputComponent*) GetEntityRoot()->GetComponentByName("ArcadeInput");
	//first clear out all old ones to be safe
	VariantList vList((string)"Keyboard");
	pComp->GetFunction("RemoveKeyBindingsStartingWith")->sig_function(&vList);
}

void App::AddDroidKeyboardKeys()
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
	{

	
	ArcadeInputComponent *pComp = (ArcadeInputComponent*) GetEntityRoot()->GetComponentByName("ArcadeInput");

	RemoveAndroidKeyboardKeys();

	//I think the ASWZ binding thing is for the control pad on the xperia play??

	AddKeyBinding(pComp, "KeyboardLeft",'A', VIRTUAL_KEY_DIR_LEFT);
	AddKeyBinding(pComp, "KeyboardRight",'S', VIRTUAL_KEY_DIR_RIGHT);
	AddKeyBinding(pComp, "KeyboardUp", 'W', VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "KeyboardDown", 'Z', VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "KeyboardAltMagic", 8, VIRTUAL_KEY_GAME_MAGIC);

	AddKeyBinding(pComp, "KeyboardInventory", 'I', VIRTUAL_KEY_GAME_INVENTORY);
	//AddKeyBinding(pComp, "KeyboardAltTalk", 13, VIRTUAL_KEY_GAME_TALK); //not sure what this was for, special android key?  It caused ineventory to open AND dink to talk so was bad I think
	AddKeyBinding(pComp, "KeyboardFire", VIRTUAL_KEY_DIR_CENTER, VIRTUAL_KEY_GAME_FIRE);
	AddKeyBinding(pComp, "KeyboardFire2", 'X', VIRTUAL_KEY_GAME_FIRE);
//	AddKeyBinding(pComp, "KeyboardAltFire", VIRTUAL_KEY_SHIFT, VIRTUAL_KEY_GAME_FIRE);
	}
}

void App::Update()
{
	BaseApp::Update();
	m_adManager.Update();
	g_gamepadManager.Update();

	if (!m_bDidPostInit)
	{
		m_bDidPostInit = true;
		m_special = GetSystemData() != C_PIRATED_NO;
		
		//build a GUI node
		Entity *pGUIEnt = GetEntityRoot()->AddEntity(new Entity("GUI"));

#ifdef RT_EXPIRING
		time_t rawtime, expiretime;
		time(&rawtime);
	
		expiretime = 1290050035 + ((3600)*24)*8; //expire in 8 days from Nov 18
		bool bExpired = expiretime < rawtime;

		if (bExpired)
		{
			ExpiredMenuCreate(pGUIEnt);
			return;
		}
	
#endif

		ArcadeInputComponent *pComp = (ArcadeInputComponent*) GetEntityRoot()->AddComponent(new ArcadeInputComponent);
		
		RemoveAndAttachAllAvailableGamepads();

		//add key bindings, I may want to move these later if I add a custom key config...

		AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
		AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
		AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
		AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
		AddKeyBinding(pComp, "Talk", ' ', VIRTUAL_KEY_GAME_TALK);

		AddKeyBinding(pComp, "GamePadInventory", VIRTUAL_DPAD_SELECT, VIRTUAL_KEY_GAME_INVENTORY);
		AddKeyBinding(pComp, "GamePadInventory2", VIRTUAL_DPAD_BUTTON_UP, VIRTUAL_KEY_GAME_INVENTORY);
		AddKeyBinding(pComp, "GamePadEscape", VIRTUAL_DPAD_START, VIRTUAL_KEY_BACK, true);
		AddKeyBinding(pComp, "GamePadFire", VIRTUAL_DPAD_BUTTON_DOWN, VIRTUAL_KEY_GAME_FIRE);
		AddKeyBinding(pComp, "GamePadTalk", VIRTUAL_DPAD_BUTTON_RIGHT, VIRTUAL_KEY_GAME_TALK);
		AddKeyBinding(pComp, "GamePadMagic", VIRTUAL_DPAD_BUTTON_LEFT, VIRTUAL_KEY_GAME_MAGIC);
		
		AddKeyBinding(pComp, "GamePadSpeedup", VIRTUAL_DPAD_LBUTTON, 'M', true);
		AddKeyBinding(pComp, "GamePadSpeedup2", VIRTUAL_DPAD_RBUTTON, 9);
	
		AddKeyBinding(pComp, "GamePadInventory3", VIRTUAL_DPAD_LTRIGGER, VIRTUAL_KEY_GAME_INVENTORY);
		AddKeyBinding(pComp, "GamePadPause", VIRTUAL_DPAD_RTRIGGER, VIRTUAL_KEY_BACK, true);


//if (IsDesktop())
{
		AddKeyBinding(pComp, "Inventory", 13, VIRTUAL_KEY_GAME_INVENTORY);
		AddKeyBinding(pComp, "Magic", VIRTUAL_KEY_SHIFT, VIRTUAL_KEY_GAME_MAGIC);
		AddKeyBinding(pComp, "Fire", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);
		AddKeyBinding(pComp, "Speedup", 9, 9); //handle tab
		AddKeyBinding(pComp, "Quicksave", VIRTUAL_KEY_F4, VIRTUAL_KEY_F4);
		AddKeyBinding(pComp, "Quickload", VIRTUAL_KEY_F8, VIRTUAL_KEY_F8);
		AddKeyBinding(pComp, "DinkHDMenu", VIRTUAL_KEY_F1, VIRTUAL_KEY_F1);
}

		if (GetVar("check_icade")->GetUINT32() == 0)
		{

			AddDroidKeyboardKeys();
		}

#ifdef _DEBUG
//		BrowseMenuCreate(pGUIEnt);
	MainMenuCreate(pGUIEnt);
#else
		MainMenuCreate(pGUIEnt);
#endif
	}
	else
	{
		
		CheckForHotkeys();
	}
}

void App::Draw()
{
	BaseApp::Draw();
}

void App::OnScreenSizeChange()
{
#ifdef _DEBUG
	LogMsg("Got OnScreenSizeChange");
#endif

	BaseApp::OnScreenSizeChange();
	if (GetPrimaryGLX() != 0)
	{
		SetupOrtho();
		DinkOnForeground(); //rebuild lost surfaces
		g_dglo.m_bForceControlsRebuild = true;
		if (GetDinkGameState() != DINK_GAME_STATE_PLAYING)
		{
			PrepareForGL();
		}

	}

#ifdef WINAPI
	GetApp()->GetVar("fullscreen")->Set(uint32(g_bIsFullScreen));
	GetApp()->GetVar("videox")->Set(uint32(GetPrimaryGLX()));
	GetApp()->GetVar("videoy")->Set(uint32(GetPrimaryGLY()));
	//GetApp()->GetVarWithDefault("borderless_fullscreen", uint32(g_bUseBorderlessFullscreenOnWindows))->Set(uint32(0));

#endif

}

void App::GetServerInfo( string &server, uint32 &port )
{
#if defined (_DEBUG) && defined(WIN32)
//	server = "localhost";
//	port = 8080;

	server = "rtsoft.com";
	port = 80;

#else

	server = "rtsoft.com";
	port = 80;
#endif
}

int App::GetSpecial()
{
	return m_special; //1 means pirated copy
}

Variant * App::GetVar( const string &keyName )
{
	return GetShared()->GetVar(keyName);
}

std::string App::GetVersionString()
{
	if (IsDesktop()) return m_desktopVersionString;
	return m_versionString;
}

float App::GetVersion()
{
	if (IsDesktop()) return m_desktopVersion;
	return m_version;
}

int App::GetBuild()
{
	if (IsDesktop()) return m_desktopBuild;
	return m_build;
}

void App::OnMemoryWarning()
{
	BaseApp::OnMemoryWarning();

	GetAudioManager()->KillCachedSounds(false, true, 0, 1, false);
	DinkUnloadUnusedGraphicsByUsageTime(100); //unload anything not used in the last second
}

void App::UpdateVideoSettings()
{
	eVideoFPS v = (eVideoFPS)GetApp()->GetVarWithDefault("fpsLimit", Variant(uint32(VIDEO_FPS_LIMIT_OFF)))->GetUINT32();
	SetFPSLimit(60);
	
#ifdef _DEBUG
	//SetFPSLimit(20);

#endif
	//SetFPSLimit(v);
};

void App::SaveSettings()
{
	m_varDB.Save("save.dat");
}

void App::SaveAllData()
{

	if (GetDinkGameState() == DINK_GAME_STATE_PLAYING)
	{
	//	SaveState(GetSavePath()+"state.dat");
		SaveState(g_dglo.m_savePath+"continue_state.dat", false);
		WriteLastPathSaved(g_dglo.m_savePath); //so we know what to reload
	}

	//GetAudioManager()->StopMusic();
	SaveSettings();
	//SyncPersistentData();
}

void App::OnEnterBackground()
{
	//SaveAllData();
//	DinkUnloadGraphicsCache();


/*
//I don't think we really need to uncache everything.  If low memory is a problem we could though..

	GetAudioManager()->KillCachedSounds(false, true, 0, 1, false);
	LogMsg("Unloading some graphics");
	DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
*/

	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	if (GetPrimaryGLX() == 0) return; //not ready, probably minimized on Windows

	BaseApp::OnEnterForeground();
    
}

bool App::GetIconsOnLeft()
{
	return GetShared()->GetVar("buttons")->GetUINT32() != 0;
}

//below is a sort of hack that allows "release" builds on windows to override the settings of whatever the shared main.cpp is telling
//us for window sizes
#ifdef _WINDOWS_
#include "win/app/main.h"
#endif



bool App::OnPreInitVideo()
{
	if (!BaseApp::OnPreInitVideo()) return false;

#ifdef PLATFORM_HTML5
	//don't do anything, we get the size from the browser
#endif

//#if !defined(_DEBUG) && defined(WINAPI)
#ifdef WINAPI

#ifdef RT_SCRIPT_BUILD
		
		SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);
		g_winVideoScreenX = 1024;
		g_winVideoScreenY = 768;

#endif

//		g_winVideoScreenX = 800;
//		g_winVideoScreenY = 1280;

//windows only
		
		
		
		if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS)
		{
			VariantDB temp;
			temp.Load("save.dat");
			Variant *pVarX = temp.GetVarIfExists("videox");
			Variant *pVarY = temp.GetVarIfExists("videoy");
			if (pVarX && pVarY && pVarX->GetUINT32() != 0 && pVarY->GetUINT32() != 0)
			{

				g_winVideoScreenX = pVarX->GetUINT32();
				g_winVideoScreenY = pVarY->GetUINT32();
			}

			g_bIsFullScreen = temp.GetVarWithDefault("fullscreen", uint32(1))->GetUINT32();

			if (DoesCommandLineParmExist("-window") || DoesCommandLineParmExist("-windowed"))
			{
				g_bIsFullScreen = false;
				GetApp()->GetVar("fullscreen")->Set(uint32(0));
			}

			g_bUseBorderlessFullscreenOnWindows = temp.GetVarWithDefault("borderless_fullscreen", uint32(0))->GetUINT32() != 0;
		}
		
#endif
		return true;
}

//for palm webos and android
const char * GetBundlePrefix()
{

	char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

//applicable to Palm WebOS builds only
const char * GetBundleName()
{
	char * bundleName = "rtdink";
	return bundleName;
}


void ImportSaveState(string physicalFname)
{

	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	string originalDMODDir = g_dglo.m_dmodGameDir;
	string newDMODDir;

	bool bSuccess = GetDMODDirFromState(physicalFname, newDMODDir);
	//newDMODDir += "/";

	if (!bSuccess)
	{
		//RemoveFile(fName, false);
		GetAudioManager()->Play("audio/buzzer2.wav");
		PopUpCreate(pMenu, "Error loading save state.  Probably an older version, sorry.", "", "cancel", "Continue", "", "", true);
		//SyncPersistentData();

	}
	else
	{
		LogMsg("We are in %s, but now we need %s", originalDMODDir.c_str(), newDMODDir.c_str());

		if (!newDMODDir.empty())
		{

			if (!FileExists(GetDMODRootPath() + newDMODDir + "dmod.diz"))
			{
				PopUpCreate(pMenu, "Can't find file " + newDMODDir + "dmod.diz" + ", maybe you need to install the DMOD first?", "", "cancel", "Continue", "", "", true);
				return;
			}
		}

		if (originalDMODDir == newDMODDir)
		{
			LoadStateWithExtra(physicalFname);
		}
		else
		{
			//whole different dmod, we need to get fancy here
			LogMsg("Switching to correct DMOD dir for this quicksave...");
			//SetDinkGameState(DINK_GAME_STATE_NOT_PLAYING);

			Entity *pNewMenu = DinkQuitGame();
			KillEntity(pMenu);

			DisableAllButtonsEntity(pNewMenu);
			SlideScreen(pNewMenu, false);
			GetMessageManager()->CallEntityFunction(pNewMenu, 500, "OnDelete", NULL);

			InitDinkPaths(GetBaseAppPath(), "dink", RemoveTrailingBackslash(newDMODDir));
			GameCreate(pNewMenu->GetParent(), 0, physicalFname);
			GetBaseApp()->SetGameTickPause(false);
		}
	}
}

void ImportNormalSaveSlot(string fileName, string outputFileName)
{
	//well... directly loading it is possible but.. uhh.. DMODs may need their own startup
	//scripts so I better just copy the file over instead.
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	string path = g_dglo.m_dmodGamePathWithDir;

	if (path.empty())
	{
		//must be dink and not a DMOD, special case
		path = GetSavePath()+"/dink/";
	}

	//fix outputfilename if it's wrong
	StripWhiteSpace(outputFileName);
	int index = outputFileName.find_first_of('(');
	if (index != string::npos)
	{
		//it probably looks like "save2 (2).dat" due to chrome renaming if it existed, fix it
		outputFileName = outputFileName.substr(0, index - 1) + "."+GetFileExtension(outputFileName);
	}
	LogMsg("Copying %s to %s", fileName.c_str(), (path + outputFileName).c_str());

	if (!GetFileManager()->Copy(fileName, path + outputFileName, false))
	{

		PopUpCreate(pMenu, ("Error copying " + outputFileName + " into "+ g_dglo.m_dmodGamePathWithDir + outputFileName+"!").c_str(), "", "cancel", "Continue", "", "", true);

		return;
	}
	
	PopUpCreate(pMenu, "Ok, we've put " + outputFileName +" into the active game directory. You can now load this save slot like normal.", "", "cancel", "Continue", "", "", true);

}


void App::OnMessage( Message &m )
{
	m_adManager.OnMessage(m); //gives the AdManager a way to handle messages
	
	
	if (m.GetClass() == MESSAGE_CLASS_GUI)
	{
		if (m.GetType() == MESSAGE_TYPE_HTML5_GOT_UPLOAD)
		{
			string fName = m.GetStringParm();
			string physicalFname = "proton_temp.tmp";
			int fileSize = GetFileSize(physicalFname);
			LogMsg("Got uploaded file %s (as %s). %d bytes", m.GetStringParm().c_str(), physicalFname.c_str(),
				fileSize);
			

			if (fileSize > 1024 * 1000)
			{
				//well, it's big, let's assume it's a full save state
				ImportSaveState(physicalFname);
			}
			else
			{
				ImportNormalSaveSlot(physicalFname, fName);

			}


		}
	}

	BaseApp::OnMessage(m);
}


void App::OnLoadSurfaces()
{
	LogMsg("Reloading dink engine surfaces");
	DinkOnForeground();

}

void App::OnUnloadSurfaces()
{
	LogMsg("Unloading dink engine surfaces");
	DinkUnloadUnusedGraphicsByUsageTime(0);
	
	//g_transitionSurf.Kill();
}

void App::AddTextToLog(const char *tex, const char *filename)
	{
		if (strlen(tex) < 1) return;

		if (m_logFileHandle == NULL)
		{

			//open 'er up
			m_logFileHandle = fopen(filename, "wb");
			if (!m_logFileHandle)
			{
				assert(!"huh?");
			}
			return;
		}
		
		if (!m_logFileHandle) return;
			fwrite(tex, strlen(tex), 1, m_logFileHandle);
	
	}

#ifdef WINAPI
//our custom LogMsg that isn't slow as shit
void LogMsg(const char* traceStr, ...)
{
	va_list argsVA;
	const int logSize = 1024 * 10;
	char buffer[logSize];
	memset((void*)buffer, 0, logSize);

	va_start(argsVA, traceStr);
	vsnprintf_s(buffer, logSize, logSize, traceStr, argsVA);
	va_end(argsVA);


	OutputDebugString(buffer);
	OutputDebugString("\n");

	if (IsBaseAppInitted())
	{
		GetBaseApp()->GetConsole()->AddLine(buffer);
		strcat(buffer, "\r\n");
		//OutputDebugString( (string("writing to ")+GetSavePath()+"log.txt\n").c_str());
		//this is the slow part.  Or was...
		GetApp()->AddTextToLog(buffer, (GetSavePath() + "log.txt").c_str());
	}

}

#endif

#ifdef PLATFORM_OSX


//our custom LogMsg that isn't slow as shit
void LogMsg(const char* traceStr, ...)
{
    va_list argsVA;
    const int logSize = 1024 * 10;
    char buffer[logSize];
    memset((void*)buffer, 0, logSize);
    
    va_start(argsVA, traceStr);
    vsnprintf(buffer, logSize, traceStr, argsVA);
    va_end(argsVA);
    
    
    if (IsBaseAppInitted())
    {
        GetBaseApp()->GetConsole()->AddLine(buffer);
        strcat(buffer, "\r\n");
        //OutputDebugString( (string("writing to ")+GetSavePath()+"log.txt\n").c_str());
        //this is the slow part.  Or was...
        GetApp()->AddTextToLog(buffer, (GetSavePath() + "log.txt").c_str());
    }
    
}


#endif


bool TouchesHaveBeenReceived()
{

#ifdef _DEBUG
	//return true;
#endif

#ifdef PLATFORM_HTML5
	if (GetTouchesReceived() > 0) return true;
#endif
	return false;
}
