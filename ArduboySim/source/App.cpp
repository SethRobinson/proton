/*
 *  App.cpp
 *  Created by Seth Robinson
 *
 */
#include "PlatformPrecomp.h"
#include "App.h"

#include "Entity/CustomInputComponent.h" //used for the back button (android)
#include "Entity/FocusInputComponent.h" //needed to let the input component see input messages
#include "Entity/ArcadeInputComponent.h" 
#include "Entity/EntityUtils.h"
#include "Renderer/SoftSurface.h"


//uncomment below if you want the device turned sideways
//#define RT_ARDU_LANDSCAPE


#ifdef RT_USE_SDL_AUDIO
#include "Audio/AudioManagerSDL.h"
AudioManagerSDL g_audioManager; //to disable sound, this is a dummy

#else
//no audio
#include "Audio/AudioManager.h"
AudioManager g_audioManager; //to disable sound, this is a dummy
#endif

#include "Arduboy.h"
#include "Entity/TouchHandlerArcadeComponent.h"
#include "Entity/EmitVirtualKeyComponentAdvanced.h"

const float C_ARDUBOY_SCALE_MULT= 1.0f; //1 = normal (which is actually 2x scale), 2 for double, 0.5 for half (which is 1.x scale)
int C_ARDUBOY_SIDE_PADDING=0; //0 for none, otherwise it adds black strips on the left and right side
void main_setup();
void main_loop();

extern Arduboy arduboy;

MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}



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
	assert(g_pApp && "GetBaseApp must be called used first");
	return g_pApp;
}



App::App()
{
	m_bDidPostInit = false;
	m_bWantToResetArduboy = false;
}

App::~App()
{
}

bool App::Init()
{
	
	if (m_bInitted)	
	{
		return true;
	}
	
	if (!BaseApp::Init()) return false;
	
	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS || GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
	{
		//SetLockedLandscape( true); //if we don't allow portrait mode for this game
		//SetManualRotationMode(true); //don't use manual, it may be faster (33% on a 3GS) but we want iOS's smooth rotations
	}

	LogMsg("The Save path is %s", GetSavePath().c_str());
	LogMsg("Region string is %s", GetRegionString().c_str());

#ifdef _DEBUG
	LogMsg("Built in debug mode");
#endif
#ifndef C_NO_ZLIB
	//fonts need zlib to decompress.  When porting a new platform I define C_NO_ZLIB and add zlib support later sometimes
	if (!GetFont(FONT_SMALL)->Load("interface/font_trajan.rtfont")) return false;
#endif
	//SetDefaultButtonStyle(Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);


	
	//GetBaseApp()->SetFPSVisible(true); //can show FPS, but it's not the arduboy fps, it's our own update
	GetBaseApp()->SetFPSLimit(0); //let arduboy code do the limiting


	//scale screen up?



	SetManualRotationMode(true);
	SetForcedOrientation(ORIENTATION_DONT_CARE);
	SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);

	#ifdef RT_ARDU_LANDSCAPE
	SetupScreenInfo(GetPrimaryGLY(), GetPrimaryGLX(), ORIENTATION_LANDSCAPE_RIGHT); //it's actually left, I don't know why this is reversed
	#endif

	if (C_ARDUBOY_SCALE_MULT != 1)
	{
		int scaleToX = (C_ARDUBOY_SIDE_PADDING*2)+414;
		int scaleToY = 661;

		SetupFakePrimaryScreenSize(scaleToX,scaleToY); //game will think it's this size, and will be scaled up
	}

	return true;
}

void App::Kill()
{
	arduboy.end();
	BaseApp::Kill();
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
	
	exit(0); //shouldn't do this, but makes Escape work to exit on the Arduboy simulator
}

#define kFilteringFactor 0.1f
#define C_DELAY_BETWEEN_SHAKES_MS 500


void App::OnArcadeInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	eVirtualKeyInfo keyInfo = (eVirtualKeyInfo) pVList->Get(1).GetUINT32();
	
	string pressed;

	bool bDown = false;

	switch (keyInfo)
	{
		case VIRTUAL_KEY_PRESS:
			pressed = "pressed";
			bDown = true;
			break;

		case VIRTUAL_KEY_RELEASE:
			pressed = "released";
			break;

		default:
			LogMsg("OnArcadeInput> Bad value of %d", keyInfo);
	}

	

	string keyName = "unknown";

	switch (vKey)
	{
	case VIRTUAL_KEY_GAME_INVENTORY:
		{
			if (bDown)
			{
				LogMsg("You want to reset the Arduboy?!");
				m_bWantToResetArduboy = true;
				
				return;
			}
			break;
		}
		case VIRTUAL_KEY_DIR_LEFT:
			g_gameKeys.buttonStatus[GAME_KEY_LEFT] = bDown;
			keyName = "Left";
			break;

		case VIRTUAL_KEY_DIR_RIGHT:
			g_gameKeys.buttonStatus[GAME_KEY_RIGHT] = bDown;
			keyName = "Right";
			break;

		case VIRTUAL_KEY_DIR_UP:
			g_gameKeys.buttonStatus[GAME_KEY_UP] = bDown;
			keyName = "Up";
			break;

		case VIRTUAL_KEY_DIR_DOWN:
			g_gameKeys.buttonStatus[GAME_KEY_DOWN] = bDown;
			keyName = "Down";
			break;
		
		case VIRTUAL_KEY_GAME_FIRE:
			g_gameKeys.buttonStatus[GAME_KEY_A] = bDown;
			keyName = "ButtonA";
			break;
		case VIRTUAL_KEY_GAME_TALK:
			g_gameKeys.buttonStatus[GAME_KEY_B] = bDown;
			keyName = "ButtonB";
			break;

	}
	
	
	//LogMsg("Arcade input: Hit %d (%s) (%s)", vKey, keyName.c_str(), pressed.c_str());
}


void AppInput(VariantList *pVList)
{

	//0 = message type, 1 = parent coordinate offset, 2 is fingerID
	eMessageType msgType = eMessageType( int(pVList->Get(0).GetFloat()));
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	//pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	
	uint32 fingerID = 0;
	if ( msgType != MESSAGE_TYPE_GUI_CHAR && pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	CL_Vec2f vLastTouchPt = GetBaseApp()->GetTouch(fingerID)->GetLastPos();

	switch (msgType)
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		//LogMsg("Touch start: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		//LogMsg("Touch mode: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_END:
		//LogMsg("Touch end: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	}	

}

EntityComponent * MakeButtonEmitVirtualGameKeyAdvanced(Entity *pEnt, uint32 keycode)
{
	if (!pEnt)
	{
		assert(!"Serious error");
		return NULL;
	}
	EntityComponent *pComp = pEnt->AddComponent(new EmitVirtualKeyComponentAdvanced);
	pComp->GetVar("keycode")->Set(keycode);
	return pComp;
}

void SetupButtonOverlay(Entity *pParent, CL_Vec2f vPos, CL_Vec2f vBounds, string name, uint32 virtualKey)
{
	//add some overlay buttons
	vPos += CL_Vec2f(C_ARDUBOY_SIDE_PADDING,0);
	uint32 color = MAKE_RGBA(255,255,255,0);
	//Entity * pButton = CreateButtonHotspot(pParent, name, vPos, vBounds, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	Entity *pButton = CreateOverlayRectEntity(pParent, vPos, vBounds, color);
	//Entity *pButton = CreateOverlayEntity(pParent, "Name", "interface/test.rttex", vPos.x, vPos.y);
	pButton->AddComponent(new TouchHandlerArcadeComponent);
	pButton->GetVar("touchPadding")->Set(CL_Rectf(0,0,0,0));
	//pButton->GetVar("color")->Set(color);
	MakeButtonEmitVirtualGameKeyAdvanced(pButton, virtualKey);
	pParent->MoveEntityToTopByAddress(pButton);
}

void App::Update()
{
	//game can think here.  The baseApp::Update() will run Update() on all entities, if any are added.  The only one
	//we use in this example is one that is watching for the Back (android) or Escape key to quit that we setup earlier.

	BaseApp::Update();

	if (!m_bDidPostInit)
	{
		//stuff I want loaded during the first "Update"
		m_bDidPostInit = true;
		
		//for android, so the back key (or escape on windows) will quit out of the game
		Entity *pEnt = GetEntityRoot()->AddEntity(new Entity("Gamelogic"));
		//give focus to the entire entity tree
		AddFocusIfNeeded(GetApp()->GetEntityRoot());
	
		EntityComponent *pComp = pEnt->AddComponent(new CustomInputComponent);
		//tell the component which key has to be hit for it to be activated
		pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));
		//attach our function so it is called when the back key is hit
		pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, this, _1));

		
		pComp = pEnt->AddComponent(new ArcadeInputComponent);
		

		GetBaseApp()->m_sig_arcade_input.connect(1, boost::bind(&App::OnArcadeInput, this, _1));
	
		//these arrow keys will be triggered by the keyboard, if applicable
		
		AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
		AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
		AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
		AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
		AddKeyBinding(pComp, "Fire", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);
		AddKeyBinding(pComp, "FireB", 'Z', VIRTUAL_KEY_GAME_TALK);
	
		//INPUT TEST - wire up input to some functions to manually handle.  AppInput will use LogMsg to
		//send them to the log.  (Each device has a way to view a debug log in real-time)
		GetBaseApp()->m_sig_input.connect(&AppInput);

		//proton fonts

		/*
		if (!g_big_font.Load("interface/font_century_gothic_bigx2.rtfont", false)) 
		{
			LogMsg("Can't load font 2");
		}

		if (!g_font.Load("interface/font_century_gothic.rtfont", false)) 
		{
			LogMsg("Can't load font 1");
		}
		*/
	
		CreateOverlayEntity(pEnt, "background", "interface/arduboy2x.rttex", C_ARDUBOY_SIDE_PADDING,0);

		//arduboy hotspots on the above image

		SetupButtonOverlay(pEnt, CL_Vec2f(82,319), CL_Vec2f(61,55), "up",VIRTUAL_KEY_DIR_UP);
		SetupButtonOverlay(pEnt, CL_Vec2f(82,437), CL_Vec2f(59,54), "down",VIRTUAL_KEY_DIR_DOWN);
		SetupButtonOverlay(pEnt, CL_Vec2f(29,379), CL_Vec2f(48,59), "left",VIRTUAL_KEY_DIR_LEFT);
		SetupButtonOverlay(pEnt, CL_Vec2f(151,372), CL_Vec2f(49,64), "right",VIRTUAL_KEY_DIR_RIGHT);

		SetupButtonOverlay(pEnt, CL_Vec2f(247,402), CL_Vec2f(59,59), "buttona",VIRTUAL_KEY_GAME_FIRE);
		SetupButtonOverlay(pEnt, CL_Vec2f(320,371), CL_Vec2f(59,59), "buttonb",VIRTUAL_KEY_GAME_TALK);

		
		//ucomment for and clicking the USB connector will reset the arduboy.  But it only works between main_loop calls and won't reset the sketch
		//variables so it's pretty useless unless you wrote your sketch code to allow this and reset your vars in the main_setup() part.
		//SetupButtonOverlay(pEnt, CL_Vec2f(178,610), CL_Vec2f(59,34), "reset", VIRTUAL_KEY_GAME_INVENTORY);

		//oh, let's pop up a message telling them about the hardware keys

		Entity *pTextEnt = CreateTextLabelEntity(pEnt, "Help", 15+C_ARDUBOY_SIDE_PADDING, 277, "`bCan use Ctrl, Z, & Arrow keys too!");
		SetAlphaEntity(pTextEnt, 0.0f);
		FadeInEntity(pTextEnt, 800,2000);
		FadeOutAndKillEntity(pTextEnt, false, 1000, 7000);

		main_setup(); //hook into our arduboy code

	}

	if (m_bWantToResetArduboy)
	{
		m_bWantToResetArduboy = false;
		//uhh... how do we reset the arduboy though?
		main_setup();
	}
	main_loop();

#ifdef RT_ARDU_LANDSCAPE
	static int timer = GetTick()+500;

	if (timer != 0 && timer < GetTick())
	{
	}
#endif
}


void App::Draw()
{

	//Use this to prepare for raw GL calls
	PrepareForGL();
#ifdef _DEBUG
	//LogMsg("Doing draw");
#endif
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CLEAR_GL_ERRORS() //honestly I don't know why I get a 0x0502 GL error when doing the FIRST gl action that requires a context with emscripten only
	//the base handles actually drawing the GUI stuff over everything else, if applicable, which in this case it isn't.
	BaseApp::Draw();

	//blit arduboys visual buffer onto the screen
	arduboy.GLBlit();

}


void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

void App::OnEnterBackground()
{
	//save your game stuff here, as on some devices (Android <cough>) we never get another notification of quitting.
	LogMsg("Entered background");
	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	LogMsg("Entered foreground");
	BaseApp::OnEnterForeground();
}

const char * GetAppName() {return "Proton Arduboy Simulator";}

//the stuff below is for android/webos builds.  Your app needs to be named like this.

//note: these are put into vars like this to be compatible with my command-line parsing stuff that grabs the vars

const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

const char * GetBundleName()
{
	const char * bundleName = "arduboysim";
	return bundleName;
}
extern int g_winVideoScreenX;
extern int g_winVideoScreenY;

bool App::OnPreInitVideo()
{
	//only called for desktop systems
	//override in App.* if you want to do something here.  You'd have to
	//extern these vars from main.cpp to change them...

	//SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);
	
	g_winVideoScreenX = (414+(C_ARDUBOY_SIDE_PADDING*2))*C_ARDUBOY_SCALE_MULT;
	g_winVideoScreenY = 661*C_ARDUBOY_SCALE_MULT;
	
#ifdef RT_ARDU_LANDSCAPE
swap(g_winVideoScreenX, g_winVideoScreenY);
#endif
	return true; //no error
}

void App::FakeUpdate()
{
	BaseApp::Update();
}
