#pragma once
#include "PlatformPrecomp.h"



#include "GamepadIOS.h"
//#include "GamepadProviderIOSNative.h"

GamepadIOS::GamepadIOS()
{
	m_pPadProvider = NULL;
	
}

GamepadIOS::~GamepadIOS()
{
	Kill();
}

bool GamepadIOS::Init()
{
    m_axisUsedCount = 4;
    SetRightStickAxis(2,3);
    
    m_name = "IOS";
    
    //button mappings
   
    m_buttons[GP_DPAD_BUTTON_UP].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;
    m_buttons[GP_DPAD_BUTTON_RIGHT].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
    m_buttons[GP_DPAD_BUTTON_DOWN].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
    m_buttons[GP_DPAD_BUTTON_LEFT].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
  
    m_buttons[GP_DPAD_LBUTTON].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
    m_buttons[GP_DPAD_LTRIGGER].m_virtualKey = VIRTUAL_DPAD_LTRIGGER;

    m_buttons[GP_DPAD_RBUTTON].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
    m_buttons[GP_DPAD_RTRIGGER].m_virtualKey = VIRTUAL_DPAD_RTRIGGER;
 
    m_buttons[GP_DPAD_HAT_UP].m_virtualKey = VIRTUAL_DPAD_HAT_UP;
    m_buttons[GP_DPAD_HAT_DOWN].m_virtualKey = VIRTUAL_DPAD_HAT_DOWN;
    m_buttons[GP_DPAD_HAT_LEFT].m_virtualKey = VIRTUAL_DPAD_HAT_LEFT;
    m_buttons[GP_DPAD_HAT_RIGHT].m_virtualKey = VIRTUAL_DPAD_HAT_RIGHT;

    m_buttons[GP_DPAD_SELECT].m_virtualKey = VIRTUAL_DPAD_SELECT;
    m_buttons[GP_DPAD_START].m_virtualKey = VIRTUAL_DPAD_START;
   
    m_buttons[GP_JOYSTICK_BUTTON_LEFT].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_LEFT;
    m_buttons[GP_JOYSTICK_BUTTON_RIGHT].m_virtualKey = VIRTUAL_JOYSTICK_BUTTON_RIGHT;
    
 
   
    
	return true;
}


void GamepadIOS::Kill()
{
}

void GamepadIOS::Update()
{

	GamepadProviderIOS *pProv = (GamepadProviderIOS*)m_pPadProvider;

    /*
    CL_Vec2f vLeft = pProv->GetLeftStickPos();
    SetAxis(0, vLeft.x);
    SetAxis(1, vLeft.y);

    CL_Vec2f vRight = pProv->GetRightStickPos();
    SetAxis(2, vRight.x);
    SetAxis(3, vRight.y);

	*/
	
    Gamepad::Update();
}


