//  ***************************************************************
//  GamepadXInput - Creation date: 11/03/2020
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2020 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GamepadXInput_h__
#define GamepadXInput_h__

#include "Gamepad.h"
#include "GamepadProviderXInput.h"

class GamepadXInput : public Gamepad
{
public:
	GamepadXInput();
	virtual ~GamepadXInput();
	virtual bool Init();
	virtual void Kill();
	void SendArcadeDirectionByKeyIfChanged(eVirtualKeys key, bool bDown);
	virtual void Update();

protected:
	
	float ConvertToProtonStick(float xInputStick);
	void CheckButton(int buttonMask, int buttonID);
	void CheckTrigger(float trigger, int buttonID);
	DWORD _lastdwPacketNumber; //if not the same, state has changed.  Just a trick to avoid polling when not needed.
	XINPUT_STATE m_state;
	bool m_haveScannedDeviceAbilities = false;

private:
};

#endif // GamepadXInput_h__
