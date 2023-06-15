#include "PlatformPrecomp.h"
#include "GamepadSDL2.h"

GamepadSDL2::GamepadSDL2()
{
	m_pSDLJoy = NULL;
	m_pSDLController = NULL;
}

GamepadSDL2::~GamepadSDL2()
{
	Kill();
}

bool GamepadSDL2::Init()
{
	assert(!m_pSDLJoy);

		m_axisUsedCount = 2;
		m_name = "SDL2 Joystick Device";
		m_buttonsUsedCount = 16;
	
	return true;
}

void GamepadSDL2::InitExtraSDLStuff(SDL_GameController* pController, SDL_Joystick* pJoystick)
{
	m_pSDLJoy = pJoystick;
	m_pSDLController = pController;

	const char *pName = SDL_JoystickNameForIndex(GetID() - SDL_JOYSTICK_ID_OFFSET);
	m_name = "Unknown";
	if (m_pSDLController)
	{
		if (pName != NULL)
		{
			LogMsg("SDL controller detected as %s", pName);
			m_name = pName;

		}

		if (GetPlatformID() == PLATFORM_ID_LINUX)
		{
			//for whatever reason, a 360 maps incorrectly on raspberry pi os, but correctly on Windows?  Oh well, here's the linux version:

			LogMsg("Doing Linux mapping for SDL controllers...");
			SetRightStickAxis(4, 2);
			m_buttons[SDL_CONTROLLER_BUTTON_A].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
			m_buttons[SDL_CONTROLLER_BUTTON_B].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
			m_buttons[SDL_CONTROLLER_BUTTON_X].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
			m_buttons[SDL_CONTROLLER_BUTTON_Y].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

			m_buttons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
			m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
		
			m_buttons[SDL_CONTROLLER_BUTTON_GUIDE].m_virtualKey = VIRTUAL_DPAD_SELECT;
			
			//m_buttons[SDL_CONTROLLER_BUTTON_BACK].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;
			//m_buttons[SDL_CONTROLLER_BUTTON_START].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;
			m_buttons[SDL_CONTROLLER_BUTTON_BACK].m_virtualKey = VIRTUAL_KEY_NONE;
			m_buttons[SDL_CONTROLLER_BUTTON_START].m_virtualKey = VIRTUAL_KEY_NONE;
			
			m_buttons[SDL_CONTROLLER_BUTTON_LEFTSTICK].m_virtualKey = VIRTUAL_DPAD_START;
			m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSTICK].m_virtualKey = VIRTUAL_DPAD_MENU;

			m_buttons[11].m_virtualKey = VIRTUAL_KEY_NONE;

			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_DOWN].m_virtualKey = VIRTUAL_KEY_DIR_DOWN;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT].m_virtualKey = VIRTUAL_KEY_DIR_RIGHT;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_LEFT].m_virtualKey = VIRTUAL_KEY_DIR_LEFT;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_UP].m_virtualKey = VIRTUAL_KEY_DIR_UP;
		}
		else
		{

			//default mapping, works on Windows
			SetRightStickAxis(2, 3);

			m_buttons[SDL_CONTROLLER_BUTTON_A].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
			m_buttons[SDL_CONTROLLER_BUTTON_B].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
			m_buttons[SDL_CONTROLLER_BUTTON_X].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
			m_buttons[SDL_CONTROLLER_BUTTON_Y].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

			m_buttons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
			m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
			m_buttons[SDL_CONTROLLER_BUTTON_BACK].m_virtualKey = VIRTUAL_DPAD_SELECT;

			m_buttons[SDL_CONTROLLER_BUTTON_GUIDE].m_virtualKey = VIRTUAL_DPAD_MENU;
			m_buttons[SDL_CONTROLLER_BUTTON_START].m_virtualKey = VIRTUAL_DPAD_START;
			m_buttons[SDL_CONTROLLER_BUTTON_LEFTSTICK].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_LEFT;
			m_buttons[SDL_CONTROLLER_BUTTON_RIGHTSTICK].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_RIGHT;

			m_buttons[11].m_virtualKey = VIRTUAL_KEY_NONE;

			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_DOWN].m_virtualKey = VIRTUAL_KEY_DIR_DOWN;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT].m_virtualKey = VIRTUAL_KEY_DIR_RIGHT;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_LEFT].m_virtualKey = VIRTUAL_KEY_DIR_LEFT;
			m_buttons[SDL_CONTROLLER_BUTTON_DPAD_UP].m_virtualKey = VIRTUAL_KEY_DIR_UP;
		}


	}
	else
	{
		if (pName != NULL)
		{
			LogMsg("SDL joystick detected as %s", pName);
			m_name = pName;
		}

		//"Xbox 360 Controller"
		m_buttons[0].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
		m_buttons[1].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
		m_buttons[2].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
		m_buttons[3].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

		m_buttons[4].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
		m_buttons[5].m_virtualKey = VIRTUAL_DPAD_RBUTTON;

		m_buttons[6].m_virtualKey = VIRTUAL_DPAD_SELECT;
		m_buttons[7].m_virtualKey = VIRTUAL_DPAD_START;
		m_buttons[8].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_LEFT;
		m_buttons[9].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_RIGHT;

		/*

		//mapping for 8bitdo Pro 2
		SetRightStickAxis(3, 4);

		m_buttons[0].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
		m_buttons[1].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
		m_buttons[4].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
		m_buttons[3].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

		//m_buttons[4].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
		m_buttons[5].m_virtualKey = VIRTUAL_DPAD_RBUTTON;

		m_buttons[6].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
		m_buttons[7].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
		m_buttons[8].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;
		m_buttons[9].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;

		m_buttons[10].m_virtualKey = VIRTUAL_DPAD_SELECT;
		m_buttons[11].m_virtualKey = VIRTUAL_DPAD_START;

		m_buttons[12].m_virtualKey = VIRTUAL_DPAD_MENU;
		m_buttons[13].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_LEFT;
		m_buttons[14].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_RIGHT;
		m_buttons[15].m_virtualKey = VIRTUAL_KEY_DIR_UP;
		*/
	}
}

void GamepadSDL2::Kill()
{
	if (m_pSDLController)
	{
		LogMsg("Closing SDL controller %d", GetID());
		SDL_GameControllerClose(m_pSDLController);
		m_pSDLController = NULL;
		m_pSDLJoy = NULL; //this is actually the same structure
	}
	else
	{
		//free it this way
		LogMsg("Closing SDL joystick %d", GetID());
		SDL_JoystickClose(m_pSDLJoy);
		m_pSDLJoy = NULL;
	}

}


void GamepadSDL2::Update()
{

	Gamepad::Update();
}

void GamepadSDL2::OnSDLEvent(SDL_Event* pEvent)
{

	
	switch (pEvent->type)
	{
	//For things detected as "controllers"
	case SDL_CONTROLLERBUTTONDOWN:
		//LogMsg("SDL GameController: Pad %d button %d down", GetID(), pEvent->jbutton.button);
		
		OnButton(true, pEvent->jbutton.button);
		break;

	case SDL_CONTROLLERBUTTONUP:
		//LogMsg("SDL GameController: Pad %d button %d up", GetID(), pEvent->jbutton.button);

		OnButton(false, pEvent->jbutton.button);
		break;

	case SDL_CONTROLLERAXISMOTION:

		//LogMsg("Pad %d - Getting Controller axis %d: %d", GetID(), pEvent->caxis.axis, pEvent->caxis.value);


		switch (pEvent->caxis.axis)
		{

		case SDL_CONTROLLER_AXIS_LEFTX:
			SetAxis(0, ConvertToProtonStick(pEvent->caxis.value));
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			SetAxis(1, ConvertToProtonStick(pEvent->caxis.value));
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			SetAxis(2, ConvertToProtonStick(pEvent->caxis.value));
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			SetAxis(3, ConvertToProtonStick(pEvent->caxis.value));
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			//printf("Left trigger motion: %d\n", pEvent->caxis.value);
			
			//Why the hell on linux this is actually SDL_CONTROLLER_AXIS_RIGHTY on the xbox 360 controller? And
			//goes 0 to 32768?
			SetAxis(4, ConvertToProtonStickHalf(pEvent->caxis.value));

			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			//printf("Right trigger motion: %d\n", pEvent->caxis.value);
			SetAxis(5, ConvertToProtonStick(pEvent->caxis.value));
			break;
		}
	}

	if (m_pSDLController)
	{
		//ignore the joystick messages
		return;
	}

	switch (pEvent->type)
	{

		//for things not detected as "controllers", but just "joysticks"
	case SDL_JOYBUTTONDOWN:
		//LogMsg("SDL Joy: Pad %d button %d down", GetID(), pEvent->jbutton.button);
		OnButton(true, pEvent->jbutton.button);
		break;

	case SDL_JOYBUTTONUP:
		//LogMsg("SDL Joy: %d button %d down", GetID(), pEvent->jbutton.button);
		OnButton(false, pEvent->jbutton.button);
		break;

	case SDL_JOYAXISMOTION:
		//LogMsg("SDL Joy: Pad %d  axis %d: %d", GetID(), pEvent->jaxis.axis, pEvent->jaxis.value);
		SetAxis(pEvent->jaxis.axis, ConvertToProtonStick((float)pEvent->jaxis.value));
		break;
	}

}

float GamepadSDL2::ConvertToProtonStick(float xInputStick)
{
	xInputStick /= 32767.0f;
	return xInputStick;
}


float GamepadSDL2::ConvertToProtonStickHalf(float xInputStick)
{
	xInputStick -= (32767.0f / 2);
	xInputStick /= (32767.0f / 2);
	return xInputStick;
}

