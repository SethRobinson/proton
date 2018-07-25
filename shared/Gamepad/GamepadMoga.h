//  ***************************************************************
//  GamepadMoga - Creation date: 08/30/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//The DirectInput code was mostly stolen from Clanlib, thanks guys!

#ifndef GamepadMoga_h__
#define GamepadMoga_h__

#include "Gamepad.h"
#include "GamepadProviderMoga.h"

class GamepadMoga: public Gamepad
{
public:
	GamepadMoga();
	virtual ~GamepadMoga();
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void SetProvider(GamepadProviderMoga *pProvider) {m_pPadProvider = pProvider;}

	
	//Moga specific
	void OnJoypadEvent(VariantList *pVList);

protected:

private:

	GamepadProviderMoga *m_pPadProvider;
	bool m_bConnected;
};

#endif // GamepadMoga_h__
