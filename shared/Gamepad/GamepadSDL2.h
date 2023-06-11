//  ***************************************************************
//  GamepadSDL2 - Creation date: 6/11/2023 2:16:25 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "Gamepad.h"
#include "GamepadProviderSDL2.h"

class GamepadSDL2 : public Gamepad
{
public:

	GamepadSDL2();
	virtual ~GamepadSDL2();

	virtual bool Init();
	void InitExtraSDLStuff(SDL_GameController* pController, SDL_Joystick* pJoystick);
	virtual void Kill();
	virtual void Update();

	float ConvertToProtonStick(float xInputStick);

	float ConvertToProtonStickHalf(float xInputStick);

	void OnSDLEvent(SDL_Event* pEvent);

protected:

	SDL_Joystick* m_pSDLJoy;
	SDL_GameController* m_pSDLController;
};
