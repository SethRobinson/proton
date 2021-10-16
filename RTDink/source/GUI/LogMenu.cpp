#include "PlatformPrecomp.h"
#include "LogMenu.h"
#include "Entity/EntityUtils.h"
#include "../dink/dink.h"
#include "MainMenu.h"

void LogEnd(Entity *pMenu)
{
	//slide it off the screen and then kill the whole menu tree
	RemoveFocusIfNeeded(pMenu);
	//SlideScreen(pEntClicked->GetParent(), false);
	AddFocusIfNeeded(pMenu->GetParent());
	FadeOutEntity(pMenu, true, 499);
	GetMessageManager()->CallEntityFunction(pMenu, 500, "OnDelete", NULL);
}

void LogMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();

	LogMsg("Clicked %s entity", pEntClicked->GetName().c_str());

	Entity *pMenu = GetEntityRoot()->GetEntityByName("LogMenu");

	if (pEntClicked->GetName() == "Back")
	{
		LogEnd(pMenu);
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for Log
}



void LogAddScrollContent(Entity *pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	float x = iPhoneMapX(5);
	float y = 0;

	
	CL_Vec2f vTextBoxPos(iPhoneMapX(20),y);
	CL_Vec2f vTextBounds(iPhoneMapX(434), iPhoneMapY(200));
	string msg = GetBaseApp()->GetConsole()->GetAsSingleString();

	CreateTextBoxEntity(pParent, "", vTextBoxPos, vTextBounds, msg, 0.7f);
	VariantList vList(pParent->GetParent());
    ResizeScrollBounds(&vList);
}


Entity * LogMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayRectEntity(pParentEnt, CL_Vec2f(0,0), GetScreenSize(), MAKE_RGBA(0,0,0,140));
	
	pBG->SetName("LogMenu");
	AddFocusIfNeeded(pBG, true);
	
	Entity *pButtonEntity;

	//setup a scrolling window

	CL_Vec2f vTextAreaPos = CL_Vec2f(0,0);
	float offsetFromBottom = iPhoneMapY(30);

	CL_Vec2f vTextAreaBounds = GetScreenSize()-CL_Vec2f(15,offsetFromBottom);
	Entity *pScroll = pBG->AddEntity(new Entity("scroll"));
	pScroll->GetVar("size2d")->Set(vTextAreaBounds);
	pScroll->AddComponent(new TouchHandlerComponent);

	EntityComponent *pFilter = pScroll->AddComponent(new FilterInputComponent);

	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	pScrollComp->GetVar("fingerTracking")->Set(uint32(1));

	EntityComponent *pClip = pScroll->AddComponent(new RenderClipComponent);
	pClip->GetVar("clipMode")->Set(uint32(RenderClipComponent::CLIP_MODE_BOTTOM));

	//Entity *pOverlay = CreateOverlayEntity(pBG, "", ReplaceWithDeviceNameInFileName("interface/iphone/bg_stone_overlay.rttex"), 0, GetScreenSizeYf()); 
	//SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);

	LogAddScrollContent(pBG);

    VariantList vList(CL_Vec2f(0.0f, 1.0f));
	pScrollComp->GetFunction("SetProgress")->sig_function(&vList); //scroll to the end

	pButtonEntity = CreateTextButtonEntity(pBG, "Back", iPhoneMapX(35), iPhoneMapY(300), "CONTINUE", false);
	pButtonEntity->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&LogMenuOnSelect);
	pButtonEntity->GetVar("alignment")->Set(uint32(ALIGNMENT_CENTER));
	AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);

	FadeInEntity(pBG, true);
	return pBG;
}

