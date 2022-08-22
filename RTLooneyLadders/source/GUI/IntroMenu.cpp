#include "PlatformPrecomp.h"
#include "IntroMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/CustomInputComponent.h"
#include "GUI/MainMenu.h"
#include "GUI/ControllerTestMenu.h"


void IntroMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = GetEntityRoot()->GetEntityByName("Intro");

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	DisableAllButtonsEntity(pMenu);
	SlideScreen(pEntClicked->GetParent(), false);
	//kill this menu entirely, but we wait half a second while the transition is happening before doing it
	GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 

	if (pEntClicked->GetName() == "Looney")
	{
		MainMenuCreate(pMenu->GetParent());
		return;
	}

	if (pEntClicked->GetName() == "Test")
	{
		ControllerTestMenuCreate(pMenu->GetParent());
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

Entity * IntroMenuCreate(Entity *pParentEnt)
{
	//Entity *pBG = CreateOverlayEntity(pParentEnt, "IntroMenu", "interface/title.rttex", 0,0);
	Entity *pBG = CreateOverlayRectEntity(pParentEnt, GetScreenRect(), MAKE_RGBA(0,0,0,255));
	pBG->SetName("Intro"); //so we can find this entiry again later


	//add buttons on top of this entity
	Entity *pEnt;

	pEnt = CreateOverlayButtonEntity(pBG, "Test", "interface/intro_button_controller_test.rttex", 80, 250);
	pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&IntroMenuOnSelect);
	BobEntity(pEnt);
	ZoomToPositionFromThisOffsetEntity(pEnt, CL_Vec2f(-500, 200), 2000);

	pEnt = CreateOverlayButtonEntity(pBG, "Looney", "interface/intro_button_looney.rttex", 550, 250);
	pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&IntroMenuOnSelect);
	BobEntity(pEnt);
	ZoomToPositionFromThisOffsetEntity(pEnt, CL_Vec2f(500, 200), 2000);

	pEnt = CreateTextLabelEntity(pBG, "Text", GetScreenSizeXf()/2, 30, "Choose your path!");
	SetupTextEntity(pEnt, FONT_LARGE, 2.0f);
	SetAlignmentEntity(pEnt, ALIGNMENT_UPPER_CENTER);

	AddFocusIfNeeded(pBG);
	
	//for android, so the back key (or escape on windows) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	//tell the component which key has to be hit for it to be activated
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	return pBG;
}
