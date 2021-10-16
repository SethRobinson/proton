#include "PlatformPrecomp.h"
#include "OptionsMenu.h"
#include "Entity/EntityUtils.h"
#include "../App.h"
#include "../dink/Dink.h"
#include "Entity/SliderComponent.h"
#include "MainMenu.h"
#include "PauseMenu.h"
#include "GameMenu.h"
#include "Gamepad/GamepadManager.h"
#include "PopUpMenu.h"

#ifdef PLATFORM_IOS
#include "Gamepad/GamepadProvider60Beat.h"
#endif

#ifdef WINAPI
extern bool g_bUseBorderlessFullscreenOnWindows;
extern bool g_bIsFullScreen;
#endif
#include "Gamepad/GamepadProvideriCade.h"

void UpdateOptionsGUI();

void OptionsMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = pEntClicked->GetParent();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	if (pEntClicked->GetName() == "music_0")
	{
		GetApp()->GetShared()->GetVar("musicDisabled")->Set(uint32(1));
		GetAudioManager()->SetMusicEnabled(0);
	}

	if (pEntClicked->GetName() == "music_1")
	{
		GetApp()->GetShared()->GetVar("musicDisabled")->Set(uint32(0));
		GetAudioManager()->SetMusicEnabled(1);
	}

	if (pEntClicked->GetName() == "toggle_fullscreen")
	{
		GetBaseApp()->OnFullscreenToggleRequest();

		//if you wanted to set a specific size instead:
		//GetBaseApp()->SetVideoMode(200, 200, false);
	}
	if (pEntClicked->GetName() == "vid_small")
	{
		GetBaseApp()->SetVideoMode(640, 480, false);
	}
	
	if (pEntClicked->GetName() == "vid_med")
	{
		GetBaseApp()->SetVideoMode(1024, 768, false);
	}

	if (pEntClicked->GetName() == "vid_big")
	{
		GetBaseApp()->SetVideoMode(1280, 960, false);
	}

	if (pEntClicked->GetName() == "vid_hd")
	{
		GetBaseApp()->SetVideoMode(1920, 1080, false);
	}
	

	if (pEntClicked->GetName() == "controls_0")
	{
		GetApp()->GetShared()->GetVar("controlStyle")->Set(uint32(CONTROLS_JOYPAD));
	}

	if (pEntClicked->GetName() == "controls_1")
	{
		GetApp()->GetShared()->GetVar("controlStyle")->Set(uint32(CONTROLS_DRAG_ANYWHERE));
	}

	if (pEntClicked->GetName() == "controls_2")
	{
		GetApp()->GetShared()->GetVar("controlStyle")->Set(uint32(CONTROLS_FLING));
	}


	if (pEntClicked->GetName() == "buttons_0")
	{
		GetApp()->GetShared()->GetVar("buttons")->Set(uint32(0));
	}

	if (pEntClicked->GetName() == "buttons_1")
	{
		GetApp()->GetShared()->GetVar("buttons")->Set(uint32(1));
	}

	if (pEntClicked->GetName() == "fps_limit_0")
	{
		GetApp()->GetShared()->GetVar("fpsLimit")->Set(uint32(VIDEO_FPS_LIMIT_ON));
		GetApp()->UpdateVideoSettings();
	}
	if (pEntClicked->GetName() == "fps_limit_1")
	{
		GetApp()->GetShared()->GetVar("fpsLimit")->Set(uint32(VIDEO_FPS_LIMIT_OFF));
		GetApp()->UpdateVideoSettings();
	}


	if (pEntClicked->GetName() == "smoothing_0")
	{
		GetApp()->GetShared()->GetVar("smoothing")->Set(uint32(1));
		GetApp()->UpdateVideoSettings();
		DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
		DinkReInitSurfacesAfterVideoChange();
		DinkOnForeground();
	}

	if (pEntClicked->GetName() == "smoothing_1")
	{
		GetApp()->GetShared()->GetVar("smoothing")->Set(uint32(0));
		GetApp()->UpdateVideoSettings();
		DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
		DinkReInitSurfacesAfterVideoChange();
		DinkOnForeground();

	}

	if (pEntClicked->GetName() == "check_stretch")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetVar("check_stretch")->Set(uint32(bChecked));
		GetApp()->UpdateVideoSettings();
		DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
		DinkReInitSurfacesAfterVideoChange();
		DinkOnForeground();
	}

	if (pEntClicked->GetName() == "check_checkboard")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetVar("checkerboard_fix")->Set(uint32(bChecked));
		GetApp()->UpdateVideoSettings();
		DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
		DinkReInitSurfacesAfterVideoChange();
		DinkOnForeground();
	}

	//if (GetEmulatedPlatformID() == PLATFORM_ID_ANDROID)
	{
		if (pEntClicked->GetName() == "allow_glread")
		{
			bool bChecked = IsCheckboxChecked(pEntClicked);
			GetApp()->GetVar("disable_glread")->Set(uint32(!bChecked));
		}
	}
	


#ifdef WINAPI
	if (pEntClicked->GetName() == "check_borderless")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetVar("borderless_fullscreen")->Set(uint32(bChecked));
		
		if (g_bIsFullScreen)
		{
			if (!g_bUseBorderlessFullscreenOnWindows)
			{
				g_bUseBorderlessFullscreenOnWindows = bChecked;
				ChangeDisplaySettings(NULL, 0);
				//GetBaseApp()->SetVideoMode(640, 480, false);
				g_bIsFullScreen = false;
				GetBaseApp()->OnFullscreenToggleRequest();
			}
			else
			{
				//we're currently borderless.. how to we remove that?
				g_bUseBorderlessFullscreenOnWindows = bChecked;
				GetBaseApp()->SetVideoMode(640, 480, true);

			}

		}
		else
		{

		}
		

		g_bUseBorderlessFullscreenOnWindows = bChecked;
		
		/*
		GetApp()->UpdateVideoSettings();
		DinkUnloadUnusedGraphicsByUsageTime(0); //unload anything not used in the last second
		DinkReInitSurfacesAfterVideoChange();
		DinkOnForeground();
		*/
	}

#endif

	if (pEntClicked->GetName() == "sbeat_ad")
	{
		string url = "http://www.60beat.com/?Click=326";
		PopUpCreate(pEntClicked->GetParent()->GetParent()->GetParent(), "Would you like to visit 60beat's webpage and learn more about their gamepad?", url,
			"cancel", "`wCancel", "url", "`wLaunch", true);
		return;
	}

	if (pEntClicked->GetName() == "check_icade")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetVar("check_icade")->Set(uint32(bChecked));

		GetGamepadManager()->RemoveProviderByName("iCade");
		GetApp()->RemoveAndroidKeyboardKeys();

		if (bChecked)
		{
			GetApp()->AddIcadeProvider();
			GetApp()->RemoveAndAttachAllAvailableGamepads();
		} else
		{
			GetApp()->AddDroidKeyboardKeys();
		    //GetBaseApp()->SetAllowScreenDimming(true);
		}
	}

#ifdef PLATFORM_IOS
	
    /*
    if (pEntClicked->GetName() == "check_60beat")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetVar("check_60beat")->Set(uint32(bChecked));

		GetGamepadManager()->RemoveProviderByName("60Beat");

		if (bChecked)
		{
			GetGamepadManager()->AddProvider(new GamepadProvider60Beat);
			GetBaseApp()->SetAllowScreenDimming(false);
			GetApp()->RemoveAndAttachAllAvailableGamepads();
		} else
		{
			//GetBaseApp()->SetAllowScreenDimming(true);
		}
	}
     */
#endif

	if (pEntClicked->GetName() == "Back")
	{
		RemoveFocusIfNeeded(pMenu);
		ZoomToPositionEntity(pMenu, CL_Vec2f(GetScreenSizeXf(),0), 500); //slide up
		KillEntity(pMenu, 500);
		AddFocusIfNeeded(pMenu->GetParent(), true, 500);
		GetApp()->SaveSettings();
		SyncPersistentData();
	}

	if (pEntClicked->GetName() == "sound_1")
	{
		GetApp()->GetShared()->GetVar("sound")->Set(uint32(1));
		GetAudioManager()->SetSoundEnabled(true);

		//restart music if applicable
		bool bMusicDisabled = GetApp()->GetShared()->GetVar("musicDisabled")->GetUINT32() != 0;

		if (GetDinkGameState() == DINK_GAME_STATE_PLAYING)
		{
		} else
		{
			PlayMenuMusic();
		}

		UpdateOptionsGUI();

		return;
	}
	if (pEntClicked->GetName() == "sound_0")
	{
		GetApp()->GetShared()->GetVar("sound")->Set(uint32(0));
		GetAudioManager()->SetSoundEnabled(false);
		GetAudioManager()->StopMusic();
		UpdateOptionsGUI();
		return;
	}

	if (pEntClicked->GetName() == "FPS")
	{
		GetBaseApp()->SetFPSVisible(!GetBaseApp()->GetFPSVisible());
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


void OnProgressChangedMusic(Variant *pDataObject)
{
	float musicVol = pDataObject->GetFloat();
	GetApp()->GetVar("music_vol")->Set(musicVol);
	//LogMsg("Music vol changed to %.2f", musicVol);
	GetAudioManager()->SetMusicVol(musicVol);
}

void OnProgressChangedGUI(Variant *pDataObject)
{
	float guiTrans = pDataObject->GetFloat();
	GetApp()->GetVar("gui_transparency")->Set(guiTrans);
	UpdateOptionsGUI();
}


void UpdateOptionsGUI()
{
	float alpha = GetApp()->GetVar("gui_transparency")->GetFloat();
	
	Entity *inventoryIcon = GetEntityRoot()->GetEntityByName("options_inventory");

	if (inventoryIcon)
	{
		inventoryIcon->GetVar("alpha")->Set( alpha);
	}
	
	bool bSound = GetApp()->GetShared()->GetVar("sound")->GetUINT32() != 0;

	if (bSound)
	{
		//enable the music slider
	}

}

void OptionsMenuAddScrollContent(Entity *pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities();

	Entity *pBG = pParent;

	Entity *pEnt;
	float y = iPhoneMapY(70);
	float startX = iPhoneMapX(28);
	float offsetX = iPhoneMapX(0);
	float spacerX = iPhoneMapX(46);
	float spacerY = iPhoneMapY(27.5);
	float columnX = 140;

	eFont fontID = FONT_SMALL;

	//title at the top
	pEnt = CreateTextLabelEntity(pBG, "title", GetScreenSizeXf()/2, iPhoneMapY(40), "Options");
	SetupTextEntity(pEnt, FONT_LARGE);
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);

	if (IsDesktop())
	{
		pEnt = CreateTextButtonEntity(pBG, "toggle_fullscreen", startX, y, "Toggle fullscreen (or Alt-Enter)");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		y += spacerY;

		pEnt = CreateTextButtonEntity(pBG, "vid_small", startX, y, "640X480");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		
		pEnt = CreateTextButtonEntity(pBG, "vid_med", startX+350, y, "1024X768");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	
		pEnt = CreateTextButtonEntity(pBG, "vid_big", startX+700, y, "1280X960");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		y += spacerY;
		pEnt = CreateTextButtonEntity(pBG, "vid_hd", startX, y, "1920X1080");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		y += spacerY;
	}

	//control method
	
	if (GetApp()->GetUsingTouchScreen())
	{

	pEnt = CreateTextLabelEntity(pBG, "", startX, y, "Controls:");
	SetupTextEntity(pEnt,fontID);
	offsetX =  iPhoneMapX(columnX);
	pEnt = CreateTextButtonEntity(pBG, "controls_0", offsetX, y, "Virtual Joypad", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	
	float smallSpacerX = spacerX;
	if (IsIPADSize)
	{
		smallSpacerX = 34;
	}
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + smallSpacerX;

	pEnt = CreateTextButtonEntity(pBG, "controls_1", offsetX, y, "Drag Anywhere", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + smallSpacerX;

	/*
	if (IsIPADSize)
	{
		pEnt = CreateTextButtonEntity(pBG, "controls_2", offsetX, y, "Fling Mode", false);
		SetupTextEntity(pEnt,fontID);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
		offsetX += pEnt->GetVar("size2d")->GetVector2().x + smallSpacerX;
	}

	*/

	uint32 controlsID = GetApp()->GetVar("controlStyle")->GetUINT32();
	SetupLightBarSelect(pBG, "controls_", controlsID, MAKE_RGBA(190, 0, 35, 255));

	y += spacerY;
	y += spacerY;


	/*
	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
	{
		bool bUse60Beat = GetApp()->GetVar("check_60beat")->GetUINT32() != 0;
		pEnt = CreateCheckbox(pBG, "check_60beat", "Use 60beat� GamePad", startX, y, bUse60Beat, FONT_SMALL, 1.0f);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);

		//add the image to the right
		CL_Sizef vCheckBoxSizeWithText = MeasureEntityAndChildren(pEnt).get_size();
		offsetX = startX + vCheckBoxSizeWithText.width+iPhoneMapX(20);

		Entity *pAd = CreateOverlayButtonEntity(pBG, "sbeat_ad", "interface/sixtybeat_ad.rttex", offsetX, y+vCheckBoxSizeWithText.height/2);
		pAd->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);

		SetAlignmentEntity(pAd, ALIGNMENT_LEFT_CENTER);
		//We don't want it too big, scale it down if needed
		EntityScaleiPad(pAd, true);
		y += GetSize2DEntity(pAd).y;
		//y += spacerY;
	}

	*/

	
	//********* icade option

	string bName = "iCade Controller Mode";

	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
	{
		//bName = "Bluetooth Arcade Stick Mode";
	}

	bool bUseicade = GetApp()->GetVar("check_icade")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_icade", "Use "+bName, startX, y, bUseicade, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;


	pEnt = CreateTextLabelEntity(pBG, "", startX, y, "GUI Icons:");
	SetupTextEntity(pEnt,fontID);
	offsetX =  iPhoneMapX(columnX);
	pEnt = CreateTextButtonEntity(pBG, "buttons_0", offsetX, y, "Right side", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

	pEnt = CreateTextButtonEntity(pBG, "buttons_1", offsetX, y, "Left side", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

	uint32 buttons = GetApp()->GetVar("buttons")->GetUINT32();
	SetupLightBarSelect(pBG, "buttons_", buttons, MAKE_RGBA(190, 0, 35, 255));
	y += spacerY;
	}

	
	bool bStretchToFit = GetApp()->GetVar("check_stretch")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_stretch", "Force screen stretching (ignore aspect ratio)", startX, y, bStretchToFit, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

#ifdef WINAPI
	bool bBorderlessFullscreen = GetApp()->GetVar("borderless_fullscreen")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_borderless", "Use Windowed borderless fullscreen mode", startX, y, bBorderlessFullscreen, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;
#endif

	bool bCheckerboardFix = GetApp()->GetVar("checkerboard_fix")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_checkboard", "Apply improved shadows", startX, y, bCheckerboardFix, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

	bool bDisableRead = GetApp()->GetVar("disable_glread")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "allow_glread", "Enable screen scroll effect", startX, y, !bDisableRead, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;


	/*
	//fps limit
	pEnt = CreateTextLabelEntity(pBG, "", startX, y, "Lock to 30 FPS:");
	SetupTextEntity(pEnt,fontID);
	offsetX =  iPhoneMapX(columnX);
	pEnt = CreateTextButtonEntity(pBG, "fps_limit_0", offsetX, y, "On", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

	pEnt = CreateTextButtonEntity(pBG, "fps_limit_1", offsetX, y, "Off", false);
	SetupTextEntity(pEnt,fontID);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
	offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

	uint32 videoFPS = GetApp()->GetVar("fpsLimit")->GetUINT32();
	SetupLightBarSelect(pBG, "fps_limit_", videoFPS, MAKE_RGBA(190, 0, 35, 255));
	*/

	if (GetPlatformID() != PLATFORM_ID_IOS)
	{
		//y += spacerY;
		//audio on/off button

		pEnt = CreateTextLabelEntity(pBG, "", startX, y, "Audio:");
		SetupTextEntity(pEnt,fontID);
		offsetX =  iPhoneMapX(columnX);
		pEnt = CreateTextButtonEntity(pBG, "sound_1", offsetX, y, "On", false);
		SetupTextEntity(pEnt,fontID);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

		pEnt = CreateTextButtonEntity(pBG, "sound_0", offsetX, y, "Off", false);
		SetupTextEntity(pEnt,fontID);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
		offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

		bool bSound = GetApp()->GetShared()->GetVar("sound")->GetUINT32() != 0;
		SetupLightBarSelect(pBG, "sound_", bSound, MAKE_RGBA(190, 0, 35, 255));
	}
	
	//music vol slider
	y += spacerY+iPhoneMapY2X(26);
	EntityComponent *pSliderComp = CreateSlider(pBG, startX, y, iPhoneMapX(360), "interface/slider_button.rttex", "Min", "Music volume", "Max");
	pSliderComp->GetVar("progress")->Set( GetApp()->GetVar("music_vol")->GetFloat());
	pSliderComp->GetVar("progress")->GetSigOnChanged()->connect(&OnProgressChangedMusic);

	//transparency slider
	if (GetApp()->GetUsingTouchScreen())
	{
		y += spacerY+iPhoneMapY2X(36);
		pSliderComp = CreateSlider(pBG, startX, y, iPhoneMapX(360), "interface/slider_button.rttex", "Min", "Game Interface Visibility", "Max");
		
		//a thing to visually show them how much alpha they've set it too
		Entity *pChest = CreateOverlayEntity(pBG, "options_inventory", ReplaceWithLargeInFileName("interface/iphone/button_inventory.rttex"),  startX+iPhoneMapX(363), y-iPhoneMapY(34));
		SetAlignmentEntity(pChest, ALIGNMENT_UPPER_LEFT);
		UpdateOptionsGUI();

		pSliderComp->GetVar("progress")->Set( GetApp()->GetVar("gui_transparency")->GetFloat());
		pSliderComp->GetVar("progress")->GetSigOnChanged()->connect(&OnProgressChangedGUI);
		y += spacerY;
	} else
	{
		y += spacerY;

	}
	
//if (GetPlatformID() != PLATFORM_ID_IOS)
{
		//smoothing
		pEnt = CreateTextLabelEntity(pBG, "", startX, y, "Pic smoothing:");
		SetupTextEntity(pEnt,fontID);
		offsetX =  iPhoneMapX(columnX);
		pEnt = CreateTextButtonEntity(pBG, "smoothing_0", offsetX, y, "On", false);
		SetupTextEntity(pEnt,fontID);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
		offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

		pEnt = CreateTextButtonEntity(pBG, "smoothing_1", offsetX, y, "Off", false);
		SetupTextEntity(pEnt,fontID);
		pEnt->GetFunction("OnButtonSelected")->sig_function.connect(OptionsMenuOnSelect);
		offsetX += pEnt->GetVar("size2d")->GetVector2().x + spacerX;

		uint32 smoothing = !GetApp()->GetVar("smoothing")->GetUINT32();
		SetupLightBarSelect(pBG, "smoothing_", smoothing, MAKE_RGBA(190, 0, 35, 255));

		//show fps
		y += spacerY;
		pEnt = CreateTextButtonEntity(pBG, "FPS",startX, y, "Toggle FPS Display");
		pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
}
	
    VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
}

Entity * OptionsMenuCreate( Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayEntity(pParentEnt, "OptionsMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	AddFocusIfNeeded(pBG, true, 500);

//add the header

	CL_Vec2f vTextAreaPos = iPhoneMap(2,10);
	float offsetFromBottom = iPhoneMapY(42);
	float offsetFromRight = iPhoneMapY(0);

	CL_Vec2f vTextAreaBounds = (GetScreenSize()- CL_Vec2f(offsetFromRight,offsetFromBottom))-vTextAreaPos;
	Entity *pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	EntityComponent *pFilter = pScroll->AddComponent(new FilterInputComponent);
	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	
	/*
	//too slow/broken on Android, we'll do it another way
	EntityComponent *pClip = pScroll->AddComponent(new RenderClipComponent);
	pClip->GetVar("clipMode")->Set(uint32(RenderClipComponent::CLIP_MODE_BOTTOM));
	*/

	Entity *pOverlay = CreateOverlayEntity(pBG, "", ReplaceWithDeviceNameInFileName("interface/iphone/bg_stone_overlay.rttex"), 0, GetScreenSizeYf()); 
	SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);
	
	OptionsMenuAddScrollContent(pBG);
	//	ZoomFromPositionEntity(pBG, CL_Vec2f(0, -GetScreenSizeYf()), 500);
	//the continue button
	Entity *pEnt;

	//pEnt = CreateOverlayRectEntity(pBG, CL_Rectf(0, GetScreenSizeYf()-offsetFromBottom, GetScreenSizeXf(), 320), MAKE_RGBA(0,0,0,100));

	eFont fontID = FONT_SMALL;

	pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(5), iPhoneMapY(BACK_BUTTON_Y), "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&OptionsMenuOnSelect);
	SetupTextEntity(pEnt, fontID);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);

	ZoomFromPositionEntity(pBG, CL_Vec2f( GetScreenSizeXf(),0), 500);

	return pBG;
}

