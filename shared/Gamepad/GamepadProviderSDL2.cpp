#include "PlatformPrecomp.h"
#include "GamepadProviderSDL2.h"
#include "GamepadSDL2.h"
#include "GamepadManager.h"

GamepadProviderSDL2::GamepadProviderSDL2()
{

}

GamepadProviderSDL2::~GamepadProviderSDL2()
{
}


void GamepadProviderSDL2::AddGamepadBySDLID(int i)
{
	SDL_GameController* controller = NULL;
	SDL_Joystick* j = NULL;

	if (GetGamepadManager()->GetGamepadByUniqueID(i+ SDL_JOYSTICK_ID_OFFSET))
	{
		LogMsg("Joystick %d already handled", i);
		return;
	}

	if (SDL_IsGameController(i))
	{
		 controller = SDL_GameControllerOpen(i);
		 assert(controller);
		 j = SDL_GameControllerGetJoystick(controller);
		 assert(j);
	}
	else
	{
		LogMsg("Could not open as game controller %d: %s\n", i, SDL_GetError());
		j = SDL_JoystickOpen(i);
		assert(j);
	}

	int adjustedID = SDL_JoystickInstanceID(j) + SDL_JOYSTICK_ID_OFFSET;
	if (GetGamepadManager()->GetGamepadByUniqueID(adjustedID))
	{
		LogMsg("Ignoring joystick, already exists");
		
		/*
		if (controller)
		{
			SDL_GameControllerClose(controller);
		}
		else
		{
			SDL_JoystickClose(j);
		}
		*/
		return;
	}

	GamepadSDL2* pPad = new GamepadSDL2();
	pPad->SetProvider(this);
	GetGamepadManager()->AddGamepad(pPad, adjustedID);
	pPad->InitExtraSDLStuff(controller, j);
}

void GamepadProviderSDL2::OnSDLEvent(VariantList* pVList)
{
	SDL_Event* pEvent = (SDL_Event*)pVList->Get(0).GetEntity();

	switch (pEvent->type)
	{

		
	case SDL_CONTROLLERDEVICEADDED:
	{
		AddGamepadBySDLID(pEvent->jdevice.which);
		break;
	}

	case SDL_CONTROLLERDEVICEREMOVED:
	{
		SDL_JoystickID joystickID = SDL_JOYSTICK_ID_OFFSET + pEvent->jdevice.which;
		//LogMsg("Joystick removed, instance id: %d\n", joystickID);
		GetGamepadManager()->RemoveGamepadByUniqueID(joystickID);
		break;
	}

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_CONTROLLERAXISMOTION:
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		//LogMsg("Got button event");

		GamepadSDL2* pPad = (GamepadSDL2*)GetGamepadManager()->GetGamepadByUniqueID(pEvent->jbutton.which + SDL_JOYSTICK_ID_OFFSET);
		if (pPad)
		{
			pPad->OnSDLEvent(pEvent);
		}
		else
		{
			LogMsg("SDL Joystick error: Gamepad device %d doesn't exist", pEvent->jbutton.which + SDL_JOYSTICK_ID_OFFSET);
		}
		break;

	}
}

bool GamepadProviderSDL2::Init()
{
	LogMsg("Initting SDL2 windows gamepad provider");

	
	int numJoysticks = SDL_NumJoysticks();
	LogMsg("We can see %d joysticks are detected", numJoysticks);

	// Check for joysticks that are already connected

	for (int i = 0; i < numJoysticks; i++)
	{
		AddGamepadBySDLID(i);
	}
	g_sig_SDLEvent.connect(1, boost::bind(&GamepadProviderSDL2::OnSDLEvent, this, _1));
	return true;
}

void GamepadProviderSDL2::PreallocateControllersEvenIfMissing(bool bNew)
{

}

void GamepadProviderSDL2::Kill()
{
	//LogMsg("Killing provider");
}

void GamepadProviderSDL2::Update()
{


}

