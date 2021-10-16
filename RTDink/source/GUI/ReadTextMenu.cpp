#include "PlatformPrecomp.h"
#include "ReadTextMenu.h"
#include "MainMenu.h"
#include "GameMenu.h"
#include "Entity/EntityUtils.h"
#include "../dink/dink.h"
#include "PopUpMenu.h"
#include "util/TextScanner.h"
#include "DMODMenu.h"



void ReadTextMenuAddScrollContent(Entity *pParent, TextScanner &t);


void ReadTextMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("ReadTextMenu");

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pMenu);	
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		DMODMenuCreate(pEntClicked->GetParent()->GetParent(), true);
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


void ReadTextMenuAddScrollContent(Entity *pParent)
{
	TextScanner t(pParent->GetVar("textfile")->GetString(), false);

	pParent = pParent->GetEntityByName("scroll_child");

	pParent->RemoveAllEntities();
	float x = 5;
	float y = 0;

	//Entity *pEnt;

	CL_Vec2f vTextBoxPos(iPhoneMapX(20),iPhoneMapY(y));
	CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
	

	CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, t.GetAll());
	
    VariantList vList(pParent->GetParent());
	ResizeScrollBounds(&vList);
}


void ReadTextOnPostIntroTransition(VariantList *pVList)
{
	Entity *pBG = pVList->Get(0).GetEntity();
	ReadTextMenuAddScrollContent(pBG);
}

Entity * ReadTextMenuCreate( Entity *pParentEnt, string fileName, string prettyFileName )
{
	
	GetBaseApp()->ClearError();

	Entity *pBG = NULL;
	pBG =  CreateOverlayEntity(pParentEnt, "ReadTextMenu", ReplaceWithDeviceNameInFileName("interface/iphone/bkgd_stone.rttex"), 0,0);
	AddFocusIfNeeded(pBG, true, 500);
	pBG->AddComponent(new FocusRenderComponent);
	pBG->GetVar("textfile")->Set(fileName);
		
	CL_Vec2f vTextAreaPos = iPhoneMap(2,10);
	float offsetFromBottom = iPhoneMapY(42);
	float offsetFromRight = iPhoneMapY(0);

	CL_Vec2f vTextAreaBounds = (GetScreenSize()- CL_Vec2f(offsetFromRight,offsetFromBottom))-vTextAreaPos;
	Entity *pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("pos2d")->Set(vTextAreaPos);
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	
	Entity *pLabel = CreateTextLabelEntity(pBG, "Reading "+prettyFileName, GetScreenSizeXf()/2, GetScreenSizeYf()/2, "Updating add-on ReadText list...");
	SetAlignmentEntity(pLabel, ALIGNMENT_CENTER);
	FadeOutAndKillEntity(pLabel, true, 300, 501);


	EntityComponent *pFilter = pScroll->AddComponent(new FilterInputComponent);

	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	//EntityComponent *pClip = pScroll->AddComponent(new RenderClipComponent);
	//pClip->GetVar("clipMode")->Set(uint32(RenderClipComponent::CLIP_MODE_BOTTOM));

	Entity *pOverlay = CreateOverlayEntity(pBG, "", ReplaceWithDeviceNameInFileName("interface/iphone/bg_stone_overlay.rttex"), 0, GetScreenSizeYf()); 
	SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);


	//	ZoomFromPositionEntity(pBG, CL_Vec2f(0, -GetScreenSizeYf()), 500);
	//the continue button
	Entity *pEnt;

	//pEnt = CreateOverlayRectEntity(pBG, CL_Rectf(0, GetScreenSizeYf()-offsetFromBottom, GetScreenSizeXf(), 320), MAKE_RGBA(0,0,0,100));

	eFont fontID = FONT_SMALL;

	pEnt = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(40), iPhoneMapY(BACK_BUTTON_Y), "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&ReadTextMenuOnSelect);
	SetupTextEntity(pEnt, fontID);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK);

	SlideScreen(pBG, true, 500);
	pBG->GetFunction("OnPostIntroTransition")->sig_function.connect(&ReadTextOnPostIntroTransition);
	VariantList vList(pBG, string(""));
    GetMessageManager()->CallEntityFunction(pBG, 500, "OnPostIntroTransition", &vList); 

	return pBG;
}