#include "PlatformPrecomp.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"
#include "../Component/FPSControlComponent.h"
#include "../Component/CursorComponent.h"
#include "../Component/InventoryComponent.h"
#include "../Component/ActionButtonComponent.h"
#include "DebugMenu.h"
#include "PauseMenu.h"
#include "PopUpMenu.h"
#include "../Component/DragControlComponent.h"
#include "QuickTipMenu.h"
#include "Entity/SelectButtonWithCustomInputComponent.h"
#include "Entity/ArcadeInputComponent.h"
#include "Renderer/SoftSurface.h"

#define AUTO_SAVE_MS (1000*60*5)

void UpdatePauseMenuPosition(Entity *pBG);

void ShowQuickMessage(string msg)
{
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	//assert(pMenu);
	if (!pMenu)
	{
		//	pMenu = GetEntityRoot();
		return;
	}
	Entity *pEnt = CreateTextLabelEntity(pMenu, "GameMsg", GetScreenSizeXf() / 2, iPhoneMapY(100), msg);

	//SetupTextEntity(pEnt, FONT_LARGE);
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);

	FadeInEntity(pEnt);
	FadeOutAndKillEntity(pEnt, true, 1000, 1000);

}


void ShowQuickMessageBottom(string msg)
{

	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	if (!pMenu)
	{
		return;
	}
	Entity *pEnt = CreateTextLabelEntity(pMenu, "GameMsg", GetScreenSizeXf() / 2, iPhoneMapY(250), msg);
	
	
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
	pEnt->GetComponentByName("TextRender")->GetVar("shadowColor")->Set(MAKE_RGBA(0, 0, 0, 200));
	FadeInEntity(pEnt);
	FadeOutAndKillEntity(pEnt, true, 1000, 1000);

}

void GameOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	if (pMenu->GetEntityByName("PauseMenu")) return;

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "pause")
	{
		if (g_dglos.g_wait_for_button.active == true)
		{
			//sjoy.joybit[5] = TRUE
			g_dglo.m_dirInput[DINK_INPUT_BUTTON5] = true;
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON5] = true;

			//if we ONLY want the game to handle this, we'd enable this return...
			//return;
		}

		if (GetApp()->UseClassicEscapeMenu())
		{

#ifdef WINAPI
/*
			if (GetKeyState(VK_SHIFT) & 0xfe)
			{
				if (!pMenu->GetEntityByName("PauseMenu"))
				{
					pMenu->RemoveComponentByName("FocusInput");
					PauseMenuCreate(pMenu);
				}
				return;
			}
			*/
#endif

			g_dglo.m_dirInput[DINK_INPUT_BUTTON5] = true;
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON5] = true;

			ShowQuickMessageBottom("(Use `wF1`` to bring up the Dink HD menu!)");
			return;
		}
		else
		{
			if (!pMenu->GetEntityByName("PauseMenu"))
			{
				pMenu->RemoveComponentByName("FocusInput");
				PauseMenuCreate(pMenu);
			}
		}

		
	}

	if (pEntClicked->GetName() == "attack")
	{
		
		if (DinkIsWaitingForSkippableDialog())
		{
			if (DinkSkipDialogLine())
			{
				//g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;
				//don't really attack
				return;
			}
		}

		//clicked the attack button
		if (g_dglo.m_lastSubGameMode == DINK_SUB_GAME_MODE_DIALOG)
		{
			//meh, skip it.  On dialog menus we have a button labeled "select" over the punch button we use instead
			return;
		}

		
		g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = true;
		return;
	}

	if (pEntClicked->GetName() == "magic")
	{
		if (DinkIsWaitingForSkippableDialog())
		{
			if (DinkSkipDialogLine())
			{
				//g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;
				//don't really attack
				return;
			}
		}
		g_dglo.m_dirInput[DINK_INPUT_BUTTON3] = true;
		return;
	}

	if (pEntClicked->GetName() == "inventory")
	{
		if (IsDisabledEntity(pEntClicked)) return;
		g_dglo.m_dirInput[DINK_INPUT_BUTTON4] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON4] = true;
		//DisableAllButtonsEntity(pEntClicked);
	return;
	}

	if (pEntClicked->GetName() == "examine")
	{
		g_dglo.m_dirInput[DINK_INPUT_BUTTON2] = true;
		//SendFakeInputMessageToEntity(GetEntityRoot(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message
		return;
	}

	if (pEntClicked->GetName() == "arrow_up")
	{
		g_dglo.m_dirInput[DINK_INPUT_UP] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_UP] = true; //turn off right away
		return;
	}

	if (pEntClicked->GetName() == "arrow_down")
	{
		g_dglo.m_dirInput[DINK_INPUT_DOWN] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_DOWN] = true; //turn off right away
		return;
	}

	if (pEntClicked->GetName() == "select")
	{
		g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true; //turn off right away
		return;
	}

	if (pEntClicked->GetName() == "viewMode")
	{
		LogMsg("Got viewmode command");
		g_dglo.ToggleView();
		return;
	}

	if (pEntClicked->GetName() == "speedup")
	{

		if (g_dglo.m_bSpeedUpMode)
		{
			//turn it off
			GetAudioManager()->Play("audio/speedup_end.wav");

			DinkSetSpeedUpMode(false);
			FlashStopEntity(pEntClicked);
			SetAlphaEntity(pEntClicked, GetApp()->GetVar("gui_transparency")->GetFloat());
			} else
		{

		//turn it on
		GetAudioManager()->Play("audio/speedup_start.wav");
		DinkSetSpeedUpMode(true);
		FlashStartEntity(pEntClicked, 2500);
		}
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void GameOnStopSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	//LogMsg("UNClicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "attack")
	{
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;
		return;
	}

	if (pEntClicked->GetName() == "magic")
	{
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON3] = true;
		return;
	}

	if (pEntClicked->GetName() == "inventory")
	{
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON4] = true;
		return;
	}

	if (pEntClicked->GetName() == "examine")
	{
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON2] = true;
		return;
	}
	
	if (pEntClicked->GetName() == "speedup")
	{
		
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


void KillControls(float fadeTimeMS)
{
	Entity *pControls =GetEntityRoot()->GetEntityByName("Controls");

	if (!pControls) return;

	if (fadeTimeMS != 0)
	{
		pControls->SetName("DYING"); //so we won't find it again
		RemoveFocusIfNeeded(pControls);
	
        VariantList vList(pControls);
        pControls->GetFunction("OnKillingControls")->sig_function(&vList); //just in case someone wants to listen to this message to do 
		//something like stop a fade in-in progress
		FadeEntity(pControls, true, 0, fadeTimeMS, 0);
	}

	g_dglo.m_lastGameMode = DINK_GAME_MODE_NONE;
	g_dglo.m_bFullKeyboardActive = false;

	KillEntity(pControls, fadeTimeMS);
}

void AddViewModeHotspot(Entity *pBG)
{
	if (!GetApp()->GetUsingTouchScreen()) return;

	CL_Vec2f vBarSize = iPhoneMap(CL_Vec2f(220,35));

	if (IsIPADSize)
	{
		vBarSize = iPhoneMap(CL_Vec2f(310,35));
	}

	if (GetApp()->GetIconsOnLeft())
	{
		vBarSize = iPhoneMap(CL_Vec2f(180,35));
		if (IsIPADSize)
		{
			vBarSize = iPhoneMap(CL_Vec2f(270,35));
		}
	}

	Entity * pButtonEntity = CreateButtonHotspot(pBG, "viewMode", iPhoneMap(CL_Vec2f(70,0)), vBarSize);
	pButtonEntity->GetVar("color")->Set(MAKE_RGBA(255,0,0,0));
	pButtonEntity->GetShared()->GetFunction("OnTouchStart")->sig_function.connect(&GameOnSelect);

	if (GetApp()->GetIconsOnLeft())
	{
		pButtonEntity->GetVar("pos2d")->Set(iPhoneMap(CL_Vec2f(100,0)));

	}
	CL_Vec2f vPos = pButtonEntity->GetVar("pos2d")->GetVector2();
	CL_Vec2f vSize = pButtonEntity->GetVar("size2d")->GetVector2();
	
	if (GetApp()->GetVar("showViewHint")->GetUINT32() == 1)
	{
		GetApp()->GetVar("showViewHint")->Set(uint32(0)); //so this won't be shown again
		//give hint to the user about clicking it
		//add a bg bar to make the text easier to see
		//go above the GUI menu otherwise it's fading will effect us
		Entity * pOverlay = CreateOverlayRectEntity(pBG->GetParent(), vPos,  vSize, MAKE_RGBA(0,0,0,150));
		//make the label on top
		CL_Vec2f vLabelPos = vSize/2; //remember, we're a child of the box above, so 0 means the left part of the box, not the screen
		Entity *pLabel = CreateTextLabelEntity(pOverlay, "label",vLabelPos.x, vLabelPos.y, "Tap here to toggle view");
		SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
		
		//fade them in
		FadeInEntity(pOverlay, true, 500, 1000);
		//fade them out
		FadeOutAndKillEntity(pOverlay, true, 1400, 2500);
	}
	
}

float FlipXIfNeeded(float x)
{
	if (GetApp()->GetIconsOnLeft())
	{
		return GetScreenSizeXf()-x;
	}
	return x;
}

CL_Vec2f FlipXIfNeeded(CL_Vec2f vPos)
{
	if (GetApp()->GetIconsOnLeft())
	{
		vPos.x = GetScreenSizeXf()-vPos.x;
	}
	return vPos;
}

void AddSpeedUpButton(Entity *pBG)
{

	if (!GetApp()->GetUsingTouchScreen()) return;

	GetBaseApp()->GetEntityRoot()->RemoveEntityByName("speedup", true);


	CL_Vec2f vPos = FlipXIfNeeded (iPhoneMap(CL_Vec2f(480-115, 4)));

	if (IsIPADSize)
	{
		vPos = CL_Vec2f(FlipXIfNeeded(GetScreenSizeXf()-4), 4);
	}

	/*if (IsIphone4())
	{
		vPos = CL_Vec2f(GetScreenSizeXf()-170, 4);
	}
	*/

	Entity * pButtonEntity =  CreateOverlayButtonEntity(pBG, "speedup", ReplaceWithLargeInFileName("interface/iphone/speed_up_button.rttex"),  vPos.x, vPos.y);
	pButtonEntity->GetVar("ignoreTouchesOutsideRect")->Set(uint32(1)); //ignore touch-up messages not in our rect
	SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);

	if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_LEFT);
	SetButtonClickSound(pButtonEntity, ""); //no sound

	SetAlphaEntity(pButtonEntity, GetApp()->GetVar("gui_transparency")->GetFloat());
	//pButtonEntity->GetVar("alpha")->Set(trans);
	pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
	pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
}

void BuildMouseModeControls(float fadeTimeMS)
{
	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	pBG = pBG->AddEntity(new Entity("Controls"));

	Entity *pVirtualStickEnt = pBG->AddEntity(new Entity);
	pVirtualStickEnt->AddComponent(new CursorComponent);
}

void OnGameKillKeyboard(VariantList *pVList)
{
	g_dglo.m_bFullKeyboardActive = false;
	//g_dglo.m_bLastFullKeyboardActive = false;
}

void OnGameProcessHWKey(VariantList *pVList)
{
	if (pVList->Get(0).GetFloat() != MESSAGE_TYPE_GUI_CHAR) return;

		byte c = toupper(char(pVList->Get(2).GetUINT32()));
	
		
		if (c > 26 && c < 255 )
		{

			switch (c)
			{
		
			case 77:
			case 109:
				//open map
				g_dglo.m_dirInput[DINK_INPUT_BUTTON6] = true;
				g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON6] = true;
				break;

			default:
				if (DinkCanRunScriptNow())
				{
					if (c == 44) c = 188; //convert , to , on old dink mapping, used by some dmods
					if (c == 46) c = 190; //convert . to . on old dink mapping, used by some dmods

					DinkLoadPlayerScript(string("key-" + toString(int(c))));
				}
			}
		}
	
	//LogMsg("Got a %c (%d)", char(pVList->Get(2).GetUINT32()), pVList->Get(2).GetUINT32());
}

void OnGameProcessKey(VariantList *pVList)
{

	if (DinkCanRunScriptNow())
	{

		char c = toupper(char(pVList->Get(1).GetUINT32()));
		
		if (c > 28 && c < 255)
		{

			switch (c)
			{
			/*
			case 32:
			case 54:
			case 55:
			case 37:
			case 38:
			case 39:
			case 40:
			*/
			case 77:
				//open map
				g_dglo.m_dirInput[DINK_INPUT_BUTTON6] = true;
				g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON6] = true;
				break;

			default:
				DinkLoadPlayerScript(string("key-"+toString(toupper(c))));
			}
		}
	}
	//LogMsg("Got a %c (%d)", char(pVList->Get(1).GetUINT32()), pVList->Get(1).GetUINT32());
}

void BuildFullKeyboardControls(float fadeTimeMS)
{
	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	pBG = pBG->AddEntity(new Entity("Controls"));

	Entity *pInput = CreateInputTextEntity(pBG, "input", -500,100, "");
	EntityComponent *pInputComp = pInput->GetComponentByName("InputTextRender");

	pInputComp->GetVar("filtering")->Set(uint32(InputTextRenderComponent::FILTERING_LOOSE));
	pInputComp->GetFunction("ActivateKeyboard")->sig_function(NULL); //open it now
	pInputComp->GetFunction("CloseKeyboard")->sig_function.connect(&OnGameKillKeyboard); //get notified when it closes
	pInputComp->GetFunction("OnChar")->sig_function.connect(&OnGameProcessKey); //get notified when it closes
	pInputComp->GetVar("inputLengthMax")->Set(uint32(255));

	Entity *pBut = CreateTextButtonEntity(pBG, "close_keyboard", 200, 5, "Close Keyboard", false);
	//and run a function here so we can turn it off globally
	pBut->GetFunction("OnButtonSelected")->sig_function.connect(&OnGameKillKeyboard);
	//pVirtualStickEnt->AddComponent(new CursorComponent);
}

void BuildInventoryControls(float fadeTimeMS)
{

	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	pBG = pBG->AddEntity(new Entity("Controls"));
	AddViewModeHotspot(pBG);
	GetEntityRoot()->GetComponentByName("ArcadeInput")->GetVar("trackball_mode")->Set(uint32(ArcadeInputComponent::TRACKBALL_MODE_MENU_SELECTION));

	if (IsInFlingMode())
	{
		Entity *pStick = pBG->AddEntity(new Entity());
		pStick->AddComponent(new FPSControlComponent);

		//add an actual selection button

		Entity *pButtonEntity =  CreateOverlayButtonEntity(pBG, "select", ReplaceWithLargeInFileName("interface/iphone/button_arrow_back.rttex"), 
			iPadMapX(864), iPadMapY(120));
		pButtonEntity->GetVar("alpha")->Set(GetApp()->GetVar("gui_transparency")->GetFloat());
		//FadeInEntity(pButtonEntity);
		pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&GameOnSelect);
		if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);
		//pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		SetButtonStyleEntity(pButtonEntity, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
		SetButtonClickSound(pButtonEntity, ""); //no sound
	}

	Entity *pVirtualStickEnt = pBG->AddEntity(new Entity(new InventoryComponent));
	
	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED && IsDrawingDinkStatusBar())
	{
		float trans = 0.0f;	
		Entity *pButtonEntity;

		CL_Vec2f vButtonSize = DinkToNativeCoords(CL_Vec2f(152 + 64, 412 + 54)) - DinkToNativeCoords(CL_Vec2f(152, 412));

		pButtonEntity = CreateButtonHotspot(pBG, "magic", DinkToNativeCoords(CL_Vec2f(152, 412)), vButtonSize);

		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		
		SetButtonClickSound(pButtonEntity, ""); //no sound

		//I made this touchspot too big on purpose, easier to hit it.
		pButtonEntity = CreateButtonHotspot(pBG, "attack", DinkToNativeCoords(CL_Vec2f(556, 412)), vButtonSize);

		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		SetTouchPaddingEntity(pButtonEntity, CL_Rectf(20,15,40,40));
		SetButtonClickSound(pButtonEntity, ""); //no sound
	}
}

void BuildShowingBMPControls(float fadeTimeMS)
{
	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	pBG = pBG->AddEntity(new Entity("Controls"));

	Entity * pButtonEntity =  CreateButtonHotspot(pBG, "select", CL_Vec2f(40, 40), GetScreenSize());
	//pButtonEntity->GetVar("alpha")->Set(trans);
	pButtonEntity->GetVar("color")->Set(MAKE_RGBA(0,0,0,0)); //invisible

	pButtonEntity->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
	pButtonEntity->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
	SetButtonClickSound(pButtonEntity, ""); //no sound

	if (!IsDesktop())
	{
		Entity *pLabel = CreateTextLabelEntity(pBG, "label", GetScreenSizeXf() / 2, iPhoneMapY(285), "(tap to continue)");
		SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
	}
}

void BuildControls(float fadeTimeMS)
{
	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	pBG = pBG->AddEntity(new Entity("Controls"));

	float iconX = FlipXIfNeeded(iPhoneMapX(400));
	float iconStartY = iPhoneMapY(-2);
	float iconSpacerY = iPhoneMapY(97);
	float trans = 0.0f;
	Entity *pButtonEntity;

#ifdef PLATFORM_HTML5
	if (GetTouchesReceived() > 0)
	{
		//using a touch screen, go into that mode
		GetApp()->SetUsingTouchScreen(true);
	}

#endif

	if (GetApp()->GetUsingTouchScreen())
	{
		if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED && IsDrawingDinkStatusBar())
		{
			//not zoomed in

			if (IsIPADSize)
			{
			 iconX = FlipXIfNeeded(864);
			 iconStartY = 120;
			 iconSpacerY = iPhoneMapY(72);

			} else if (IsIphone4Size) 
			{
				iconX = FlipXIfNeeded(820);
				iconStartY = 5;
			}

		} else
		{
			//adjust for fullscreen view
			if (IsIPADSize)
			{
				iconX = FlipXIfNeeded(890);
				iconStartY = 100;
				iconSpacerY = iPhoneMapY(72);

			}  else if (IsIphone4Size) 
			{
				iconX = FlipXIfNeeded(820);
				iconStartY = 5;
				iconSpacerY = iPhoneMapY(76);
			} else
			{
				//old iphone
				iconSpacerY = iPhoneMapY(76);
				iconX = FlipXIfNeeded(iPhoneMapX(416));
			}
		}

	float iconY = iconStartY;

	//make the area where if you touch it the screen zoom will change

	AddViewModeHotspot(pBG);

	//game icons
	pButtonEntity =  CreateOverlayButtonEntity(pBG, "inventory", ReplaceWithLargeInFileName("interface/iphone/button_inventory.rttex"),  iconX, iconY);
	

	if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);
	pButtonEntity->GetVar("alpha")->Set(trans);
	iconY += iconSpacerY;
	pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
	pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
	SetButtonClickSound(pButtonEntity, ""); //no sound

	
	pButtonEntity =  CreateActionButtonEntity(pBG, "magic", ReplaceWithLargeInFileName("interface/iphone/button_magic_base.rttex"),  iconX, iconY);
	iconY += iconSpacerY;
	if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);

	pButtonEntity->GetVar("alpha")->Set(trans);
	pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
	pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
	SetButtonClickSound(pButtonEntity, ""); //no sound
	
	pButtonEntity =  CreateOverlayButtonEntity(pBG, "examine", ReplaceWithLargeInFileName("interface/iphone/button_examine.rttex"),  iconX, iconY);
	iconY += iconSpacerY;
	if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);

	pButtonEntity->GetVar("alpha")->Set(trans);
	pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
	pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
	//SetButtonRepeatDelayMS(pButtonEntity, 1);

	//pButtonEntity->GetComponentByName("Button2D")->GetVar("buttonStyle")->Set((uint32) Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	SetButtonClickSound(pButtonEntity, ""); //no sound
		
	pButtonEntity->GetParent()->MoveEntityToBottomByAddress(pButtonEntity);
	
	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED && IsDrawingDinkStatusBar())
	{
		//make it so touching the actual game icons on the bottom of the screen do stuff

		CL_Vec2f vButtonSize = DinkToNativeCoords(CL_Vec2f(152+64, 412+54)) - DinkToNativeCoords(CL_Vec2f(152, 412));
		pButtonEntity =  CreateButtonHotspot(pBG, "magic", DinkToNativeCoords(CL_Vec2f(152, 412)), vButtonSize, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
		SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_LEFT);
		pButtonEntity->GetVar("alpha")->Set(trans);
		
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		SetButtonClickSound(pButtonEntity, ""); //no sound

				//I made this touchspot too big on purpose, easier to hit it.
		pButtonEntity =  CreateButtonHotspot(pBG, "attack", DinkToNativeCoords(CL_Vec2f(556, 412)),vButtonSize, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
		iconY += iconSpacerY;
		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		pButtonEntity->GetVar("ignoreTouchesOutsideRect")->Set(uint32(1)); //ignore touch-up messages not in our rect
		SetButtonRepeatDelayMS(pButtonEntity, 0);
		
		SetTouchPaddingEntity(pButtonEntity, CL_Rectf(20,15,40,40));
		SetButtonClickSound(pButtonEntity, ""); //no sound
	
	} else
	{
	
		pButtonEntity =  CreateActionButtonEntity(pBG, "attack", ReplaceWithLargeInFileName("interface/iphone/button_item_base.rttex"),  iconX, iconY);
		if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);

		iconY += iconSpacerY;
		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		pButtonEntity->GetVar("ignoreTouchesOutsideRect")->Set(uint32(1)); //ignore touch-up messages not in our rect

		SetButtonClickSound(pButtonEntity, ""); //no sound
		pButtonEntity->GetParent()->MoveEntityToBottomByAddress(pButtonEntity);
	}

 		FadeEntity(pBG, true, GetApp()->GetVar("gui_transparency")->GetFloat(), fadeTimeMS, 0);
	}

	eControlStyle controlStyle = (eControlStyle) GetApp()->GetVar("controlStyle")->GetUINT32();
	
	switch(controlStyle)
	{
	case CONTROLS_JOYPAD:
	case CONTROLS_FLING:
		{
			Entity *pVirtualStickEnt = pBG->AddEntity(new Entity());
			pVirtualStickEnt->AddComponent(new FPSControlComponent);

			if (GetApp()->GetUsingTouchScreen())
			{
			
				bool moveStick = false;

				if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED && IsDrawingDinkStatusBar())
				{
					moveStick = true;
					//ignore touches around the magic button
					Entity *pVirtualStickArrowEnt = pBG->GetEntityByName("arrow_gui"); //in case we need it later on
					
					/*
					EntityComponent *pFilter = pVirtualStickArrowEnt->AddComponent(new FilterInputComponent);
					pFilter->GetVar("mode")->Set(uint32(FilterInputComponent::MODE_IGNORE_ABSOLUTE_CLIP_RECT));
					pFilter->GetVar("clipRect")->Set(CL_Rectf(iPhoneMapX(113), iPhoneMapY(272),iPhoneMapX(113+55), iPhoneMapY(272+46)));
					*/
				}

				if (moveStick)
				{
					//make some changes to the virtual pad arrow to fit this layout better
					Entity *pVirtualStickArrowEnt = pBG->GetEntityByName("arrow_gui"); //in case we need it later on

					CL_Vec2f vArrowPos = CL_Vec2f(FlipXIfNeeded(iPhoneMapX(80)), iPhoneMapY(201));

					if (IsIPADSize)
					{
						vArrowPos = CL_Vec2f( FlipXIfNeeded(149-30), 520+30);
						if (IsInFlingMode())
						{

							vArrowPos.y = C_FLING_JOYSTICK_Y;
						}
					}

					pVirtualStickArrowEnt->GetVar("pos2d")->Set(vArrowPos);
				}
			}
		}
		break;

	case CONTROLS_DRAG_ANYWHERE:
		{
			Entity *pControls = pBG->AddEntity(new Entity());
			pControls->AddComponent(new DragControlComponent);
		}
		break;


	default:

		assert(!"Just no");
	}
}


void BuildDialogModeControls(float fadeTimeMS)
{

	
	Entity *pBG = GetEntityRoot()->GetEntityByName("GameMenu");

	if (!pBG)
	{
		assert(!"herm");
		return;
	}

	 
	pBG = pBG->AddEntity(new Entity("Controls"));

    EntityComponent *pCursor = pBG->AddComponent(new CursorComponent); //for mouse control of dialog

	if (!GetApp()->GetUsingTouchScreen()) return;

	float trans = 0.0f;
	Entity *pButtonEntity;

	//GetEntityRoot()->GetComponentByName("ArcadeInput")->GetVar("trackball_mode")->Set(uint32(ArcadeInputComponent::TRACKBALL_MODE_MENU_SELECTION));

	//make the area where if you touch it the screen zoom will change
	AddViewModeHotspot(pBG);
	
	//game icons
	float posX = iPhoneMapX(1);

	CL_Vec2f vUpArrowPos = iPhoneMap(posX,70);
	CL_Vec2f vDownArrowPos = iPhoneMap(posX,212);
	CL_Vec2f vOkPos = iPhoneMap(409, 136+90);

	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED)
	{
		float posX = iPhoneMapX(10);
		vUpArrowPos = iPhoneMap(posX,46);
		vDownArrowPos =  iPhoneMap(posX,181);
		vOkPos = iPhoneMap(398,110+70);
	}

	if (IsIPADSize)
	{
		posX = 17;
		vUpArrowPos = CL_Vec2f(posX,277+50);
		vDownArrowPos =  CL_Vec2f(posX,404+50);
		vOkPos = CL_Vec2f(815+50,330+60+70);

		if (g_dglo.GetActiveView() == DinkGlobals::VIEW_ZOOMED)
		{
			posX = 2;
			vUpArrowPos = CL_Vec2f(posX,277+50+100);
			vDownArrowPos =  CL_Vec2f(posX,404+50+100);
			vOkPos = CL_Vec2f(815+50+10,330+60+80+70);
		}
	}

	if (IsInFlingMode())
	{
	
			Entity *pStick = pBG->AddEntity(new Entity());
			pStick->AddComponent(new FPSControlComponent);


			/*
		posX = 47;
		vUpArrowPos = CL_Vec2f(posX, (277+50+90)-20);
		vDownArrowPos =  CL_Vec2f(posX, (404+50+90) + 20);
		*/
	}

	if (GetApp()->GetIconsOnLeft())
	{
		//flip controls around
		vUpArrowPos = FlipXIfNeeded(vUpArrowPos);
		vDownArrowPos = FlipXIfNeeded(vDownArrowPos);
		vOkPos = FlipXIfNeeded(vOkPos);
	}

	if (!IsInFlingMode())
	{
		pButtonEntity =  CreateOverlayButtonEntity(pBG, "arrow_up", ReplaceWithLargeInFileName("interface/iphone/button_arrow_up.rttex"),  vUpArrowPos.x, vUpArrowPos.y);
		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		SetButtonClickSound(pButtonEntity, ""); //no sound
		Entity * pUp = pButtonEntity;

		if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);

		pButtonEntity =  CreateOverlayButtonEntity(pBG, "arrow_down", ReplaceWithLargeInFileName("interface/iphone/button_arrow_down.rttex"),  vDownArrowPos.x, vDownArrowPos.y);
		pButtonEntity->GetVar("alpha")->Set(trans);
		pButtonEntity->GetShared()->GetFunction("OnOverStart")->sig_function.connect(&GameOnSelect);
		if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);
		pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
		SetButtonClickSound(pButtonEntity, ""); //no sound
		Entity * pDown = pButtonEntity;
	}

	pButtonEntity =  CreateOverlayButtonEntity(pBG, "select", ReplaceWithLargeInFileName("interface/iphone/button_arrow_back.rttex"),  vOkPos.x, vOkPos.y);
	pButtonEntity->GetVar("alpha")->Set(trans);
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&GameOnSelect);
	if (GetApp()->GetIconsOnLeft()) SetAlignmentEntity(pButtonEntity, ALIGNMENT_UPPER_RIGHT);
	
	
	//pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
	//SetButtonStyleEntity(pButtonEntity, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE);
	SetButtonClickSound(pButtonEntity, ""); //no sound
	//SetButtonRepeatDelayMS(pButtonEntity, 1);

	Entity * pSelect = pButtonEntity;

	FadeEntity(pBG, true, GetApp()->GetVar("gui_transparency")->GetFloat(), fadeTimeMS, 0);


	//disable it until it's visible
	DisableComponentByName(pButtonEntity, "Button2D");
	EnableComponentByName(pButtonEntity, "Button2D", 1000);

	//override the fading on this one button because we want them to see it's not clickable yet
	FadeEntity(pButtonEntity, false, 0, 0, 0, false);
	FadeInEntity(pButtonEntity, false, 300, 900, GetApp()->GetVar("gui_transparency")->GetFloat());

	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED)
	{
		if (IsDrawingDinkStatusBar())
		{
			//let's let the punch icon also select, it just feels natural
			//pButtonEntity =  CreateButtonHotspot(pBG, "select", CL_Vec2f(iPhoneMapX(412), iPhoneMapY(272)), CL_Vec2f(iPhoneMapX(62), iPhoneMapY(46)));
		
			CL_Vec2f vButtonSize = DinkToNativeCoords(CL_Vec2f(152 + 64, 412 + 54)) - DinkToNativeCoords(CL_Vec2f(152, 412));

			pButtonEntity = CreateButtonHotspot(pBG, "select", DinkToNativeCoords(CL_Vec2f(556, 412)), vButtonSize);



			pButtonEntity->GetVar("alpha")->Set(trans);
			pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&GameOnSelect);
		//	pButtonEntity->GetShared()->GetFunction("OnOverEnd")->sig_function.connect(&GameOnStopSelect);
			SetButtonClickSound(pButtonEntity, ""); //no sound

			DisableComponentByName(pButtonEntity, "Button2D");
			EnableComponentByName(pButtonEntity, "Button2D", 1000);

		}
	} 

	pBG->MoveComponentToBottomByAddress(pCursor);
}

void RecomputeAspectRatio();

void UpdateControlsGUIIfNeeded()
{

	if (g_dglo.m_lastGameMode != GetDinkGameMode()
		|| g_dglo.m_lastSubGameMode != GetDinkSubGameMode()
		//|| g_dglo.m_bWaitingForSkippableConversation != DinkIsWaitingForSkippableDialog()
		|| g_dglo.m_lastActiveView != g_dglo.GetActiveView()
		|| g_dglo.m_lastIsUpdatingDinkStatusBar != IsDrawingDinkStatusBar()
		|| g_dglo.m_bFullKeyboardActive != g_dglo.m_bLastFullKeyboardActive
		|| g_dglo.m_bForceControlsRebuild
	)
	{
		
		if (g_dglo.m_bLastFullKeyboardActive && g_dglo.m_bFullKeyboardActive)
		{
			//don't care, leave the keyboard up
			return;
		}
		g_dglo.m_bForceControlsRebuild = false;
		//kill any existing controls
		KillControls(300);

		g_dglo.m_lastGameMode = GetDinkGameMode();
		g_dglo.m_lastSubGameMode = GetDinkSubGameMode();
		g_dglo.m_bWaitingForSkippableConversation = DinkIsWaitingForSkippableDialog();
		g_dglo.m_lastActiveView = g_dglo.GetActiveView();
		g_dglo.m_lastIsUpdatingDinkStatusBar = IsDrawingDinkStatusBar();
		g_dglo.m_bLastFullKeyboardActive = g_dglo.m_bFullKeyboardActive;

		GetEntityRoot()->GetComponentByName("ArcadeInput")->GetVar("trackball_mode")->Set(uint32(ArcadeInputComponent::TRACKBALL_MODE_WALKING));

		Entity *pMainMenu = GetEntityRoot()->GetEntityByName("GameMenu");
		//RecomputeAspectRatio();

		AddSpeedUpButton(pMainMenu);
		UpdatePauseMenuPosition(pMainMenu);
		
		if (g_dglo.m_bFullKeyboardActive)
		{
			BuildFullKeyboardControls(300);
			return;
		}

		if (g_dglo.m_lastSubGameMode == DINK_SUB_GAME_MODE_DIALOG)
		{
			BuildDialogModeControls(300);
			return;
		}

		if (g_dglo.m_lastSubGameMode == DINK_SUB_GAME_MODE_SHOWING_BMP)
		{
			BuildShowingBMPControls(300);
			return;
		}
		if (g_dglo.m_lastGameMode == DINK_GAME_MODE_INVENTORY)
		{
			BuildInventoryControls(300);
			return;
		}

		if (g_dglo.m_lastGameMode == DINK_GAME_MODE_MOUSE)
		{
			BuildMouseModeControls(300);
			return;
		}
		
		BuildControls(300);
	}
}

void OnGameMenuDelete(VariantList *pVList)
{
	finiObjects();
}

void ApplyAspectRatioGLMatrix();

void OnGameMenuRender(VariantList *pVList)
{
	//apply matrix
	glMatrixMode(GL_MODELVIEW);
	
	//well, we might as well always clear the BG, because some dmods like redink1 set transparency in the status bar which causes glitches if we don't

	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED)
	{
		//clear background if needed
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	//apply scale offset
	ApplyAspectRatioGLMatrix();

	updateFrame();

	if (DinkGetSpeedUpMode())
	{
		//3x speed
		
		
		int speedup = 2;

#ifdef WINAPI

		if (GetKeyState(VK_SHIFT) & 0xfe)
		{
			speedup = 8; //super turbo mode
			if (GetKeyState(VK_CONTROL) & 0xfe)
			{
				speedup *= 3; //super turbo mode
			}
		}

#endif
		for (int i = 0; i < speedup; i++)
		{
			//GetApp()->SetGameTick(GetApp()->GetGameTick() + GetApp()->GetDeltaTick() * 3);
			//GetApp()->GetGameTimer()->Update();

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			updateFrame();
		}

	}
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	//remove matrix
}

void OnAutoSave(VariantList *pVList)
{
	if (GetDinkGameState() == DINK_GAME_STATE_PLAYING && DinkGetHealthPercent() > 0.3f)
	{
		if (GetDinkGameMode() == DINK_GAME_MODE_NORMAL &&
			( GetDinkSubGameMode() == DINK_SUB_GAME_MODE_NORMAL || GetDinkSubGameMode() == DINK_SUB_GAME_MODE_NONE)
			)
		{
			SaveAutoSave();
			SyncPersistentData();
			//reschedule this function to run again in a bit
			GetMessageManager()->CallEntityFunction(pVList->Get(0).GetEntity(), AUTO_SAVE_MS, "OnAutoSave", pVList);
			return;
		}

	} 

	//if we got here, we failed to save due to it being dangerous because of low health.  Let's try again in a few seconds...
	GetMessageManager()->CallEntityFunction(pVList->Get(0).GetEntity(), 5000, "OnAutoSave", pVList);
}

void OnArcadeInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	//LogMsg("GameMenuArcade: Key %d, down is %d", vKey, int(bIsDown));

	switch (vKey)
	{
	case 9: //tab
		//LogMsg("Tab: %d", int(bIsDown));
		DinkSetSpeedUpMode(bIsDown);
		break;

	case VIRTUAL_KEY_CUSTOM_QUIT: //tab
			//LogMsg("Tab: %d", int(bIsDown));
		LogMsg("Quitting");
		break;

	case VIRTUAL_KEY_F4:
	
		if (bIsDown)
		SaveStateWithExtra();
		break;

	case VIRTUAL_KEY_F1:

		if (bIsDown)
		{

			Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

				if (pMenu)
				{
					if (!pMenu->GetEntityByName("PauseMenu"))
					{
						pMenu->RemoveComponentByName("FocusInput");
						PauseMenuCreate(pMenu);
					}
				}
		}
		break;


	case VIRTUAL_KEY_F8:
		{
			if (bIsDown)
			{
				if (GetDinkGameState() == DINK_GAME_STATE_PLAYING)
				{
					string fName = DinkGetSavePath() + "quicksave.dat";

					if (FileExists(fName))
					{
						LoadStateWithExtra();
					}
					else
					{
						ShowQuickMessage("No state to load yet.");
					}
				}
				else
				{
					ShowQuickMessage("(can't load state yet, still loading game!)");
				}
			}
		}
		break;

	case VIRTUAL_KEY_GAME_MAGIC:

		if (bIsDown)
		{
			if (DinkIsWaitingForSkippableDialog())
			{
				if (DinkSkipDialogLine())
				{
					return;
				}
			}
			g_dglo.m_dirInput[DINK_INPUT_BUTTON3] = true;
		} else
		{
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON3] = true;
		}
		break;
		
		//EXAMINE
	case VIRTUAL_KEY_GAME_TALK:

		/*
		if (DinkIsWaitingForSkippableDialog())
		{
			if (DinkSkipDialogLine())
			{
				return;
			}
		}
		*/

		if (bIsDown)
		{
			g_dglo.m_dirInput[DINK_INPUT_BUTTON2] = true;
		} else
		{
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON2] = true;
		}
		break;

	case VIRTUAL_KEY_GAME_INVENTORY: //inventory

		if (bIsDown)
		{
			g_dglo.m_dirInput[DINK_INPUT_BUTTON4] = true;
		} else
		{
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON4] = true;
		}
		break;

	case VIRTUAL_KEY_GAME_FIRE: //select//atack

		if (bIsDown)
		{
			if (DinkIsWaitingForSkippableDialog())
			{
				if (DinkSkipDialogLine())
				{
					return;
				}
			}
			
			g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = true;
		} else
		{
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;

		}
		break;
	}


	if (GetDinkSubGameMode() == DINK_SUB_GAME_MODE_DIALOG || GetDinkGameMode() == DINK_GAME_MODE_INVENTORY || GetDinkGameMode() ==
		DINK_GAME_MODE_MOUSE)
	{
		switch(vKey)
		{
		case VIRTUAL_KEY_DIR_LEFT:
			SendKey(DINK_INPUT_LEFT, bIsDown);
			break;

		case VIRTUAL_KEY_DIR_RIGHT:
			SendKey(DINK_INPUT_RIGHT, bIsDown);
			break;

		case VIRTUAL_KEY_DIR_UP:
			SendKey(DINK_INPUT_UP, bIsDown);
			break;

		case VIRTUAL_KEY_DIR_DOWN:
			SendKey(DINK_INPUT_DOWN, bIsDown);
			break;
		}
	}

}

void OnRawCharInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;
	//LogMsg("GameMenuRaw: Key %d, down is %d", vKey, int(bIsDown));
}

void UpdatePauseMenuPosition(Entity *pBG)
{
	Entity *pPause = pBG->GetEntityByName("pause");

	assert(pPause);

	CL_Vec2f vPausePos = CL_Vec2f(12,0);

	if (IsLargeScreen())
	{
		vPausePos = CL_Vec2f(27,0);
	}

	vPausePos.x = FlipXIfNeeded(vPausePos.x);

	pPause->GetVar("pos2d")->Set(vPausePos);

	if (GetApp()->GetIconsOnLeft())
	{
		SetAlignmentEntity(pPause, ALIGNMENT_UPPER_RIGHT);
	} else
	{

		SetAlignmentEntity(pPause, ALIGNMENT_UPPER_LEFT);
	}

}

void GameFinishLoading(Entity *pBG)
{

#ifdef PLATFORM_HTML5
	if (GetTouchesReceived() > 0)
	{
		GetApp()->SetUsingTouchScreen(true);
	}
#endif

	float trans = rt_max(0.4, GetApp()->GetVar("gui_transparency")->GetFloat());
	Entity *pButtonEntity;
	DestroyUnusedTextures();
	
	pButtonEntity =  CreateOverlayButtonEntity(pBG, "pause", ReplaceWithLargeInFileName("interface/iphone/pause_icon.rttex"), 0, 0);

	UpdatePauseMenuPosition(pBG);

	SetTouchPaddingEntity(pButtonEntity, CL_Rectf(0,0,0,0));
	//SetButtonStyleEntity(pButtonEntity, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);

	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);	
	//AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_PROPERTIES);	

	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&GameOnSelect);
	SetButtonClickSound(pButtonEntity, ""); //no sound

	if (!GetApp()->GetUsingTouchScreen())
	{
		pButtonEntity->GetVar("alpha")->Set(0.0f);
	} else
	{
		pButtonEntity->GetVar("alpha")->Set(trans);
	}

	FadeOutAndKillEntity(pBG->GetEntityByName("game_loading"), true, 300, 50);

	pBG->GetShared()->GetFunction("OnRender")->sig_function.connect(&OnGameMenuRender);
	pBG->GetShared()->GetFunction("OnDelete")->sig_function.connect(&OnGameMenuDelete);
	
	
	GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
	pBG->GetShared()->GetFunction("OnArcadeInput")->sig_function.connect(&OnArcadeInput);


//if (IsDesktop())
{
		static bool bOneTimeKeyboardAttach = false;
		if (!bOneTimeKeyboardAttach)
		{
			GetBaseApp()->m_sig_input.connect(&OnGameProcessHWKey);
			bOneTimeKeyboardAttach = true;
		}
}

	/*
	GetBaseApp()->m_sig_raw_keyboard.connect(pBG->GetFunction("OnRawCharInput")->sig_function);
	pBG->GetShared()->GetFunction("OnRawCharInput")->sig_function.connect(&OnRawCharInput);
*/

	AddSpeedUpButton(pBG);
	SetDinkGameState(DINK_GAME_STATE_PLAYING);

	if (GetPrimaryGLX() != 0)
	{
		SetupOrtho();
		DinkOnForeground(); //rebuild lost surfaces

		if (GetDinkGameState() != DINK_GAME_STATE_PLAYING)
		{
			PrepareForGL();
		}

	}

	if (AUTO_SAVE_MS != 0)
	{
		//AutoSave
		pBG->GetFunction("OnAutoSave")->sig_function.connect(&OnAutoSave);
		VariantList vList(pBG);
        GetMessageManager()->CallEntityFunction(pBG, AUTO_SAVE_MS, "OnAutoSave", &vList);
	}
}


void GameSetProgressBar(float progress)
{
	Entity *pBar = GetEntityRoot()->GetEntityByName("bar");

	if (pBar)
	{
		pBar->GetComponentByName("ProgressBar")->GetVar("progress")->Set(progress);
	}
}

void ShowQuickTip(VariantList *pVList)
{
	Entity *pBG = pVList->m_variant[0].GetEntity();
	CreateQuickTip(pBG, pVList->m_variant[1].GetString(), false);
}


void GameLoadPiece(VariantList *pVList)
{
	Entity *pBG = pVList->m_variant[0].GetEntity();
	string stateToLoad = pVList->m_variant[2].GetString();

	float &progress = pBG->GetVar("progress")->GetFloat();
	int gameIdToLoad = pVList->m_variant[1].GetUINT32();


	if (pBG->GetVar("didInit")->GetUINT32() == 0)
	{
		//initialize Dink
		if (!InitDinkEngine())
		{
			LogMsg("Error initializing videohw");
			Entity *pMenu = DinkQuitGame();
			PopUpCreate(pMenu, "Error initializing Dink engine.  Don't know why!", "", "cancel", "Continue", "", "", true);
			return;

		
		}
		pBG->GetVar("didInit")->Set(uint32(1));

	}

	if (progress == 1)
	{
		//we're done
		GameFinishLoading(pBG);
		return;
	}

	if (!stateToLoad.empty())
	{
		//loading a state

		if (progress == 0)
		{
			//first time, setup paths only
			bool bSuccess = LoadState(stateToLoad, true);
		
			if (!bSuccess)
			{
				RemoveFile(stateToLoad, false);
				WriteLastPathSaved("");
				Entity *pMenu = DinkQuitGame();
				PopUpCreate(pMenu, "Error loading save state.  Probably an older version, sorry.", "", "cancel", "Continue", "", "", true);
				return;
			}
		} else
		{
			bool bSuccess = LoadState(stateToLoad, false);
			
			if (!IsInString(stateToLoad, "autosave.dat"))
			{
				//instead of removing it, let it stay around in case there is a game crash?
				//RemoveFile(stateToLoad, false); //SETH changed 8/28/2017
			}

			//we're done
			progress = 1;
			g_dglo.m_curLoadState = FINISHED_LOADING;
			GameSetProgressBar(progress);
			VariantList vList(pBG, uint32(gameIdToLoad), stateToLoad);
            GetMessageManager()->CallEntityFunction(pBG, 1, "GameLoadPiece", &vList); 
			return;
		}
	} 
	
	if (!LoadGameChunk(gameIdToLoad, progress))
	{
		LogMsg("Error initializing game engine");
		return;
	}

	GameSetProgressBar(progress);
	//reschedule this to run again
	VariantList vList(pBG, uint32(gameIdToLoad), stateToLoad);
    GetMessageManager()->CallEntityFunction(pBG, 1, "GameLoadPiece", &vList); 
}



Entity * GameCreate(Entity *pParentEnt, int gameIDToLoad, string stateToLoad, string msgToShow)
{
	Entity *pBG = pParentEnt->AddEntity(new Entity("GameMenu"));
	AddFocusIfNeeded(pBG);
	Entity *pLoading = CreateOverlayEntity(pBG, "game_loading", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0, 0);

	//setup the splash too
	string splashBmp;
	if (!g_dglo.m_dmodGamePathWithDir.empty() && FileExists(g_dglo.m_dmodGamePathWithDir + "tiles\\splash.bmp"))
	{
		splashBmp = g_dglo.m_dmodGamePathWithDir + "tiles\\splash.bmp";
	}
	else
	{
		splashBmp = g_dglo.m_gamePathWithDir + "tiles\\splash.bmp";
	}

	SoftSurface s8bit;
	if (!s8bit.LoadFile(splashBmp, SoftSurface::COLOR_KEY_NONE, false))
	{
	//give uop
	}
	else
	{
		//if it was 8bit, this will convert it to 32

		SoftSurface s;
		s.Init(s8bit.GetWidth(), s8bit.GetHeight(), SoftSurface::SURFACE_RGBA);
		s.Blit(0, 0, &s8bit);
		s.FlipY();

		SurfaceAnim *pSurf;

		pSurf = new SurfaceAnim;

		pSurf->SetTextureType(Surface::TYPE_DEFAULT); //insure no mipmaps are created
		pSurf->InitBlankSurface(s.GetWidth(), s.GetHeight());
		pSurf->UpdateSurfaceRect(rtRect(0, 0, s.GetWidth(), s.GetHeight()), s.GetPixelData());

		//add the icon
		Entity *pEnt = CreateOverlayEntity(pLoading, "icon", "", GetScreenSizeXf()/2, GetScreenSizeYf()/2);
		OverlayRenderComponent *pOverlay = (OverlayRenderComponent*)pEnt->GetComponentByName("OverlayRender");
		pOverlay->SetSurface(pSurf, true);
		SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
		//EntitySetScaleBySize(pEnt, GetDMODBarIconSize());
	}


	//*********************
	if (msgToShow.empty())
	{
		msgToShow = "Loading...";
	}
	
	Entity *pLabel = CreateTextLabelEntity(pLoading, "load_label", GetScreenSizeXf()/2, GetScreenSizeYf()-30, msgToShow);
	SetupTextEntity(pLabel, FONT_LARGE);
	SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
	pBG->GetFunction("GameLoadPiece")->sig_function.connect(&GameLoadPiece);
    VariantList vList(pBG, uint32(gameIDToLoad), stateToLoad);
	GetMessageManager()->CallEntityFunction(pBG, 501, "GameLoadPiece", &vList); 
	SlideScreen(pBG, true);
	
	Entity *pProgressBar = pLoading->AddEntity(new Entity("bar"));
	EntityComponent *pBar = pProgressBar->AddComponent(new ProgressBarComponent);
	pProgressBar->GetVar("pos2d")->Set(CL_Vec2f(iPhoneMapX(80),iPhoneMapY(280)));
	pProgressBar->GetVar("size2d")->Set(CL_Vec2f(iPhoneMapX(310),iPhoneMapY(15)));
	pProgressBar->GetVar("color")->Set(MAKE_RGBA(200,200,0,60));
	pBar->GetVar("interpolationTimeMS")->Set(uint32(1)); //update faster
	pBar->GetVar("borderColor")->Set(MAKE_RGBA(200,200,0,180));
	pBG->GetFunction("ShowQuickTip")->sig_function.connect(&ShowQuickTip);
	return pBG;
}

bool IsInFlingMode()
{
	return false; //fling is dead
	//return (GetApp()->GetShared()->GetVar("controlStyle")->GetUINT32() == CONTROLS_FLING);
}

