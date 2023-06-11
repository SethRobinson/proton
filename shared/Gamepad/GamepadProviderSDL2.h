//  ***************************************************************
//  GamepadProviderSDL2 - Creation date: 6/11/2023 1:31:47 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "GamepadProvider.h"
#include "Gamepad.h"
#include <SDL2/SDL.h>
#include "SDL/SDL2Main.h"

const int SDL_JOYSTICK_ID_OFFSET = 100; //doesn't really matter, just don't want IDs overlapping

class GamepadProviderSDL2 : public GamepadProvider
{
public:
	GamepadProviderSDL2();
	virtual ~GamepadProviderSDL2();


	virtual string GetName() { return "SDL2"; }
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void PreallocateControllersEvenIfMissing(bool bNew);

protected:

	void AddGamepadBySDLID(int id);
	void OnSDLEvent(VariantList* pVList);



};
