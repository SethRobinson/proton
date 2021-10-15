#include "GamepadVita.h"

GamepadVita::GamepadVita()
{

}

GamepadVita::~GamepadVita()
{

}

bool GamepadVita::Init()
{
    m_name = "Vita";
    m_buttonsUsedCount = 14;
    m_axisUsedCount = 2;

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    m_buttons[0].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
    m_buttons[1].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;
    m_buttons[2].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
    m_buttons[3].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
    m_buttons[4].m_virtualKey = VIRTUAL_DPAD_SELECT;
    m_buttons[5].m_virtualKey = VIRTUAL_DPAD_START;
    m_buttons[6].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
    m_buttons[7].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
    m_buttons[8].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;
    m_buttons[9].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;
    m_buttons[10].m_virtualKey = VIRTUAL_DPAD_HAT_UP;
    m_buttons[11].m_virtualKey = VIRTUAL_DPAD_HAT_RIGHT;
    m_buttons[12].m_virtualKey = VIRTUAL_DPAD_HAT_DOWN;
    m_buttons[13].m_virtualKey = VIRTUAL_DPAD_HAT_LEFT;

    return true;
}

void GamepadVita::Kill()
{

}

void GamepadVita::Update()
{
    sceCtrlPeekBufferPositive(0, &m_state, 1);

    PressButton(SCE_CTRL_TRIANGLE, 1);
    PressButton(SCE_CTRL_CIRCLE, 2);
    PressButton(SCE_CTRL_CROSS, 3);
    PressButton(SCE_CTRL_SQUARE, 0);
    
    PressButton(SCE_CTRL_UP, 10);
    PressButton(SCE_CTRL_RIGHT, 11);
    PressButton(SCE_CTRL_DOWN, 12);
    PressButton(SCE_CTRL_LEFT, 13);

    PressButton(SCE_CTRL_START, 5);
    PressButton(SCE_CTRL_SELECT, 4);
    
    PressButton(SCE_CTRL_L1, 6);
    PressButton(SCE_CTRL_R1, 7);

	SetAxis(0, ConvertToProtonStickWithDeadZone(m_state.lx));
	SetAxis(1, ConvertToProtonStickWithDeadZone(m_state.ly * -1.0f));
	SetAxis(2, ConvertToProtonStickWithDeadZone(m_state.rx));
	SetAxis(3, ConvertToProtonStickWithDeadZone(m_state.ry * -1.0f));
}

void GamepadVita::PressButton(int mask, int id)
{
    if (m_buttons[id].m_bDown != ((m_state.buttons & mask) != 0))
	{
		OnButton(m_state.buttons & mask, id);
	}
}

float GamepadVita::ConvertToProtonStickWithDeadZone(float stick)
{
	stick /= 32767.0f;
	const float deadZone = 0.15f;
	if (fabs(stick) < deadZone)
	{
		stick = 0;
	}
	return stick;
}