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
	
	bool _preallocateControllersEvenIfMissing;

private:
};

#endif // GamepadProviderXInput_h__
