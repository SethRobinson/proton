#include "PlatformPrecomp.h"

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT

#include "Gamepad60Beat.h"
#include "GamepadProvider60BeatNative.h"


Gamepad60Beat::Gamepad60Beat()
{
	m_pPadProvider = NULL;
	
}

Gamepad60Beat::~Gamepad60Beat()
{
	Kill();
}

bool Gamepad60Beat::Init()
{
    m_axisUsedCount = 4;
    SetRightStickAxis(2,3);
    m_name = "60Beat";
    
    //improve button mappings
    
    m_buttons[SBEAT_BUTTON_1].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;
    m_buttons[SBEAT_BUTTON_2].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
    m_buttons[SBEAT_BUTTON_3].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
    m_buttons[SBEAT_BUTTON_4].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
  
    m_buttons[SBEAT_BUTTON_L1].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
    m_buttons[SBEAT_BUTTON_L2].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;

    m_buttons[SBEAT_BUTTON_R1].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
    m_buttons[SBEAT_BUTTON_R2].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;
 
    m_buttons[SBEAT_BUTTON_UP].m_virtualKey = VIRTUAL_DPAD_HAT_UP;
    m_buttons[SBEAT_BUTTON_DOWN].m_virtualKey = VIRTUAL_DPAD_HAT_DOWN;
    m_buttons[SBEAT_BUTTON_LEFT].m_virtualKey = VIRTUAL_DPAD_HAT_LEFT;
    m_buttons[SBEAT_BUTTON_RIGHT].m_virtualKey = VIRTUAL_DPAD_HAT_RIGHT;

    m_buttons[SBEAT_BUTTON_START].m_virtualKey = VIRTUAL_DPAD_SELECT;
    m_buttons[SBEAT_BUTTON_SELECT].m_virtualKey = VIRTUAL_DPAD_START;
    
	return true;
}


void Gamepad60Beat::Kill()
{
	
}

void Gamepad60Beat::Update()
{

	GamepadProvider60Beat *pProv = (GamepadProvider60Beat*)m_pPadProvider;

    CL_Vec2f vLeft = pProv->GetLeftStickPos();
    SetAxis(0, vLeft.x);
    SetAxis(1, vLeft.y);

    
    CL_Vec2f vRight = pProv->GetRightStickPos();
    SetAxis(2, vRight.x);
    SetAxis(3, vRight.y);

    Gamepad::Update();
}

#endif