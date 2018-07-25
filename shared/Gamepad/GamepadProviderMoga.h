//  ***************************************************************
//  GamepadProviderMoga - Creation date: 08/30/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


/*
 Provides an interface for the Moga controller. (Under NDA.. , so the meat is not included.. 
 this is all non-API specific stuff... will add the missing parts as soon as it's legal)
 */

#ifndef GamepadProviderMoga_h__
#define GamepadProviderMoga_h__

#include "GamepadProvider.h"

class GamepadMoga;

class GamepadProviderMoga: public GamepadProvider
{
public:
	GamepadProviderMoga();
	virtual ~GamepadProviderMoga();

	virtual string GetName() {return "Moga";}
	virtual bool Init();
	virtual void Kill();
	virtual void Update();
	
protected:
	
private:

    GamepadMoga *m_pPad;
};

#endif // GamepadProviderMoga_h__
