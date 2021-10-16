#include "PlatformPrecomp.h"
#include "PauseMenu.h"
#include "Entity/EntityUtils.h"
#include "../dink/dink.h"
#include "MainMenu.h"
#include "DebugMenu.h"
#include "GameMenu.h"
#include "DMODMenu.h"
#include "OptionsMenu.h"
#include "PopUpMenu.h"
#include "Entity/SelectButtonWithCustomInputComponent.h"


#if defined _DEBUG || defined PLATFORM_HTML5

const bool G_ALLOW_SAVE_EXPORTING = true;

#else
const bool G_ALLOW_SAVE_EXPORTING = false;
#endif

#ifdef PLATFORM_HTML5
#include "HTML5Utils.h"
#endif


void PlayMenuMusic()
{
	
#ifdef PLATFORM_HTML5
	//hacking this in for now, figure out how to consolidate file types between all platforms later. I mean, mp3 was a legal problem
	//previously but now it's ok.  But not sure if native android build can play them or not.  Does the android build even use FMOD?  Can't recall
	GetAudioManager()->Play("dink/sound/3.mp3", true, true, true);

#else
	GetAudioManager()->Play("dink/sound/3.ogg", true, true, true);

#endif
}

Entity * DinkQuitGame()
{

	SetDinkGameState(DINK_GAME_STATE_NOT_PLAYING);

	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	assert(pMenu);
	if (!pMenu) return NULL;
	
	GetBaseApp()->SetGameTickPause(false);
	//AddFocusIfNeeded(pMenu);
	
	SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	
	Entity *pFinalMenu = NULL;

	if (g_dglo.m_dmodGameDir.empty())
	{
		pFinalMenu = MainMenuCreate(pMenu->GetParent(), true);
	} else
	{
		pFinalMenu = DMODMenuCreate(pMenu->GetParent(), true);
	}

	PlayMenuMusic();

	return pFinalMenu;
}

Entity * DinkRestartGame()
{

	SetDinkGameState(DINK_GAME_STATE_NOT_PLAYING);

	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	assert(pMenu);
	if (!pMenu) return NULL;

	GetBaseApp()->SetGameTickPause(false);
	AddFocusIfNeeded(pMenu);

	string dmodDirAndPath = RemoveTrailingBackslash(g_dglo.m_dmodGamePathWithDir);

	LogMsg("Restarting dmod %s", dmodDirAndPath.c_str());
	InitDinkPaths(GetBaseAppPath(), "dink", dmodDirAndPath);
	//SlideScreen(pMenu, false);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	
	Entity *pFinalMenu = NULL;
	GameCreate(pMenu->GetParent(), 0, "", "Restarting...");
	/*
	if (g_dglo.m_dmodGameDir.empty())
	{
	pFinalMenu = MainMenuCreate(pMenu->GetParent(), true);
	} else
	{
	pFinalMenu = DMODMenuCreate(pMenu->GetParent(), true);
	}


	PlayMenuMusic();
	*/
	return pFinalMenu;
}

void PauseEnd(Entity *pMenu)
{
	//slide it off the screen and then kill the whole menu tree
	RemoveFocusIfNeeded(pMenu);
	DisableAllButtonsEntity(pMenu);
	//SlideScreen(pEntClicked->GetParent(), false);
	AddFocusIfNeeded(pMenu->GetParent());
	FadeOutEntity(pMenu, true, 499);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
	GetBaseApp()->SetGameTickPause(false);

	//SyncPersistentData();

	
}

void PrepareForPopup(Entity *pMainMenuEnt, VariantList *pVList)
{
	SendFakeInputMessageToEntity(GetEntityRoot(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message
	pMainMenuEnt->RemoveComponentByName("FocusInput");
	pMainMenuEnt->RemoveComponentByName("FocusUpdate");
	GetMessageManager()->RemoveComponentByName(pMainMenuEnt, 500, "FocusRender");
}

void UpdatePauseButtons(Entity *pMenu)
{
	Entity *pEnt = pMenu->GetEntityByName("QuickLoad");

	if (!pEnt)
	{
		assert(0);
		return;
	}
	unsigned int color = MAKE_RGBA(255,255,255,255);
	
	if (!FileExists(DinkGetSavePath()+"quicksave.dat"))
	{

		//we'll, let's make it look not highlighted
		color = MAKE_RGBA(255,255,255,100);

	}

	MorphToColorEntity(pEnt, false, 500, color, 0);
}


void PauseMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity", pEntClicked->GetName().c_str());
	
	Entity *pMenu = GetEntityRoot()->GetEntityByName("PauseMenu");

	if (pEntClicked->GetName() == "Back")
	{
		GetAudioManager()->Play("audio/pause_close.wav");
		PauseEnd(pMenu);
	}

	if (pEntClicked->GetName() == "OldOptions")
	{
		PauseEnd(pMenu);
		
		g_dglo.m_dirInput[DINK_INPUT_BUTTON5] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON5] = true;
	}

	if (pEntClicked->GetName() == "Map")
	{
		PauseEnd(pMenu);

		g_dglo.m_dirInput[DINK_INPUT_BUTTON6] = true;
		g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON6] = true;
	}
	if (pEntClicked->GetName() == "Keyboard")
	{
		PauseEnd(pMenu);

		g_dglo.m_bFullKeyboardActive = true;
	}

	if (pEntClicked->GetName() == "Quit")
	{
		//slide it off the screen and then kill the whole menu tree
		RemoveFocusIfNeeded(pMenu);
		SaveState(g_dglo.m_savePath+"continue_state.dat", false);
		WriteLastPathSaved("");
		//kill our state.dat if it existed, not needed now, this can exist if an iphone goes into suspend, but then is resumed
		RemoveFile(GetSavePath()+"state.dat", false);
		//SlideScreen(pEntClicked->GetParent()->GetParent(), false);			
		DinkQuitGame();
		//SyncPersistentData();
	}

	if (pEntClicked->GetName() == "Debug")
	{
		//overlay the debug menu over this one
		pMenu->RemoveComponentByName("FocusInput");
		DebugMenuCreate(pMenu);
	}

	if (pEntClicked->GetName() == "Options")
	{
		//overlay the debug menu over this one
		PrepareForPopup(pMenu, pVList);
		OptionsMenuCreate(pMenu);
	}
	if (pEntClicked->GetName() == "empty_cache")
	{
		DinkUnloadGraphicsCache();
		LogMsg("Cache emptied");
	}

	if (pEntClicked->GetName() == "QuickSave")
	{
		SaveStateWithExtra();
		UpdatePauseButtons(pMenu);
		PauseEnd(pMenu);

	}

		if (pEntClicked->GetName() == "ExportNormalSave")
		{
			if (g_lastSaveSlotFileSaved.empty())
			{
				PopUpCreate(pMenu, "You haven't used a save machine recently. Go use one and come back!", "", "cancel", "Continue", "", "", true);
				return;
			}

			string fileName = GetFileNameFromString(g_lastSaveSlotFileSaved);

#ifdef PLATFORM_HTML5
			HTMLDownloadFileFromFileSystem(g_lastSaveSlotFileSaved, fileName);
#else
#endif
			UpdatePauseButtons(pMenu);
			PauseEnd(pMenu);
			PopUpCreate(pMenu->GetParent(), "Started download of last save slot saved. (" + fileName + ")", "", "cancel", "Continue", "", "", true);
		}

		if (pEntClicked->GetName() == "ImportNormalSave")
		{
#ifdef PLATFORM_HTML5
			HTMLUploadFileToFileSystem();
#else

			GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_HTML5_GOT_UPLOAD, 0, 0, 0, "purpoise_quicksave.dat", 0);
#endif
			PauseEnd(pMenu);

		}

	if (pEntClicked->GetName() == "ExportQuickSave")
	{
		//SaveStateWithExtra(false);
		//UpdatePauseButtons(pMenu);
	
		GetAudioManager()->Play("audio/quick_save.wav");
		SaveState(DinkGetSavePath() + "quicksave.dat");

		string prepend = g_dglo.m_dmodGameDir;

		if (prepend.empty()) prepend = "dink";
		prepend = RemoveTrailingBackslash(prepend);
#ifdef PLATFORM_HTML5
		
		HTMLDownloadFileFromFileSystem(DinkGetSavePath() + "quicksave.dat", prepend + "_quicksave.dat");
#else

		//Fake message for debugging

#endif
		PauseEnd(pMenu);
		ShowQuickMessage("Download started");
	
	}

	if (pEntClicked->GetName() == "ImportQuickSave")
	{
	
#ifdef PLATFORM_HTML5
		HTMLUploadFileToFileSystem();
#else
		LogMsg("Um, not a browser version, what are we supposed to do here?  Here's a way to just loading a quicksave for testing though.");
		GetMessageManager()->SendGUIStringEx(MESSAGE_TYPE_HTML5_GOT_UPLOAD, 0, 0, 0, "purpoise_quicksave.dat", 0);
#endif
		PauseEnd(pMenu);

	}

	if (pEntClicked->GetName() == "QuickLoad")
	{
		LogMsg("loading state");
		string fName = DinkGetSavePath()+"quicksave.dat";

		if (FileExists(fName))
		{
			
			bool bSuccess = LoadState(fName, true);

			if (!bSuccess)
			{
				RemoveFile(fName, false);
				GetAudioManager()->Play("audio/buzzer2.wav");
				PopUpCreate(pMenu, "Error loading save state.  Probably an older version, sorry.", "", "cancel", "Continue", "", "", true);
				//SyncPersistentData();

			} else
			{
				LoadStateWithExtra();
				PauseEnd(pMenu);
			}

		} else
		{
			//disabled, play buzzer sound
			GetAudioManager()->Play("audio/buzzer2.wav");
		}

		UpdatePauseButtons(pMenu);
			
	}
	
	//GetEntityRoot()->PrintTreeAsText(); //useful for Pause
}


void OnPauseArcadeInput(VariantList *pVList)
{
	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	//LogMsg("GameMenuArcade: Key %d, down is %d", vKey, int(bIsDown));

	Entity *pQuickSaveEnt = GetEntityRoot()->GetEntityByName("QuickSave");
	Entity *pQuickLoadEnt = GetEntityRoot()->GetEntityByName("QuickLoad");

	if (!EntityHasInputFocus(GetEntityRoot()->GetEntityByName("PauseMenu")))
	{
		return;
	}

	if (!bIsDown)
	{
		switch (vKey)
		{
			case VIRTUAL_KEY_DIR_LEFT:
				BobEntityStop(pQuickSaveEnt);
			break;

			case VIRTUAL_KEY_DIR_RIGHT:
				BobEntityStop(pQuickLoadEnt);
			break;

		}
	}
	
	int bobAmount =iPadMapY(40);
	int bobCycleMS = 300;

	if (bIsDown)
	{
		switch (vKey)
		{
			case VIRTUAL_KEY_DIR_LEFT:
				
				BobEntity(pQuickSaveEnt, bobAmount, 0, bobCycleMS);
				break;

			case VIRTUAL_KEY_DIR_RIGHT:
				BobEntity(pQuickLoadEnt, bobAmount, 0, bobCycleMS);
				break;

			case VIRTUAL_KEY_GAME_FIRE:
			case VIRTUAL_KEY_GAME_INVENTORY:
			case VIRTUAL_KEY_GAME_TALK:

				if ( IsEntityBobbing(pQuickSaveEnt) )
				{
					FakeClickAnEntity(pQuickSaveEnt);
				} else if (IsEntityBobbing(pQuickLoadEnt) )
				{
					FakeClickAnEntity(pQuickLoadEnt);
				} else
				{
					GetAudioManager()->Play("audio/buzzer2.wav");
				}
				break;
			default: ;
		}
	}
}

Entity * PauseMenuCreate(Entity *pParentEnt)
{
	
	//Entity *pBG = pParentEnt->AddEntity(new Entity("PauseMenu"));
	Entity * pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0,0), GetScreenSize(), MAKE_RGBA(0,0,0,140));
	pBG->SetName("PauseMenu");
	
	//so we can snag gamepad messages too:
	GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
	pBG->GetShared()->GetFunction("OnArcadeInput")->sig_function.connect(&OnPauseArcadeInput);


	Entity * pBackdrop = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0), CL_Vec2f(0,0), MAKE_RGBA(0,0,0,140));
	Entity *pTextBG = pBG->AddEntity(new Entity("PauseTextBG"));
	
	GetAudioManager()->Play("audio/pause_open.wav");

	pBackdrop->GetVar("pos2d")->Set(GetScreenSize()/2);
	pTextBG->GetVar("pos2d")->Set(GetScreenSize()/2+CL_Vec2f(0,iPhoneMapY(-30)));
	GetBaseApp()->SetGameTickPause(true);
	AddFocusIfNeeded(pBG, true);
	KillControls();

	g_dglo.m_bLastFullKeyboardActive = false;

	Entity *pButtonEntity;
	float x = 0;
	float y = iPhoneMapX(-70);
	float ySpacer = iPhoneMapY(40);
	eFont fontID = FONT_SMALL;
	float fontScale = 1;
	
	pButtonEntity = CreateTextButtonEntity(pBG , "Debug", iPhoneMapX(440), iPhoneMapY(20), "Cheats", false); 
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));

	//pButtonEntity->GetVar("color")->Set(MAKE_RGBA(0,0,0,0));

	//quick save/load buttons first

	pButtonEntity = CreateOverlayButtonEntity(pBG , "QuickSave", ReplaceWithLargeInFileName("interface/iphone/button_quicksave.rttex"), iPhoneMapX(85), GetScreenSizeYf()/2); 
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));

	if (G_ALLOW_SAVE_EXPORTING)
	{
		Entity *pButton;

		CL_Vec2f vEntSize = GetSize2DEntity(pButtonEntity);
		CL_Vec2f vEntPos = GetPos2DEntity(pButtonEntity);
		
		float spacer = 80;
		pButton = CreateTextButtonEntity(pBG, "ExportQuickSave", vEntPos.x, (vEntPos.y + vEntSize.y/2) +spacer, "(Export Quicksave to file)", true);
		SetAlignmentEntity(pButton, ALIGNMENT_DOWN_CENTER);
		pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);

		//next button
		pButton = CreateTextButtonEntity(pBG, "ExportNormalSave", vEntPos.x, (vEntPos.y + vEntSize.y / 2) + spacer+spacer, "(Export save slot to file)", true);
		SetAlignmentEntity(pButton, ALIGNMENT_DOWN_CENTER);
		pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);


	}
	
	pButtonEntity = CreateOverlayButtonEntity(pBG , "QuickLoad", ReplaceWithLargeInFileName("interface/iphone/button_quickload.rttex"), iPhoneMapX(395), GetScreenSizeYf()/2); 
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));


	if (G_ALLOW_SAVE_EXPORTING)
	{
		Entity *pButton;

		CL_Vec2f vEntSize = GetSize2DEntity(pButtonEntity);
		CL_Vec2f vEntPos = GetPos2DEntity(pButtonEntity);

		float spacer = 80;
		pButton = CreateTextButtonEntity(pBG, "ImportQuickSave", vEntPos.x, (vEntPos.y + vEntSize.y / 2) + spacer, "(Import Quicksave file)", true);
		SetAlignmentEntity(pButton, ALIGNMENT_DOWN_CENTER);
		pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
		SetButtonStyleEntity(pButton, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH); //to get around HTML5 rules on uploading, required because it's only
		//allowed when initiated by a user click
		
		//next button
		pButton = CreateTextButtonEntity(pBG, "ImportNormalSave", vEntPos.x, (vEntPos.y + vEntSize.y / 2) + spacer + spacer, "(Import save slot file)", true);
		SetAlignmentEntity(pButton, ALIGNMENT_DOWN_CENTER);
		pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
		SetButtonStyleEntity(pButton, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH); //to get around HTML5 rules on uploading, required because it's only
																					   //allowed when initiated by a user click


	}

	UpdatePauseButtons(pBG);
	
	pButtonEntity = CreateTextButtonEntity(pTextBG, "Map", x, y, "VIEW MAP", false); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	SetupTextEntity(pButtonEntity, fontID, fontScale);
	if (!DinkCanRunScriptNow())
	{
		pButtonEntity->GetVar("color")->Set(MAKE_RGBA(255,255,255,50));
	}

	pButtonEntity = CreateTextButtonEntity(pTextBG, "OldOptions", x, y, "DINK MENU", false); y += ySpacer;
	pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	SetupTextEntity(pButtonEntity, fontID, fontScale);

	if (!DinkCanRunScriptNow())
	{
		pButtonEntity->GetVar("color")->Set(MAKE_RGBA(255,255,255,50));
	}

	if (GetApp()->GetUsingTouchScreen())
	{
		pButtonEntity = CreateTextButtonEntity(pTextBG, "Keyboard", x, y, "FULL KEYBOARD", false); y += ySpacer;
		pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
		pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
		SetupTextEntity(pButtonEntity, fontID,fontScale);
	}

	pButtonEntity = CreateTextButtonEntity(pTextBG, "Quit", x, y, "QUIT", false); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	
	
	SetupTextEntity(pButtonEntity, fontID, fontScale);

	pButtonEntity = CreateTextButtonEntity(pTextBG, "Options", x, y, "OPTIONS", false); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	SetupTextEntity(pButtonEntity, fontID,fontScale);

	pButtonEntity = CreateTextButtonEntity(pTextBG, "Back", x, y, "CONTINUE", false); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&PauseMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	SetupTextEntity(pButtonEntity, fontID, fontScale);
	SetButtonClickSound(pButtonEntity, ""); //no sound

	EntityComponent *pKeys = AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);
	
	//work around problem of it instantly closing
	pKeys->GetVar("disabled")->Set(uint32(1));
	GetMessageManager()->SetComponentVariable(pKeys, 500, "disabled", uint32(0)); //enable it again

	if (IsDesktop() || GetEmulatedPlatformID() == PLATFORM_ID_HTML5)
	{
		EntityComponent *pKeys = AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_F1);
		//work around problem of it instantly closing
		pKeys->GetVar("disabled")->Set(uint32(1));
		GetMessageManager()->SetComponentVariable(pKeys, 500, "disabled", uint32(0)); //enable it again
	}



	/*
	pKeys = AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_PROPERTIES);	
	//work around problem of it instantly closing
	pKeys->GetVar("disabled")->Set(uint32(1));
	GetMessageManager()->SetComponentVariable(pKeys, 1, "disabled", uint32(0)); //enable it again
*/

	CL_Rectf size = MeasureEntityAndChildren(pTextBG);
	
	size.expand(iPhoneMapY(27));
	pBackdrop->GetVar("size2d")->Set(CL_Vec2f(size.get_width(), size.get_height()));
	pBackdrop->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	FadeInEntity(pBG, true, 450);
	return pBG;
}

