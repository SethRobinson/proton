//  ***************************************************************
//  GamepadiCade - Creation date: 07/1/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GamepadiCade_h__
#define GamepadiCade_h__

#include "Gamepad.h"
#include "GamepadProvideriCade.h"

class ArcadeKeyboardControl
{
public:
	ArcadeKeyboardControl()
	{
		m_bPushed = false;
        
	}


	void Setup(GamepadiCade *pPad, char keyDown, char keyUp, int buttonID, eVirtualKeys vKeyToSend);
	char m_keyDown, m_keyUp;
	bool m_bPushed;

};

enum eArcadeKeyboardControls
{
	//I've set these up to match the ICG07 rather than the old
	//one.. so the button/trigger mappings are really just the
	//"Other four buttons" on the old icade

    //don't change the order, they match the virtual keys
	KEY_ARCADE_BUTTON_LEFT,
	KEY_ARCADE_BUTTON_UP,
	KEY_ARCADE_BUTTON_RIGHT,
	KEY_ARCADE_BUTTON_DOWN,

	KEY_ARCADE_BUTTON_L,
	KEY_ARCADE_BUTTON_R,
	KEY_ARCADE_TRIGGER_L,
	KEY_ARCADE_TRIGGER_R,
	KEY_ARCADE_BUTTON_COUNT,

	//well, these aren't buttons, but we'll keep track this way
	KEY_ARCADE_DIR_LEFT,
	KEY_ARCADE_DIR_RIGHT,
	KEY_ARCADE_DIR_UP,
	KEY_ARCADE_DIR_DOWN,
	KEY_ARCADE_TOTAL_COUNT

};
class GamepadiCade: public Gamepad
{
public:
	GamepadiCade();
	virtual ~GamepadiCade();
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

private:

	void OnRawKeyboardInput( VariantList *pVList );
	void ActivateFakeKeyboardIOS(); //only does something on iOS
	void OnHardwareMessage( VariantList *pVList );

	ArcadeKeyboardControl m_keys[KEY_ARCADE_TOTAL_COUNT];
    bool m_bScanningForICade; //only used for iOS..
    bool m_bCurrentlyActive; //iOS...
};

#endif // GamepadiCade_h__
