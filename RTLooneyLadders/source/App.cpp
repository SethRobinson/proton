
#include "PlatformPrecomp.h"
#include "App.h"
#include "GUI/MainMenu.h"
#include "Renderer/LinearParticle.h"
#include "Entity/EntityUtils.h"//create the classes that our globally library expects to exist somewhere.
#include "Renderer/SoftSurface.h"
#include "Entity/ArcadeInputComponent.h" 
#include "GUI/GameMenu.h"
#include "Gamepad/GamepadManager.h"

#include "GUI/IntroMenu.h"

MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

GamepadManager g_gamepadManager;
GamepadManager * GetGamepadManager() {return &g_gamepadManager;}

#ifdef __APPLE__

#if TARGET_OS_IPHONE == 1
	//it's an iPhone or iPad
	#include "Gamepad/GamepadProviderIOS.h"

#include "Audio/AudioManagerOS.h"

AudioManagerOS g_audioManager;
#else
	//it's being compiled as a native OSX app
   #include "Audio/AudioManagerFMOD.h"

   AudioManagerFMOD g_audioManager; //dummy with no sound
#endif
	
#else


#include "Audio/AudioManagerSDL.h"
#include "Audio/AudioManagerAndroid.h"
 
#if defined RT_USE_SDL

//let's assume we're using SDL2 for everything actually, so this will happen in the Windows SDL build as well as linux
#include "Gamepad/GamepadProviderSDL2.h"
#endif

#if defined RT_USE_SDL_AUDIO
AudioManagerSDL g_audioManager; //sound in windows and WebOS


//AudioManager g_audioManager; //to disable sound
#elif defined ANDROID_NDK
//AudioManager g_audioManager; //to disable sound
AudioManagerAndroid g_audioManager; //sound for android
#else
//in windows

#include "Gamepad/GamepadProviderDirectX.h"
#include "Gamepad/GamepadProviderXInput.h"
#include "Audio/AudioManagerAudiere.h"
//#include "Audio/AudioManagerFMOD.h"

AudioManagerAudiere g_audioManager;  //Use Audiere for audio
//AudioManagerFMOD g_audioManager; //if we wanted FMOD sound in windows
//AudioManager g_audioManager; //to disable sound

#endif
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

App::App()
{
	m_bDidPostInit = false;
}

App::~App()
{
	L_ParticleSystem::deinit();
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

bool App::Init()
{
	GetBaseApp()->SetDisableSubPixelBlits(false);
	//SetDefaultAudioClickSound("audio/enter.wav");
	SetDefaultButtonStyle(Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);
	
	L_ParticleSystem::init(2000);

	if (m_bInitted)	
	{
		return true;
	}
	
	if (!BaseApp::Init()) return false;

	LogMsg("Save path is %s", GetSavePath().c_str());

	if (!GetFont(FONT_SMALL)->Load("interface/font_trajan.rtfont")) 
	{
		LogMsg("Can't load font 1");
		return false;
	}
	if (!GetFont(FONT_LARGE)->Load("interface/font_trajan_big.rtfont"))
	{
		LogMsg("Can't load font 2");
		return false;
	}
	//GetFont(FONT_SMALL)->SetSmoothing(false); //if we wanted to disable bilinear filtering on the font

#ifdef _DEBUG
	GetBaseApp()->SetFPSVisible(true);
#endif
	
	bool bFileExisted;
	m_varDB.Load("save.dat", &bFileExisted);
	m_varDB.GetVarWithDefault("level", Variant(uint32(1)));
	
	//Actually, because this isn't a real game, let's just cheat and give the player access to all the levels
	UnlockAllLevels();
	
	//preload audio
	GetAudioManager()->Preload("audio/click.wav");
	

#if defined( PLATFORM_WINDOWS) && !defined(RT_USE_SDL)
	//XInput is newer and works better, doesn't require that the controller already be plugged in at start
	
	GamepadProviderXInput* pTemp = new GamepadProviderXInput(); //this MUST be added first!
	//pTemp->PreallocateControllersEvenIfMissing(true);
	GetGamepadManager()->AddProvider(pTemp); //use XInput joysticks
	

	//do another scan for the older style directx devices, it will ignore any sticks that are already initialized as XInput
	GamepadProviderDirectX* pTempDirectX = new GamepadProviderDirectX;
	pTempDirectX->SetIgnoreXInputCapableDevices(true);
	GetGamepadManager()->AddProvider(pTempDirectX); //use directx joysticks

	GetGamepadManager()->m_sig_gamepad_connected.connect(1, boost::bind(&App::OnGamepadConnected, this, _1));
	GetGamepadManager()->m_sig_gamepad_disconnected.connect(1, boost::bind(&App::OnGamepadDisconnected, this, _1));

#endif

#if defined (RT_USE_SDL)

	//we're going to use SDL for our gamepad support
	GamepadProviderSDL2* pTemp = new GamepadProviderSDL2(); 
	GetGamepadManager()->AddProvider(pTemp); //use XInput joysticks


#endif

	
#if defined(PLATFORM_IOS)
		GetGamepadManager()->AddProvider(new GamepadProviderIOS);
		//GetBaseApp()->SetAllowScreenDimming(false); //I don't think this matters anymore, the controller support will automatically handle
        //that
#endif

	return true;
}

void AttachGamepadsIfPossible()
{
	Entity* pEntWithArcadeComp = GetApp()->GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pEntWithArcadeComp)
	{
		pEntWithArcadeComp = GetApp()->GetEntityRoot()->GetEntityByName("SelectIcon");
	}

	if (pEntWithArcadeComp)
	{
		ArcadeInputComponent* pComp = (ArcadeInputComponent*) pEntWithArcadeComp->GetComponentByName("ArcadeInput");
		
		if (pComp)
		{
			LogMsg("Connecting gamepads...");
			for (int i = 0; i < GetGamepadManager()->GetGamepadCount(); i++)
			{
				Gamepad* pPad = GetGamepadManager()->GetGamepad((eGamepadID)i);
				pPad->ConnectToArcadeComponent(pComp, true, true);

				//if we cared about the analog sticks too, we'd do this:
				//pPad->m_sig_left_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
				//pPad->m_sig_right_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
			}
		}
		else
		{
			LogMsg("Couldn't find ArcadeInput on entity %s", pEntWithArcadeComp->GetName().c_str());
		}
	}
}


void App::OnGamepadConnected(Gamepad* pPad)
{
	AttachGamepadsIfPossible();
	
}

void App::OnGamepadDisconnected(eGamepadID id)
{
	LogMsg("Pad was disconnected");
}

void App::Kill()
{
	m_varDB.Save("save.dat");
	GetGamepadManager()->ClearProviders();
	BaseApp::Kill();
	g_pApp = NULL;
}

void App::Update()
{
	BaseApp::Update();
	g_gamepadManager.Update();

	if (!m_bDidPostInit)
	{
		m_bDidPostInit = true;
	
		//build a dummy entity called "GUI" to put our GUI menu entities under
		Entity *pGUIEnt = GetEntityRoot()->AddEntity(new Entity("GUI"));
	
#ifdef _DEBUG
		IntroMenuCreate(pGUIEnt);
//		MainMenuCreate(pGUIEnt);
//		GameMenuCreate(pGUIEnt);
#else
		IntroMenuCreate(pGUIEnt);
#endif
	}
}

void App::Draw()
{
	PrepareForGL();
//	glClearColor(0.6,0.6,0.6,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BaseApp::Draw();
}

void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

Variant * App::GetVar( const string &keyName )
{
	return GetShared()->GetVar(keyName);
}

std::string App::GetVersionString()
{
	return "V0.7";
}

float App::GetVersion()
{
	return 0.7f;
}

int App::GetBuild()
{
	return 1;
}

const char * GetAppName() {return "Looney Ladders by Seth A. Robinson (rtsoft.com)";}

//for palm webos and android
const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

const char * GetBundleName()
{
	const char * bundleName = "rtlooneyladders";
	return bundleName;
}

bool App::OnPreInitVideo()
{
	if (!BaseApp::OnPreInitVideo()) return false;

	SetupScreenInfo(1024, 768, ORIENTATION_DONT_CARE);

	return true;
}

