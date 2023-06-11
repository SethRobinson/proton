#include "PlatformPrecomp.h"
#include "ControllerTestMenu.h"
#include "Entity/EntityUtils.h"
#include "Entity/CustomInputComponent.h"
#include "GUI/IntroMenu.h"
#include "Gamepad/GamepadManager.h"
#include "Entity/LogDisplayComponent.h"

void DisconnectGameControllers();

void ControllerTestMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	Entity *pMenu = GetEntityRoot()->GetEntityByName("ControllerTest");
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[1].Print().c_str());

	if (pEntClicked->GetName() == "back")
	{
		//SetConsole(false);

		DisconnectGameControllers();
		DisableAllButtonsEntity(pMenu);
		SlideScreen(pEntClicked->GetParent(), false);
		//kill this menu entirely, but we wait half a second while the transition is happening before doing it
		GetMessageManager()->CallEntityFunction(pEntClicked->GetParent(), 500, "OnDelete", NULL); 
		IntroMenuCreate(pMenu->GetParent());
		SetConsole(false, false);
		return;
	}
}


void OnGamepadButton(VariantList *m_pVList)
{
	eVirtualKeys vKey = (eVirtualKeys) m_pVList->Get(0).GetUINT32();
	eVirtualKeyInfo vKeyInfo = (eVirtualKeyInfo) m_pVList->Get(1).GetUINT32();
	int gamepadID = m_pVList->Get(2).GetUINT32();

	string buttonName;
	string action;

	if (vKeyInfo == VIRTUAL_KEY_PRESS)
	{
		action = "Pressed";
	} else
	{
		action = "Released";
	}

	if (vKey == VIRTUAL_KEY_NONE) return; //who cares


	LogMsg("`6Gamepad `w%d``: `#%s`` is `$%s````", gamepadID, ProtonVirtualKeyToString(vKey).c_str(), action.c_str());
}


void DisconnectGameControllers()
{
	for (int i=0; i < GetGamepadManager()->GetGamepadCount(); i++)
	{
		Gamepad *pPad = GetGamepadManager()->GetGamepad(i);
		pPad->m_sig_gamepad_buttons.disconnect(OnGamepadButton);
	}
}

void SetupGameControllers(Entity *pBG)
{
	
	//we'll directly connect to the game controllers.  For an example of how to link them to an ArcadeComponent instead, look at
	//GameMenu.cpp, the Looney Ladder example is setup that way.

	//Because we're connecting to static functions and not to virtual functions inside of entities or components, keep in mind we'll need
	//to unplug them manually when we quit, otherwise we'd start getting multiple messages if we ran this function multiple times.

	//Just connect to all pads at once.. ok to do for a single player game.. otherwise, we should use
	//Gamepad *pPad = GetGamepadManager()->GetDefaultGamepad(); instead probably, or let the user choose.
	//Keep in mind pads can be removed/added on the fly

	for (int i=0; i < GetGamepadManager()->GetGamepadCount(); i++)
	{
	
		Gamepad *pPad = GetGamepadManager()->GetGamepad(i);
		pPad->m_sig_gamepad_buttons.connect(OnGamepadButton);

		//if we cared about the analog stick exact input, we'd do this.  For fighting games, this would be good, so you don't fail
		//your dragon punch because FPS was slow and polling missed something.

		//pPad->m_sig_left_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
		//pPad->m_sig_right_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
	}
}

void DrawPad(CL_Vec2f vDir, CL_Vec2f vPos, string name)
{
	static Surface surfPad("interface/gamepad_circle.rttex"); //never do this in real life
	
	surfPad.BlitScaled(vPos.x, vPos.y, CL_Vec2f(1,1), ALIGNMENT_CENTER, MAKE_RGBA(200,255,255,255));
	DrawFilledSquare(vPos.x + vDir.x* (surfPad.GetWidth()*.5f), vPos.y + vDir.y*(surfPad.GetHeight()*.5f), 10, MAKE_RGBA(255,0,0,180), true);
	GetBaseApp()->GetFont(FONT_SMALL)->DrawAligned(vPos.x, vPos.y+surfPad.GetHeight()/2, name, ALIGNMENT_UPPER_CENTER);
}

void DrawButtonInfoByPolling(Gamepad *pPad)
{
	float x = 340;
	float startY = 100;
	float y = startY;
	float spacerY = 30;
	
	for (int i=0; i < GAMEPAD_MAX_BUTTONS; i++)
	{
		string vKeyName = ProtonVirtualKeyToString(pPad->GetButton(i)->m_virtualKey);

		string pressed;
		if (pPad->GetButton(i)->m_bDown) pressed = "`4X``";
		GetBaseApp()->GetFont(FONT_SMALL)->Draw(x,y, ""+toString(i)+" ("+vKeyName+") "+pressed);

		y += spacerY;
		if (y > 320)
		{
			//reset to next column
			x += 350;
			y = startY;
		}
	}
}

void ControllerTestOnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2(); //offset we should render to, this changes if the parent entity hierarchy's pos2d var moves around
	//render out gamepad positions

	//To switch gamepads we're testing...
	//GetGamepadManager()->SetDefaultGamepad(GetGamepadManager()->GetGamepad(1));
	
	
	Gamepad *pPad =  GetGamepadManager()->GetDefaultGamepad();
	

	if (!pPad)
	{
		//no gamepad detect
		GetBaseApp()->GetFont(FONT_SMALL)->Draw(30, 200, "No gamepad detected.  So.. er, yeah.");
		return;
	}

	GetBaseApp()->GetFont(FONT_SMALL)->Draw(30, 680, "Showing data from gamepad ID "+toString(pPad->GetID())+", "+pPad->GetName());
	DrawPad(pPad->GetLeftStick(), CL_Vec2f(130,200), "Left stick");
	DrawPad(pPad->GetRightStick(), CL_Vec2f(250,200), "Right stick");

	DrawButtonInfoByPolling(pPad);
}

Entity * ControllerTestMenuCreate(Entity *pParentEnt)
{
	Entity *pBG = CreateOverlayRectEntity(pParentEnt, GetScreenRect(), MAKE_RGBA(0,0,0,255));
	pBG->SetName("ControllerTest"); //so we can find this entiry again later

	//Add back button
	Entity * pEnt = CreateTextButtonEntity(pBG, "back", 80, 730, "Back");
	pEnt->GetShared()->GetFunction("OnButtonSelected")->sig_function.connect(&ControllerTestMenuOnSelect);

	//text at the top
	pEnt = CreateTextLabelEntity(pBG, "Text", GetScreenSizeXf()/2, 30, "Gamepad Controller Test");
	SetupTextEntity(pEnt, FONT_LARGE, 1.0f);
	SetAlignmentEntity(pEnt, ALIGNMENT_UPPER_CENTER);

	//hook into the rendering, and render our own stuff from here
	pEnt->GetShared()->GetFunction("OnRender")->sig_function.connect(&ControllerTestOnRender);

	SetupGameControllers(pBG); //attach an arcade component and wire it to the gamepads

	AddFocusIfNeeded(pBG);

	pEnt = CreateTextLabelEntity(pBG, "Text", 200, 340, "Output Log, this shows buttons event.  Drag to scroll.");

	//oh, let's add a scrolling text window to see the log
	SetConsole(true, true);
	//adjust the console so it isn't full screen
	Entity *pConsole = GetEntityRoot()->GetEntityByName("ConsoleEnt");
	pConsole->GetVar("pos2d")->Set(CL_Vec2f(0,370));
	pConsole->GetVar("size2d")->Set(CL_Vec2f(GetScreenSizeXf(),300));
	EntityComponent *pComp = pConsole->AddComponent(new RectRenderComponent);
	pComp->GetVar("visualStyle")->Set((uint32)RectRenderComponent::STYLE_BORDER_ONLY);
	pComp->GetVar("borderColor")->Set(MAKE_RGBA(0,0,255,200));
//	pConsole->MoveComponentToTopByAddress(pComp);

	return pBG;
}
