#include "PlatformPrecomp.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "DebugMenu.h"
#include "EnterNameMenu.h"
#include "ParticleTestMenu.h"
#include "TouchTestMenu.h"
#include "Entity/CustomInputComponent.h"
#include "AboutMenu.h"
#include "Renderer/SoftSurface.h"

void MainMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "ParticleTest")
	{
		//slide it off the screen and then kill the whole menu tree
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		ParticleTestCreate(pEntClicked->GetParent()->GetParent());
	}

	if (pEntClicked->GetName() == "InputTest")
	{
		//slide it off the screen and then kill the whole menu tree
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		EnterNameMenuCreate(pEntClicked->GetParent()->GetParent());
	}
	
	if (pEntClicked->GetName() == "TouchTest")
	{
		//slide it off the screen and then kill the whole menu tree
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		TouchTestMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	if (pEntClicked->GetName() == "Debug")
	{
		//overlay the debug menu over this one
		pEntClicked->GetParent()->RemoveComponentByName("FocusInput");
		DebugMenuCreate(pEntClicked->GetParent());
	}

	if (pEntClicked->GetName() == "About")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		
		//create the new menu
		AboutMenuCreate(pEntClicked->GetParent()->GetParent());
	}

	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

Entity * MainMenuCreate(Entity *pParentEnt)
{
	/*
	//Example of loading a jpg and saving out a bmp
	SoftSurface s;
	s.LoadFile("interface/cosmo.jpg", SoftSurface::COLOR_KEY_NONE);
	s.WriteBMPOut("cosmo.bmp");
	*/
	
	/*
	//Test of how measure text works
	rtRectf twolines;
	GetBaseApp()->GetFont(FONT_SMALL)->MeasureText( &twolines,"the top line is longer\nshorter" , 1);

	rtRectf singleline;
	GetBaseApp()->GetFont(FONT_SMALL)->MeasureText( &singleline, "the top line is longer", 1);

	LogMsg( string("Two lines rect: "+PrintRect(twolines)).c_str());
	LogMsg( string("Single line rect: "+PrintRect(singleline)).c_str());
	*/


	//Entity *pBG = CreateOverlayEntity(pParentEnt, "MainMenu", "interface/summary_bg.rttex", 0,0);
	Entity *pBG = pParentEnt->AddEntity(new Entity);
	
	AddFocusIfNeeded(pBG);
	
	//for android, so the back key (or escape on windows) will quit out of the game
	EntityComponent *pComp = pBG->AddComponent(new CustomInputComponent);
	//tell the component which key has to be hit for it to be activated
	pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));

	Entity *pButtonEntity;
	float x = 50;
	float y = 40;
	float ySpacer = 45;
	
	
	//If we wanted a rect color bg we could do the folowing
	//CreateOverlayRectEntity(pBG,GetScreenRect(),MAKE_RGBA(255,0,0,255));
	
	//let's add a background image to test the jpg loading
	CreateOverlayEntity(pBG, "Cosmo", "interface/cosmo.jpg",0,0);

	pButtonEntity = CreateTextButtonEntity(pBG, "ParticleTest", x, y, "ParticleTest"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "InputTest", x, y, "Text Input Test"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "TouchTest", x, y, "Multitouch Input Test"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "Debug", x, y, "Debug and music test"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	pButtonEntity = CreateTextButtonEntity(pBG, "About", x, y, "About (scroll bar test)"); y += ySpacer;
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&MainMenuOnSelect);

	SlideScreen(pBG, true);
	
	return pBG;
}

