//  ***************************************************************
//  GamepadIOS - Creation date: 08/08/2022
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2022 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once


#include "Gamepad.h"
#include "GamepadProviderIOS.h"

class GamepadIOS : public Gamepad
{
public:
	GamepadIOS();
	virtual ~GamepadIOS();
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void SetProvider(GamepadProviderIOS* pProvider) { m_pPadProvider = pProvider; }


protected:


private:

	GamepadProviderIOS* m_pPadProvider;

};

