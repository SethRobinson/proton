//  ***************************************************************
//  GamepadProvider60Beat - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


/*
 
 Provides the GamepadManager with support for the 60beat gamepad.
 
 Just adding the three files and a GetGameManager->AddProvider(new GamepadProvider60Beat); is enough to make it work,
 but you should also define RT_60Beat_GAMEPAD_SUPPORT so the Denshion audio
 manager will also call it when audio is suspended.
 
 (This pad uses the mic jack.. I have no idea what will happen if you try it with the
 iOS fmod audio system..)
 
 
 
 */

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT


#ifndef GamepadProvider60Beat_h__
#define GamepadProvider60Beat_h__

#include "GamepadProvider.h"


class Gamepad60Beat;

class GamepadProvider60Beat: public GamepadProvider
{
public:
	GamepadProvider60Beat();
	virtual ~GamepadProvider60Beat();

	virtual string GetName() {return "60Beat";}
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	//Used by our custom Gamepad60Beat class
	
    CL_Vec2f  GetLeftStickPos();
    CL_Vec2f  GetRightStickPos();
    
protected:
	
    void OnEnterBackground(VariantList *pVList);
    void OnEnterForeground(VariantList *pVList);

private:


    Gamepad60Beat *m_pPad;
};

#endif // GamepadProvider60Beat_h__
#endif
