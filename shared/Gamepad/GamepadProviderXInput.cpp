#include "PlatformPrecomp.h"
#include "GamepadProviderXInput.h"
#include "GamepadXInput.h"
#include "GamepadManager.h"

#pragma comment (lib, "xinput.lib")

GamepadProviderXInput::GamepadProviderXInput()
{
	_preallocateControllersEvenIfMissing = false;
}

GamepadProviderXInput::~GamepadProviderXInput()
{
}

bool GamepadProviderXInput::Init()
{
	LogMsg("Initting XInput windows gamepad provider");
	
	if (_preallocateControllersEvenIfMissing)
	{
		LogMsg("(_preallocateControllersEvenIfMissing flag set - Initting space for four xinput devices, whether or not they are plugged in/on or not)");
	}
	int playerID = -1;
	XINPUT_STATE state;

	for (DWORD i = 0; i < XUSER_MAX_COUNT && playerID == -1; i++)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (_preallocateControllersEvenIfMissing || XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			//valid gamepad
			GamepadXInput* pPad = new GamepadXInput();
			pPad->SetProvider(this);
			GetGamepadManager()->AddGamepad(pPad);
			
		}
			
	}

	return true;
}

void GamepadProviderXInput::PreallocateControllersEvenIfMissing(bool bNew)
{
	_preallocateControllersEvenIfMissing = bNew;
}

void GamepadProviderXInput::Kill()
{
}

void GamepadProviderXInput::Update()
{
}

