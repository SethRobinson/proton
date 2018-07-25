#include "PlatformPrecomp.h"
#include "AboutMenu.h"
#include "MainMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/RenderScissorComponent.h"

void AboutMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->Get(1).GetEntity();
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

	if (pEntClicked->GetName() == "plogo")
	{
		//Let's add an entity to the scroll window saying something
		CL_Vec2f vPos = pEntClicked->GetVar("pos2d")->GetVector2();
		
		//add a text message that types itself onto the screen.  Slightly randomize its position
		Entity *pEnt = CreateTextLabelEntity(pEntClicked->GetParent(), "tempText", vPos.x+170, vPos.y+50+RandomRangeFloat(-50,50), "`$You `#clicked`` the logo!");
		TypeTextLabelEntity(pEnt, 0, 50); //make it type out the text
		FadeOutAndKillEntity(pEnt, true, 500, 3000);

		OneTimeBobEntity(pEntClicked, -20, 5, 250);
		//let's have the logo bob as well, I mean, it's easy to do

		return;
	}

	if (pEntClicked->GetName() == "twitter")
	{
		LaunchURL("https://twitter.com/rtsoft");
		return;
	}


	GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}

void AboutMenuAddScrollContent(Entity *pParent)
{
	//here we add our actual content we want scrolled.  At the end, we'll calculate the size used using ResizeScrollBounds and the scroll bars
	//can update.  If you are adding content over time, (like, downloading highscores or whatever) it's ok to call ResizeScrollBounds
	//repeatedly to dynamically resize the scroll area as you go.

	pParent = pParent->GetEntityByName("scroll_child");
	pParent->RemoveAllEntities(); //clear it out in case we call this more than once, say, to update/change something

	float x = 5; //inset
	float y = 0;
	float spacerY = 30; //space between thingies

	//first, a title in a big font
	Entity *pTitle = CreateTextLabelEntity(pParent, "Title", x, 0, "About & Help"); 
	SetupTextEntity(pTitle, FONT_LARGE);
	y += pTitle->GetVar("size2d")->GetVector2().y +spacerY;

	//define an area for a text box, so it will wrap in  the right places.  Height is actually ignored.
	CL_Vec2f vTextBoxPos(x,y);
	CL_Vec2f vTextBounds(434, 200);
	string msg; //we could load text, but let's just manually put junk in there:

	msg += \
		"About this example:\n\n"\
		"Particle Test ``-`7 It shows the built in 2d particle system.  Tap anywhere to cause an explosion.``\n\n"\
		"Input Test ``- `7This option demonstrates getting input from the user.  The user must tap/click the input field to modify its text.  A native keyboard will pop up if needed.\n\n``"\
		"Debug and Music Test ``- `7This option lets you groove to some background techno music.  It's an mp3 on iOS, Windows and webOS, or loaded as .ogg on Android.\n\n``"\
		"About (scroll bar test) ``- `7Why, that's this!  It illustrates using a scroll bar.  You can flick up and down.  Hmm, maybe I should make it flick backwards on Windows, as it feels wrong.\n\n``"\
		"Also, notice that you can add any kind of entity within a scroll box, here is a clickable image:";

		//actually create the text box with the above text

	Entity *pEnt = CreateTextBoxEntity(pParent, "SomeTextBox", vTextBoxPos, vTextBounds, msg);
	y += pEnt->GetVar("size2d")->GetVector2().y; //move our Y position down the exact size of the text
	y += spacerY; //don't forget our spacer

	

	//add an image
	pEnt = CreateOverlayButtonEntity(pParent, "plogo", "interface/proton-128.rttex", x, y);
	pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	y += pEnt->GetVar("size2d")->GetVector2().y; //move our Y position down 
	y += spacerY;

	//do something if someone clicks it

	//add a clickable URL
	Entity *pButton;
	pButton = CreateTextButtonEntity(pParent, "twitter", iPhoneMapX(x), y, "`$Tap here for the RTsoft twitter page", true);
	pButton->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	//To work in HTML5 right:
	SetButtonStyleEntity(pButton, Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
	y += pButton->GetVar("size2d")->GetVector2().y;
	


	//automatically calculate the total size of this entity with all its children for the scroll bars, do this at the end
	VariantList vList(pParent->GetParent());
    ResizeScrollBounds(&vList);
}


Entity * AboutMenuCreate( Entity *pParentEnt)
{
	Entity *pBG = NULL;
	pBG =  CreateOverlayEntity(pParentEnt, "AboutMenu", "interface/bkgd_stone.rttex", 0,0);
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

	EntityComponent *pScrollComp = pScroll->AddComponent(new ScrollComponent);
	
	//turn on finger tracking enforcement, it means it will mark the tap as "handled" when touched.  Doesn't make a difference here,
	//but good to know about in some cases.  (some entity types will ignore touches if they've been marked as "Handled")

	pScrollComp->GetVar("fingerTracking")->Set(uint32(1)); 

	//note: If you don't want to see a scroll bar progress indicator, comment out the next line.
	EntityComponent *pScrollBarComp = pScroll->AddComponent(new ScrollBarRenderComponent); 	//add a visual way to see the scroller position
	
	//if we wanted to change the scroll bar color we could do it this way:
	//pScroll->GetVar("color")->Set(MAKE_RGBA(61,155, 193, 255)); 
	
	Entity *pScrollChild = pScroll->AddEntity(new Entity("scroll_child"));
	
	pScroll->AddComponent(new RenderScissorComponent()); //so the text/etc won't get drawn outside our scroll box
	pScroll->AddComponent(new FilterInputComponent); //lock out taps that are not in our scroll area

	//actually add all our content that we'll be scrolling (if there is too much for one screen), as much as we want, any kind of entities ok
	AboutMenuAddScrollContent(pBG);

	//oh, let's put the Back button on the bottom bar thing
	Entity * pEnt = CreateTextButtonEntity(pBG, "Back", 20, GetScreenSizeYf()-30, "Back", false);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&AboutMenuOnSelect);
	SetupTextEntity(pEnt, FONT_SMALL);
	AddHotKeyToButton(pEnt, VIRTUAL_KEY_BACK); //for androids back button and window's Escape button
	
	//slide it in with movement
	SlideScreen(pBG, true, 500);
	return pBG;
}