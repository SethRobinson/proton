#include "PlatformPrecomp.h"
#include "GamepadXInput.h"

GamepadXInput::GamepadXInput()
{
	_lastdwPacketNumber = 0;
}

GamepadXInput::~GamepadXInput()
{

	Kill();
}

bool GamepadXInput::Init()
{
	XINPUT_STATE state;

	ZeroMemory(&state, sizeof(XINPUT_STATE));

	assert(GetID() >= 0 && GetID() < XUSER_MAX_COUNT && "Um, make sure you check for XINput FIRST before any other gamepad types.  A hack sort of");
	
	//let's just assume it's plugged in right now
	//	if (XInputGetState(GetID(), &state) == ERROR_SUCCESS)
	{
		m_axisUsedCount = 2;
		m_name = "XInput Device";
		m_buttonsUsedCount = 16;
		SetRightStickAxis(2, 3);
		//these defaults are actually for a 360 pad.  Probably wrong for other pads..?

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

		m_buttons[10].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;
		m_buttons[11].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;

		m_buttons[12].m_virtualKey = VIRTUAL_KEY_DIR_DOWN;
		m_buttons[13].m_virtualKey = VIRTUAL_KEY_DIR_RIGHT;
		m_buttons[14].m_virtualKey = VIRTUAL_KEY_DIR_LEFT;
		m_buttons[15].m_virtualKey = VIRTUAL_KEY_DIR_UP;



	}

	return true;
}

void GamepadXInput::Kill()
{
	
}

void GamepadXInput::CheckButton(int buttonMask, int buttonID)
{
	if (m_buttons[buttonID].m_bDown != ((m_state.Gamepad.wButtons & buttonMask) != 0))
	{
		OnButton(m_state.Gamepad.wButtons & buttonMask, buttonID);
	}
}

//TODO:  Make this analog instead of binary.  I just don't know where to put it, another axis set I guess
void GamepadXInput::CheckTrigger(float trigger, int buttonID)
{
	//normalize it

	float leftTrigger = trigger / 255.0f;
	
	if (leftTrigger > 0.1f)
	{
		//count that as on
		if (!m_buttons[buttonID].m_bDown)
		{
			OnButton(true, buttonID);
		}
	}
	else
	{
		//not pressing
		if (m_buttons[buttonID].m_bDown)
		{
			OnButton(false, buttonID);
		}
	}
}

void GamepadXInput::SendArcadeDirectionByKeyIfChanged(eVirtualKeys key, bool bDown)
{

	if (key)
	SendArcadeDirectionByKey(key, bDown);
}

void GamepadXInput::Update()
{

	ZeroMemory(&m_state, sizeof(XINPUT_STATE));
	assert(GetID() >= 0 && GetID() < XUSER_MAX_COUNT && "Um, make sure you check for XINput FIRST before any other gamepad types.  A hack sort of");
	if (XInputGetState(GetID(), &m_state) == ERROR_SUCCESS)
	{

		if (_lastdwPacketNumber == m_state.dwPacketNumber) return; //nothing has changed
		_lastdwPacketNumber = m_state.dwPacketNumber;

		//Read buttons
		CheckButton(XINPUT_GAMEPAD_A, 0);
		CheckButton(XINPUT_GAMEPAD_B, 1);
		CheckButton(XINPUT_GAMEPAD_X, 2);
		CheckButton(XINPUT_GAMEPAD_Y, 3);

		CheckButton(XINPUT_GAMEPAD_LEFT_SHOULDER, 4);
		CheckButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, 5);

		CheckButton(XINPUT_GAMEPAD_BACK, 6);
		CheckButton(XINPUT_GAMEPAD_START, 7);
		CheckButton(XINPUT_GAMEPAD_LEFT_THUMB, 8);
		CheckButton(XINPUT_GAMEPAD_RIGHT_THUMB, 9);
	
		CheckTrigger(m_state.Gamepad.bLeftTrigger, 10);
		CheckTrigger(m_state.Gamepad.bRightTrigger, 11);

		SetAxis(0, ConvertToProtonStickWithDeadZone(m_state.Gamepad.sThumbLX));
		SetAxis(1, ConvertToProtonStickWithDeadZone(m_state.Gamepad.sThumbLY*-1.0f));
		SetAxis(2, ConvertToProtonStickWithDeadZone(m_state.Gamepad.sThumbRX));
		SetAxis(3, ConvertToProtonStickWithDeadZone(m_state.Gamepad.sThumbRY*-1.0f));

		//dpad controls.. note, the stick also maps to these.  If we needed to separate them uhh... hm
		CheckButton(XINPUT_GAMEPAD_DPAD_DOWN, 12);
		CheckButton(XINPUT_GAMEPAD_DPAD_RIGHT, 13);
		CheckButton(XINPUT_GAMEPAD_DPAD_LEFT, 14);
		CheckButton(XINPUT_GAMEPAD_DPAD_UP, 15);

		
	}

	Gamepad::Update();
}

float GamepadXInput::ConvertToProtonStickWithDeadZone(float xInputStick)
{
	xInputStick /= 32767.0f;
	
	if (fabs(xInputStick) < m_stickAsDirectionDeadZone)
	{
		xInputStick = 0;
	}
	return xInputStick;
}

