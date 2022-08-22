#include "PlatformPrecomp.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"

#include "Entity/CustomInputComponent.h"
#include "GUI/AboutMenu.h"
#include "GUI/GameMenu.h"
#include "Entity/ArcadeInputComponent.h"
#include "Gamepad/GamepadManager.h"
#include "Gamepad/GamepadProvideriCade.h"
#include "GUI/IntroMenu.h"


void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	
	if (pEntClicked->GetName() == toString(TOTAL_LEVELS+1))
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		
		//create the new menu
		AboutMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	if (pEntClicked->GetName() == toString(TOTAL_LEVELS+2))
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);

		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 

		//create the intro menu again
		IntroMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}

	//default
	int levelsPassed =  GetApp()->GetVar("passed")->GetUINT32();

	if (  atoi(pEntClicked->GetName().c_str()) >levelsPassed+1)
	{
		GetAudioManager()->Play("audio/death.wav");
		return;
	}

	//if they clicked the mouse, we really do still need to set the level # for later
	GetApp()->GetVar("level")->Set(uint32(atoi(pEntClicked->GetName().c_str())));

	DisableAllButtonsEntity(pEntClicked->GetParent());
	SlideScreen(pEntClicked->GetParent(), false);

	//kill this menu entirely, but we wait half a second while the transition is happening before doing it
	GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		//create the new menu
	GameMenuCreate(pEntClicked->GetParent()->GetParent());
	
	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


void SelectionMove(bool bDown)
{
	Entity *pIcon = GetEntityRoot()->GetEntityByName("SelectIcon");
	uint32 selection = pIcon->GetVar("curSelection")->GetUINT32();
	uint32 itemCount = pIcon->GetVar("itemCount")->GetUINT32();
	
	if (bDown )
	{
		if (selection < itemCount)
		selection++;
	} else
	{
		if (selection > 1)
			selection--;
	}

	 pIcon->GetVar("curSelection")->Set(selection);

	Entity *pActiveSel = pIcon->GetParent()->GetEntityByName(toString(selection));

	 CL_Vec2f vPos = pActiveSel->GetVar("pos2d")->GetVector2()+CL_Vec2f(-60, 0);
	 pIcon->GetVar("pos2d")->Set(vPos);

	 GetAudioManager()->Play("audio/click.wav");

	LogMsg("Chose %d", selection);
}


void SelectionSelect()
{

	Entity *pIcon = GetEntityRoot()->GetEntityByName("SelectIcon");
	int selection = pIcon->GetVar("curSelection")->GetUINT32();
	LogMsg("Chose %d", selection);
	
	Entity* pTargetButton = pIcon->GetParent()->GetEntityByName(toString(selection));
	
	if (IsDisabledEntity(pTargetButton))
	{
		return;
	}

	SendFakeButtonPushToEntity(pTargetButton, 20);
	GetApp()->GetVar("level")->Set(uint32(selection));
}

void UnlockAllLevels()
{
	GetApp()->GetVar("passed")->Set(uint32(TOTAL_LEVELS));
}

void OnSelectInput(VariantList *pVList)
{
	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	if (!bIsDown) return;

	switch(vKey)
	{
	case 13:
		SelectionSelect();
		break;

/*
	case 'U':
		UnlockAllLevels();
		break;
*/

	case VIRTUAL_KEY_DIR_UP:
		SelectionMove(false);
	break;

	case VIRTUAL_KEY_DIR_DOWN:
		SelectionMove(true);
		break;
	}

}

void SetupArrowSelector(Entity *pBG, int itemCount, uint32 defaultItem)
{
	//first make the icon that will show the current selection
	Entity *pIcon = CreateOverlayRectEntity(pBG, CL_Vec2f(0,0),CL_Vec2f(30,30),MAKE_RGBA(180,0,0,180));
	pIcon->SetName("SelectIcon");

	ArcadeInputComponent *pComp = (ArcadeInputComponent*)pIcon->AddComponent(new ArcadeInputComponent);

	/*
	//connect to a gamepad/joystick too if available
	
	*/

	//Just connect to all pads at once.. ok to do for a single player game.. otherwise, we should use
	//Gamepad *pPad = GetGamepadManager()->GetDefaultGamepad(); instead probably, or let the user choose.
	//Keep in mind pads can be removed/added on the fly

	AttachGamepadsIfPossible();


	AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
	AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
	AddKeyBinding(pComp, "Enter", 13, 13);
	AddKeyBinding(pComp, "Enter2", VIRTUAL_KEY_CONTROL, 13);
	AddKeyBinding(pComp, "Unlock", 'U', 'U');
	AddKeyBinding(pComp, "XPeriaStart", VIRTUAL_DPAD_START, 13); 
	AddKeyBinding(pComp, "XPeriaSelect", VIRTUAL_DPAD_SELECT, VIRTUAL_KEY_DIR_DOWN); 
	AddKeyBinding(pComp, "XperiaX", VIRTUAL_KEY_DIR_CENTER, 13); //for experia plays X button
	AddKeyBinding(pComp, "XperiaO", VIRTUAL_DPAD_BUTTON_RIGHT, 13); //for xperia O button 
	AddKeyBinding(pComp, "Select1", VIRTUAL_DPAD_BUTTON_DOWN, 13); //for an icade button
	AddKeyBinding(pComp, "Select2", VIRTUAL_DPAD_BUTTON_LEFT, 13); //for another icade button
    AddKeyBinding(pComp, "Select3", VIRTUAL_DPAD_BUTTON_UP, 13); //for another icade button
    AddKeyBinding(pComp, "Select4", VIRTUAL_DPAD_RTRIGGER, 13);  //for another icade button
    

	PulsateColorEntity(pIcon, false, MAKE_RGBA(60,0,0,130));

	//route signals directly from the ArcadeInputComponent to our main menu parent, pBG
	VariantList vList(pBG);
	pComp->GetFunction("SetOutputEntity")->sig_function(&vList); //redirect its messages to our parent entity, will call OnArcadeInput
	pBG->GetFunction("OnArcadeInput")->sig_function.connect(&OnSelectInput);	

	//another way is don't tell it to route anywhere, it will default to using GetBaseApp()->m_sig_arcade_input instead..
	//Because I don't want to stay connected after this menu dies, one way is to connect to an entity, then connect the
	//entity to my static OnSelectInput... this way when the entity dies, input won't still get routed there.
    //GetBaseApp()->m_sig_arcade_input.connect(pBG->GetFunction("OnArcadeInput")->sig_function);
    //pBG->GetFunction("OnArcadeInput")->sig_function.connect(&OnSelectInput);
	
	if (defaultItem > (uint32)itemCount || defaultItem < 1) defaultItem = 1;

	pIcon->GetVar("itemCount")->Set(uint32(itemCount));
	pIcon->GetVar("curSelection")->Set(uint32(defaultItem));

	Entity *pActiveSel = pBG->GetEntityByName(toString(defaultItem));
	CL_Vec2f vPos = pActiveSel->GetVar("pos2d")->GetVector2()+CL_Vec2f(-60, 0);
	pIcon->GetVar("pos2d")->Set(vPos);
}

Entity * MainMenuCreate(Entity *pParentEnt)
{
	
	GetAudioManager()->Play("audio/title.ogg", false, true);
	Entity *pBG = CreateOverlayEntity(pParentEnt, "MainMenu", "interface/title.rttex", 0,0);
	
	AddFocusIfNeeded(pBG);
	
	//for android, so the back key (or escape on windows) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	//tell the component which key has to be hit for it to be activated
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	Entity *pButtonEntity;
	float x = 130;
	float y = 400;
	float ySpacer = 45;


	/*
	//for testing input..
	CreateInputTextEntity(pBG, "Crap", 20,40, "Test");
	CreateInputTextEntity(pBG, "Crap", 20,80, "Test2");
	*/
	
	int levelsPassed = GetApp()->GetVar("passed")->GetUINT32();

	for (int i=1; i <= TOTAL_LEVELS; i++)
	{
		string descrip;
		switch (i)
		{
		case 1: descrip = "Residential home"; break;
		case 2: descrip = "Small apartment"; break;
		case 3: descrip = "Short 'n wide"; break;
		case 4: descrip = "Skyrise"; break;
		case 5: descrip = "No ladders"; break;
		case 6: descrip = "Metropolis"; break;
	}
		
		string title = string("Level ")+toString(i) + " - "+descrip;

		if (levelsPassed+1 < i)
		{
			title += " `4(LOCKED)";
		}
		pButtonEntity = CreateTextButtonEntity(pBG, toString(i), x, y, title); y += ySpacer;
		pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);
	}

    pButtonEntity = CreateTextButtonEntity(pBG, toString(TOTAL_LEVELS+1), x, y, "About"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, toString(TOTAL_LEVELS+2), x, y, "Back"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	SetupArrowSelector(pBG, TOTAL_LEVELS+2, GetApp()->GetVar("level")->GetUINT32());

	SlideScreen(pBG, true);
	return pBG;
}
