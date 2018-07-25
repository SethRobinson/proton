//  ***************************************************************
//  GamepadProvideriCade - Creation date: 07/1/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


/*
 Provides an interface for the iCade controller.  Basically we just rewrote the keystrokes to look like a joystick.
 */

#ifndef GamepadProvideriCade_h__
#define GamepadProvideriCade_h__

#include "GamepadProvider.h"

class GamepadiCade;

class GamepadProvideriCade: public GamepadProvider
{
public:
	GamepadProvideriCade();
	virtual ~GamepadProvideriCade();

	virtual string GetName() {return "iCade";}
	virtual bool Init();
	virtual void Kill();
	virtual void Update();
	
	void OnLostiCadeConnection();
	
protected:
	
private:

    GamepadiCade *m_pPad;
};

#endif // GamepadProvideriCade_h__
