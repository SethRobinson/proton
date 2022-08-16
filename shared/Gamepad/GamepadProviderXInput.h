//  ***************************************************************
//  GamepadProviderXInput - Creation date: 11/03/2020
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2020 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GamepadProviderXInput_h__
#define GamepadProviderXInput_h__

#include <Xinput.h>

#include "GamepadProvider.h"
#include "Gamepad.h"


class GamepadProviderXInput : public GamepadProvider
{
public:
	GamepadProviderXInput();
	virtual ~GamepadProviderXInput();

	virtual string GetName() { return "XInput"; }
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void PreallocateControllersEvenIfMissing(bool bNew);
	
protected:
	
	eGamepadID m_gamepadUniqueID[XUSER_MAX_COUNT]; //remember where we put controllers
	
	bool _preallocateControllersEvenIfMissing;
	XINPUT_STATE m_stateTemp;

private:
};

#endif // GamepadProviderXInput_h__
