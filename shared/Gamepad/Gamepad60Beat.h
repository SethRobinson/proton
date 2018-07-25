//  ***************************************************************
//  Gamepad60Beat - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//The DirectInput code was mostly stolen from Clanlib, thanks guys!

#ifndef Gamepad60Beat_h__
#define Gamepad60Beat_h__

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT

#include "Gamepad.h"
#include "GamepadProvider60Beat.h"

class Gamepad60Beat: public Gamepad
{
public:
	Gamepad60Beat();
	virtual ~Gamepad60Beat();
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void SetProvider(GamepadProvider60Beat *pProvider) {m_pPadProvider = pProvider;}


protected:
	

private:

	GamepadProvider60Beat *m_pPadProvider;

	

};

#endif // Gamepad60Beat_h__

#endif