#include "PlatformPrecomp.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "MainMenu.h"
#include "Entity/DPadComponent.h"
#include "Entity/SelectButtonWithCustomInputComponent.h"
#include "Entity/ArcadeInputComponent.h"
#include "Gamepad/GamepadManager.h"
#include "Gamepad/GamepadProvideriCade.h"

#include "Component/CharComponent.h"
#include "Component/CharManagerComponent.h"

void ShowQuickMessage(string msg)
{
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	assert(pMenu);
	if (!pMenu) return;
	
	KillEntity(pMenu->GetEntityByName("GameMsg"), 0);

	Entity *pEnt = CreateTextLabelEntity(pMenu, "GameMsg", GetScreenSizeXf()/2, iPhoneMapY(100), msg);

	//SetupTextEntity(pEnt, FONT_LARGE);
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
	FadeInEntity(pEnt);
	FadeOutAndKillEntity(pEnt, true, 1000, 1000);
}

void ShowTutorialMessage(string msg, int timeMS, int timeToShow)
{
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	assert(pMenu);
	if (!pMenu) return;

	
	Entity *pEnt = CreateTextLabelEntity(pMenu, "TutMsg", GetScreenSizeXf()/2, iPhoneMapY(50), msg);

	//SetupTextEntity(pEnt, FONT_LARGE);
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
	SetAlphaEntity(pEnt, 0);
	FadeEntity(pEnt, true, 1, 500, timeMS);
	FadeEntity(pEnt, true, 0, 500, timeMS+timeToShow, true);
	KillEntity(pEnt, timeMS+timeToShow+500);
}

void ShowBigMessage(string msg)
{
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");
	assert(pMenu);
	if (!pMenu) return;

	KillEntity(pMenu->GetEntityByName("BigGameMsg"), 0);
	Entity *pEnt = CreateTextLabelEntity(pMenu, "BigGameMsg", GetScreenSizeXf()/2, iPhoneMapY(100), msg);

	SetupTextEntity(pEnt, FONT_LARGE);
	SetAlignmentEntity(pEnt, ALIGNMENT_CENTER);
	
	ZoomToPositionFromThisOffsetEntity(pEnt, CL_Vec2f(0, -GetScreenSizeYf()/2), 2000);
	FadeInEntity(pEnt);
	FadeOutAndKillEntity(pEnt, true, 1000, 5000);
}

void GameMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = GetEntityRoot()->GetEntityByName("GameMenu");

	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		pEntClicked->GetParent()->SetName("GameMenuAboutToBeDeleted"); //so a search by name won't work on it now
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 0, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}
	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void OnArcadeInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	//LogMsg("GameMenuArcade: Key %d, down is %d", vKey, int(bIsDown));

		switch(vKey)
		{
		case VIRTUAL_KEY_DIR_LEFT:
			GetPlayer()->Move(false, bIsDown);
			break;

		case VIRTUAL_KEY_DIR_RIGHT:
			GetPlayer()->Move(true, bIsDown);
			break;

		case VIRTUAL_KEY_DIR_UP:
			if (bIsDown)
			GetPlayer()->UpLadder();
			break;

		case VIRTUAL_KEY_DIR_DOWN:
			if (bIsDown)
			GetPlayer()->DownLadder();
			break;
		case VIRTUAL_KEY_GAME_FIRE:
			if (bIsDown)
				GetPlayer()->OnUse();
			break;
	
		}
	
}

void OnGotDoor()
{
    Entity *pEnt = GetEntityRoot()->GetEntityByName("GameMenu")->GetEntityByName("DoorsLeft");
    EntityComponent *pTextComp = pEnt->GetComponentByName("TextRender");
    
    string text = pTextComp->GetVar("text")->GetString();
    int numLeft = atoi(text.c_str())-1;
    if (numLeft == 0)
    {
        ShowBigMessage("YOU PASSED THE LEVEL!");
        GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_MUSIC, "audio/win.ogg", 1000);
        
        FakeClickAnEntity(GetEntityRoot()->GetEntityByName("Back"), 6000);
        LogMsg("Won");
        uint32 passed = GetApp()->GetVar("passed")->GetUINT32();
        passed = rt_max(passed, GetApp()->GetVar("level")->GetUINT32());
        GetApp()->GetVar("passed")->Set(uint32(passed));
    }
    
    pTextComp->GetVar("text")->Set(toString(numLeft));
}

int OnModLivesLost(int mod)
{
	Entity *pEnt = GetEntityRoot()->GetEntityByName("GameMenu")->GetEntityByName("LivesLeft");
	EntityComponent *pTextComp = pEnt->GetComponentByName("TextRender");

	string text = pTextComp->GetVar("text")->GetString();
	int numLeft = atoi(text.c_str())+mod;
	
	if (numLeft == 0)
	{
		ShowBigMessage("OUT OF LIVES.  YOU FAIL!");
		FakeClickAnEntity(GetEntityRoot()->GetEntityByName("Back"), 6000);
	}

	pTextComp->GetVar("text")->Set(toString(numLeft));

	return numLeft;
}

int OnModDyno(int mod)
{
	Entity *pEnt = GetEntityRoot()->GetEntityByName("GameMenu")->GetEntityByName("DynoLeft");
	EntityComponent *pTextComp = pEnt->GetComponentByName("TextRender");
	string text = pTextComp->GetVar("text")->GetString();
	int numLeft = atoi(text.c_str())+mod;
	pTextComp->GetVar("text")->Set(toString(numLeft));

	return numLeft;
}

Entity * GameMenuCreate(Entity *pParentEnt)
{

	Entity *pBG = pParentEnt->AddEntity(new Entity("GameMenu"));
	AddFocusIfNeeded(pBG);
	
	//arcade input component is a way to tie keys/etc to send signals through GetBaseApp()->m_sig_arcade_input

	ArcadeInputComponent *pComp = (ArcadeInputComponent*) pBG->AddComponent(new ArcadeInputComponent);
	
	/*
	//connect to a gamepad/joystick too if available
	
	*/

	//Just connect to all pads at once.. ok to do for a single player game.. otherwise, we should use
	//Gamepad *pPad = GetGamepadManager()->GetDefaultGamepad(); instead probably, or let the user choose.
	//Keep in mind pads can be removed/added on the fly

	AttachGamepadsIfPossible();

	//these arrow keys will be triggered by the keyboard and gamepads if applicable
	AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
	AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
	AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "Dynamite", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);

	//for xperia play on android
	//AddKeyBinding(pComp, "XperiaA", VIRTUAL_KEY_DIR_CENTER, VIRTUAL_KEY_GAME_FIRE); //for experia plays X button

	//VIRTUAL_DPAD_BUTTON_LEFT doesn't mean pushing left on the dpad, it means the button on the left.  (Like, the X button
	//on a 360 controller.  It makes more sense to use a direction for a button than BLUE, X or A because the button names change
	//too much dependong the controller)

	AddKeyBinding(pComp, "XPeriaB", VIRTUAL_DPAD_BUTTON_DOWN, VIRTUAL_KEY_GAME_FIRE); //for experia plays O button
	AddKeyBinding(pComp, "Dynamite2", VIRTUAL_DPAD_BUTTON_LEFT, VIRTUAL_KEY_GAME_FIRE); //map another icade button 
    AddKeyBinding(pComp, "Dynamite3", VIRTUAL_DPAD_BUTTON_RIGHT, VIRTUAL_KEY_GAME_FIRE); //map another icade button 
    AddKeyBinding(pComp, "Dynamite4", VIRTUAL_DPAD_BUTTON_UP, VIRTUAL_KEY_GAME_FIRE); //map another icade button 
    
    AddKeyBinding(pComp, "Back", VIRTUAL_DPAD_START, VIRTUAL_KEY_BACK, true);
    
	GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
	pBG->GetShared()->GetFunction("OnArcadeInput")->sig_function.connect(&OnArcadeInput);

	//setup level
	Entity *pLevel = pBG->AddEntity(new Entity("Level"));
	EntityComponent *pBuilding = pLevel->AddComponent(new BuildingComponent);
	
	uint32 floors = 5;
	uint32 cellsPerFloor = 10;
	uint32 elevators = 0;
	float ladderOddsPerFloor = 1.0f;
	uint32 extraLadders = 2;
	uint32 walls = 4;
	uint32 doors = 8;
	CL_Vec2f scale2d = CL_Vec2f(0.7f,0.7f);
	uint32 dynamite = 3;
	uint32 lives = 3;
	int level = GetApp()->GetVar("level")->GetUINT32();

	uint32 eguys = 0;
	uint32 mguys = 0;
	uint32 hguys = 0;

	GetAudioManager()->StopMusic();
	
	switch(level)
	{
	case 1:
		eguys = 4;
	   	ShowTutorialMessage("Use arrow keys to move.", 1000, 4500);
		if (IsDesktop())
		{
			ShowTutorialMessage("Press CTRL to place dynamite. (and then run away!)", 6000, 4000);
		} else
		{
			ShowTutorialMessage("Tap dynamite icon to place it. (and then run away!)", 6000, 4000);

		}
		ShowTutorialMessage("Rescue all girls to win.", 10000, 4000);
		ShowTutorialMessage("Girls sometimes give dynamite and extra lives.", 14000, 4000);

		break;

	case 2:
		ShowTutorialMessage("You cannot be hurt while climbing ladders or using elevators.", 2000, 4000);
		ShowTutorialMessage("Levels are randomly generated each time.", 7000, 4000);
		floors = 6;
		elevators = 0;
		walls = 6;
		ladderOddsPerFloor = 1.3f;
		doors = 10;
		eguys = 10;
		break;
		
	case 3:
	
		floors = 4;
		cellsPerFloor = 20;
		elevators = 0;
		walls = 10;
		//ladderOddsPerFloor = 0;
		doors = 16;
		eguys = 10;
		extraLadders = 10;
		scale2d = CL_Vec2f(1,1);
		break;

	case 4:
		floors = 12;
		elevators = 0;
		walls = 8;
		//ladderOddsPerFloor = 0;
		doors = 12;
		eguys = 25;
		doors = 25;	
		break;

	case 5:
		floors = 5;
		cellsPerFloor =10;
		elevators = 0;
		walls = 8;
		ladderOddsPerFloor = 0;
		doors = 10;
		eguys = 10;
		extraLadders = 0;
		scale2d = CL_Vec2f(0.7f,0.7f);
		break;

	case 6:
	floors = 12;
		elevators = 0;
		walls = 20;
		cellsPerFloor = 16;
		ladderOddsPerFloor = 1;
		scale2d = CL_Vec2f(0.4f,0.4f);
		doors = 30;
		extraLadders = 10;
		eguys = 50;
	break;
	}
	
	pLevel->GetVar("floors")->Set(floors);
	pLevel->GetVar("eguys")->Set(eguys);
	pLevel->GetVar("emuys")->Set(mguys);
	pLevel->GetVar("hguys")->Set(hguys);
	pLevel->GetVar("cellsPerFloor")->Set(cellsPerFloor);
	pLevel->GetVar("elevators")->Set(elevators); //doesn't include required ones to make the level solvable
	pLevel->GetVar("extraLadders")->Set(extraLadders);
	pLevel->GetVar("walls")->Set(walls);
	pLevel->GetVar("doors")->Set(doors);
	pLevel->GetVar("scale2d")->Set(scale2d);
	pLevel->GetVar("ladderOddsPerFloor")->Set(ladderOddsPerFloor);
	pLevel->GetFunction("BuildLevel")->sig_function(NULL);
	
	//add player
	pLevel->AddComponent(new CharManagerComponent);

	//pLevel->GetVar("pos2d")->Set(CL_Vec2f(0, 128*3));
	GetBuilding()->SetupCharacters();

	//setup game GUI
	Entity * pEnt = NULL;

	if (!IsDesktop())
	{
		//optional onscreen DPAD - it will send messages like VIRTUAL_KEY_DIR_UP to anyone listening to GetBaseApp()->m_sig_arcade_input,
		//the best part is those are the exact same signals sent for the keyboard arrows as well, so it makes multiplatform controls
		//simple to do

		Entity *pDPADEnt = pBG->AddEntity(new Entity("DPAD"));
		EntityComponent *pPadComp = pDPADEnt->AddComponent(new DPadComponent);

		//move it to a good place on the screen
		pDPADEnt->GetVar("pos2d")->Set(CL_Vec2f(90,GetScreenSizeYf()-90));

		if (GetPrimaryGLX() < 1024 && GetPrimaryGLY() < 1024) //this is a way to get the TRUE screensize, not the one we're faking (which is scaled to the true size)
		{
			//you know what? it's probably a phone.  Let's make the arrow keypad bigger by scaling just it up.
			pDPADEnt->GetVar("scale2d")->Set(CL_Vec2f(2,2)); //it will scale itself and the dpad/cliprects for us

			//better positioning for it
			pDPADEnt->GetVar("pos2d")->Set(CL_Vec2f(180,GetScreenSizeYf()-180));
		}

		//dynamite button
		pEnt = CreateOverlayButtonEntity(pBG, "dynamite", "interface/dynamite_button.rttex", GetScreenSizeXf()-110, GetScreenSizeYf()- 100);
		MakeButtonEmitVirtualGameKey(pEnt, VIRTUAL_KEY_GAME_FIRE);
	}

	//invisible back button
	pEnt = CreateTextButtonEntity(pBG, "Back", -100, GetScreenSizeYf()-30, "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&GameMenuOnSelect);
	SetupTextEntity(pEnt, FONT_SMALL);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK); //for androids back button and window's Escape button

	if (!IsDesktop())
	{
		//actually, make it visible, we need it on touchscreens.  Let's locate it on screen.  Top right?
		pEnt->GetVar("pos2d")->Set(CL_Vec2f(GetScreenSizeXf()-80, 8));
	}

	//make it more readable
	pEnt = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0), CL_Vec2f(GetScreenSizeXf(), 100), MAKE_RGBA(0,0,0,100));
    
	//doors left icon
	pEnt = CreateOverlayEntity(pBG, "DoorIcon", "game/building_overlays.rttex", 0, -70);
	SetupAnimEntity(pEnt, 6, 1, 5, 0);
	//SetAlphaEntity(pEnt, 0.5f);
	pEnt->GetVar("scale2d")->Set(CL_Vec2f(1,1));

	//doors left text
	pEnt = CreateTextLabelEntity(pBG, "DoorsLeft", 88, 10, toString(doors));
	SetupTextEntity(pEnt, FONT_LARGE, 1.6f);

	//dynamite left icon
	pEnt = CreateOverlayEntity(pBG, "DynoIcon", "game/items.rttex", 200, 6);
	SetupAnimEntity(pEnt, 8, 1, 0, 0);
	//SetAlphaEntity(pEnt, 0.5f);
	pEnt->GetVar("scale2d")->Set(CL_Vec2f(1,1));

	//dyno left text
	pEnt = CreateTextLabelEntity(pBG, "DynoLeft", 260, 10, toString(dynamite));
	SetupTextEntity(pEnt, FONT_LARGE, 1.6f);

	//lives left icon
	pEnt = CreateOverlayEntity(pBG, "LivesIcon", "game/hero.rttex", 400, 0);
	SetupAnimEntity(pEnt, 8, 1, 0, 0);
	//SetAlphaEntity(pEnt, 0.5f);
	pEnt->GetVar("scale2d")->Set(CL_Vec2f(0.6f,0.6f));

	//lives left text
	pEnt = CreateTextLabelEntity(pBG, "LivesLeft", 460, 10, toString(lives));
	SetupTextEntity(pEnt, FONT_LARGE, 1.6f);

	SlideScreen(pBG, true);
	return pBG;
}

