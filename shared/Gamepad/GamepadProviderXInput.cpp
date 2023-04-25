#include "PlatformPrecomp.h"
#include "GamepadProviderXInput.h"
#include "GamepadXInput.h"
#include "GamepadManager.h"

//#pragma comment (lib, "xinput.lib") //win 10+ only?
#pragma comment (lib, "XINPUT9_1_0.LIB") //so we can work with Win7

GamepadProviderXInput::GamepadProviderXInput()
{
	_preallocateControllersEvenIfMissing = false;

	//clear GamepadUniqueID
	memset(&m_gamepadUniqueID, 0, sizeof(m_gamepadUniqueID));
	
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


	/*
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
	*/
	
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
	
	//check if a controller was plugged in
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		if (m_gamepadUniqueID[i] != 0) continue; //already initted this one
		
		ZeroMemory(&m_stateTemp, sizeof(XINPUT_STATE));

		if (_preallocateControllersEvenIfMissing || XInputGetState(i, &m_stateTemp) == ERROR_SUCCESS)
		{
			//init this new gamepad that was just plugged in
			GamepadXInput* pPad = new GamepadXInput();
			pPad->SetProvider(this);
			GetGamepadManager()->AddGamepad(pPad);
			m_gamepadUniqueID[i] = pPad->GetID();
		}

	}
	
}

