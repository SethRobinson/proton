//  ***************************************************************
//  Gamepad - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Gamepad_h__
#define Gamepad_h__

#include "Entity/ArcadeInputComponent.h"

class GamepadProvider;

enum eGamepadID
{
	GAMEPAD_ID_NONE = -1,
	GAMEPAD_ID_0,
	GAMEPAD_ID_1,
	GAMEPAD_ID_2,
	GAMEPAD_ID_3,
	GAMEPAD_ID_4,
	GAMEPAD_ID_5,
	GAMEPAD_ID_6,
	GAMEPAD_ID_7,
	GAMEPAD_ID_8,
};

#define GAMEPAD_MAX_AXIS 32
#define GAMEPAD_MAX_BUTTONS 16

class ArcadeInputComponent;

class GamepadButton
{
public:

	GamepadButton()
	{
		m_bDown = false;
	}
	void OnPress(bool bDown);
	eVirtualKeys m_virtualKey; //what we send when this is hit
	bool m_bDown;
};

class GamepadAxis
{
public:

	GamepadAxis()
	{
		m_useAsButton = VIRTUAL_KEY_NONE; //if not VIRTUAL_KEY_NONE, we'll treat it as a button instead.. good for triggers on xbox controller
		m_axis = 0;
	}
	
	float m_axis;
	eVirtualKeys m_useAsButton;
};

class Gamepad: public boost::signals::trackable
{
public:
	
	Gamepad();
	virtual ~Gamepad();
	
	virtual const string& GetName() {return m_name;}
	virtual bool Init()=0;
	virtual void Kill()=0;
	virtual void Update();
	
	void Reset() {m_bIsUsed = false;}
	bool GetIsUsed() {return m_bIsUsed;}
	void SetIsUsed(bool bUsed) {m_bIsUsed = bUsed;}

	//if you'd like to read full analog for the sticks, you should send false for bSendPadEventsAsFourDirections and listen
	//to those events yourself straight from the signal here
	void ConnectToArcadeComponent(ArcadeInputComponent *pComp, bool bSendButtonEvents, bool bSendPadEventsAsFourDirections);

	//for directly polling the stick state
	CL_Vec2f GetLeftStick();
	CL_Vec2f GetRightStick();

	//for directly polling the button state
	GamepadButton * GetButton(int buttonID);

	//which axes are used for the right stick varies per joystick on desktops, this lets us tweak it
	void SetRightStickAxis(int axisX, int axisY);

	//for using signals to get the button/stick events as they happen (probably better than directly polling in most situations)

	boost::signal<void (VariantList*)> m_sig_gamepad_buttons; //for arcade style events coming from buttons, you'll get VIRTUAL_DPAD_BUTTON_X and so forth

	//analog input

	boost::signal<void (VariantList*)> m_sig_left_stick;
	boost::signal<void (VariantList*)> m_sig_right_stick;

	void SetID(eGamepadID id){m_id = id;}
	eGamepadID GetID() {return m_id;}

	GamepadProvider * GetProvider() {return m_pPadProvider;}
	void SetProvider(GamepadProvider *pProvider) {m_pPadProvider = pProvider;}

	//called to set the button data, usually called by our subclass, you probably don't need this stuff.
    void SetAxis(int axis, float val); //must be -1 to 1.  axis 0 and 1 should be the main joystick
	void OnButton(bool bDown, int buttonID);
	void OnHat(int index, float val); 
    void SendArcadeDirectionByKey(eVirtualKeys key, bool bDown);
    
protected:

	void SendArcadeDirectionByDegrees(int val);
	void OnArcadeCompDestroyed( VariantList *pVList );
	void SendArcadeDirectionRelease();

	bool m_bJustSentStickRelease;
	string m_name;
	GamepadProvider *m_pPadProvider;

	GamepadAxis m_axis[GAMEPAD_MAX_AXIS];
	GamepadButton m_buttons[GAMEPAD_MAX_BUTTONS] ;

	int m_buttonsUsedCount;
	int m_axisUsedCount;
	int m_rightStickAxisX;
	int m_rightStickAxisY;

	//helps us only send if things have changed
	CL_Vec2f m_vLastSentLeftStick;
	CL_Vec2f m_vLastSentRightStick;
	
	eGamepadID m_id; //0 for gamepad 0, 1 for 1 and so on.

	bool m_bIsUsed; //if true, means we've allocated it for a player, just a thing to help us know which gamepads are
	//available.  Use GetGamepadManager()->Reset(); between games to reset this;

	//helps when sending stick movement as directions
	MoveButtonState m_dirButtons[MOVE_BUTTON_DIR_COUNT];
	ArcadeInputComponent *m_pArcadeComp;


};

#endif // Gamepad_h__