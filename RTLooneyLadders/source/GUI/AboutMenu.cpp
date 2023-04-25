#include "PlatformPrecomp.h"
#include "AboutMenu.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"


void AboutMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("AboutMenu"); //we're sort of cheating by just grabbing the top level parent
	//entity by name instead of GetParent() a bunch of times to reach the top level, but meh
	
	
	if (pEntClicked->GetName() == "Back")
	{
		DisableAllButtonsEntity(pEntClicked->GetParent());
		SlideScreen(pEntClicked->GetParent(), false);
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL);
		MainMenuCreate(pEntClicked->GetParent()->GetParent());
		return;
	}



	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void AboutMenuAddScrollContent(Entity *pParent)
{
	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities(); //clear it out in case we call this more than once, say, to update/change something

	float x = 5; //inset
	float y = 0;
	float spacerY = 30; //space between thingies

	//first, a title in a big font
	Entity *pTitle = CreateTextLabelEntity(pParent, "Title", x, 0, "About"); 
	SetupTextEntity(pTitle, FONT_LARGE);
	y += pTitle->GetVar("size2d")->GetVector2().y +spacerY;

	//define an area for a text box, so it will wrap in  the right places.  Height is actually ignored.
	CL_Vec2f vTextBoxPos(x,y);
	CL_Vec2f vTextBounds(900, 700);
	string msg; //we could load text, but let's just manually put junk in there:

	msg += \
		"Looney Laddders V1.00 - A 48 hour game by Seth A. Robinson for LD19\n"\
		"\nFor more questionable/odd games visit `wrtsoft.com``.\n" \
	"This game was built with the Proton SDK - www.protonsdk.com\n" \
		"\n(c) 2010 Seth A. Robinson\n"\
		"\nPress ENTER to continue";

		//actually create the text box with the above text

	Entity *pEnt = CreateTextBoxEntity(pParent, "SomeTextBox", vTextBoxPos, vTextBounds, msg);
	y += pEnt->GetVar("size2d")->GetVector2().y; //move our Y position down the exact size of the text
	y += spacerY; //don't forget our spacer

	
	//automatically calculate the total size of this entity with all its children for the scroll bars, do this at the end

	VariantList vList(pParent->GetParent());
    ResizeScrollBounds(&vList);
}


Entity * AboutMenuCreate( Entity *pParentEnt)
{
	Entity *pBG = NULL;
	pBG =  CreateOverlayEntity(pParentEnt, "AboutMenu", "interface/bg.rttex", 0,0);
	AddFocusIfNeeded(pBG, true, 500);
	pBG->AddComponent(new FocusRenderComponent);

	//setup the dimensions of where the scroll area will go
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
	
	//note: If you don't want to see a scroll bar progress indicator, comment out the next line.  Also note that it only draws
	//a vertical progress bar if needed but doesn't draw a horizontal if needed (I just haven't needed a horizontal scroll bar yet)
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//also let's add a visual way to see the scroller position
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	
	//but the nicest way is to blit a matching bar at the bottom with transparency:
	Entity *pOverlay = CreateOverlayEntity(pBG, "", "interface/bg_overlay.rttex", 0, GetScreenSizeYf()+1); 
	SetAlignmentEntity(pOverlay, ALIGNMENT_DOWN_LEFT);
	
	//actually add all our content
	AboutMenuAddScrollContent(pBG);


	//oh, let's put the Back button on the bottom bar thing
	Entity * pEnt = CreateTextButtonEntity(pBG, "Back", 20, GetScreenSizeYf()-30, "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	SetupTextEntity(pEnt, FONT_SMALL);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK); //for androids back button and window's Escape button
	AddHotKeyToButton(pEnt, 13); //heck, let enter trigger this too

	SlideScreen(pBG, true, 500);
	return pBG;
}