//  ***************************************************************
//  GamepadManager - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
This handles gamepads for Proton. See RTLooneyLadders for a working example.

	* A gamepad provider can add one or more gamepads
	* You can use more than one gamepad provider at once
	* You can remove and add providers on the fly
	* Gamepads are pre-mapped in a universal way, instead of an "X" button on a 360, it's called VIRTUAL_KEY_DPAD_BUTTON_LEFT

To use, you would add this to your App.cpp (or somewhere)

GamepadManager g_gamepadManager;
GamepadManager * GetGamepadManager() {return &g_gamepadManager;}

You also need to call g_gamepadManager.Update() in your App::Update.

BaseApp.cpp already has it in its header.

Next you need to add a "gamepad provider" which in turn will add one or more gamepads.

For Directx in windows, you would do:

GetGamepadManager()->AddProvider(new GamepadProviderDirectX);

All gamepads would now be added, and you can access them with GetGamepadManager()->GetGamepad(0); (for gamepad 0)

Each gamepad has signals to connect to to receive events, or you can manually poll them.

Gamepads can be wired to ArcadeInputComponents:

m_pArcadeInput = (ArcadeInputComponent*) GetParent()->AddComponent(new ArcadeInputComponent);
Gamepad *pPad = GetGamepadManager()->GetUnusedGamepad();
if (pPad)
{
	//Cause gamepad virtual keys to be sent to the arcade component
	pPad->ConnectToArcadeComponent(m_pArcadeInput, true, false);
	
	//now we can just add bindings like normal to the ArcadeInputComponent
	
	AddKeyBinding(m_pArcadeInput, "Health", VIRTUAL_DPAD_BUTTON_RIGHT, VIRTUAL_KEY_CUSTOM_USE_HEALTH);

	//as for the analog stick info, we can connect directly to the pad's signals:
	pPad->m_sig_left_stick.connect(1, boost::bind(&VehicleControlComponent::OnGamepadStickUpdate, this, _1));	
	pPad->m_sig_right_stick.connect(1, boost::bind(&VehicleControlComponent::OnGamepadStickUpdate, this, _1));	
}

void VehicleControlComponent::OnGamepadStickUpdate(VariantList *pVList)
{
	CL_Vec2f vStick = pVList->Get(0).GetVector2();
	int32 padID = pVList->Get(1).GetINT32();
	int32 stickIndex = pVList->Get(2).GetINT32(); //0 for left, 1 for right
	
	LogMsg("Gamepad %d got a reading of %.2f, %.2f on stick %d", padID, vStick.x, vStick.y, stickIndex);
}

*/

#ifndef GamepadManager_h__
#define GamepadManager_h__

#include "Gamepad.h"
#include "GamepadProvider.h"

class GamepadManager
{
public:
	GamepadManager();
	virtual ~GamepadManager();

	Gamepad * GetDefaultGamepad(); //just use this for single player games?
	eGamepadID GetDefaultGamepadID(); //returns the best gamepad they have
	void Update(); //must be called every frame, handles updates, disconnections, etc
	void SetDefaultGamepad(Gamepad *pPad);
	
	int GetGamepadCount() {return m_gamepads.size();}

	Gamepad * GetGamepad(eGamepadID id);  //returns NULL if unavailable

	GamepadProvider * GetProviderByName(string name); //returns NULL if non exists

	bool RemoveProviderByName(string name); //returns true if something was actually removed
	GamepadProvider * AddProvider(GamepadProvider *provider);  //You new, we handle deleting it.  Returns false on error, and kills deletes it for you
	
	void AddGamepad(Gamepad * pad); //You new, we handle deleting it

	//some non-essential helpers for multi player gaming
	void ResetGamepads(); //set all buttons to off, and mark all as "unused"
	Gamepad * GetUnusedGamepad(); //returns the next unused gamepad and marks it as used, or NULL if none left

protected:

	void RemoveGamepadsByProvider(GamepadProvider *provider);

	eGamepadID m_defaultGamepadID;

	list<GamepadProvider*> m_providers;
	vector<Gamepad*> m_gamepads;
	
private:
};

#endif // GamepadManager_h__