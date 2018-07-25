
//
//  60BeatPadNative.m
//  
//
//  Created by Seth Robinson on 1/30/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

#include "PlatformPrecomp.h"

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT

#import "GamepadProvider60BeatNative.h"
#include "GamepadProvider60Beat.h"
#include "Gamepad60Beat.h"

void PadButtonStates::SetButtonState(bool bDown)
{
    if (m_bDown == bDown) 
    {
       // LogMsg("Bdown is %d, no change", (int)bDown);
        return; //no change
    }
    
    if (bDown)
    {
       m_bWasPressed = true;
    } else
    {
        m_bWasReleased = true;
    }
    
    m_bDown = bDown;
}

bool PadButtonStates::WasPressed()
{
    if (m_bWasPressed)
    {
        m_bWasPressed = false;
        return true;
    }
    return false;
}

bool PadButtonStates::WasReleased()
{
    if (m_bWasReleased)
    {
        m_bWasReleased = false;
        return true;
    }
    return false;
}

@implementation SBeatPadNative

- (void)controlUpdated:(SBJoystick *)joystick
{

    //update member vars so we can send events of only things that changed at the correct input update time
    
    m_left = joystick.leftJoystickVector;
    //LogMsg("LeftJoy: %.2f, %.2f", left.x, left.y);


    m_right = joystick.rightJoystickVector;
    //LogMsg("LeftJoy: %.2f, %.2f", right.x, right.y);

    //Huh, it returns 16 when pressed.  Noted.
    //LogMsg("Button 1 is %d", int(joystick.button1State)); 
    
    m_buttons[SBEAT_BUTTON_1].SetButtonState(joystick.button1State != 0);
    m_buttons[SBEAT_BUTTON_2].SetButtonState(joystick.button2State != 0);
    m_buttons[SBEAT_BUTTON_3].SetButtonState(joystick.button3State != 0);
    m_buttons[SBEAT_BUTTON_4].SetButtonState(joystick.button4State != 0);


    m_buttons[SBEAT_BUTTON_L1].SetButtonState(joystick.buttonL1State != 0);
    m_buttons[SBEAT_BUTTON_L2].SetButtonState(joystick.buttonL2State != 0);
    m_buttons[SBEAT_BUTTON_R1].SetButtonState(joystick.buttonR1State != 0);
    m_buttons[SBEAT_BUTTON_R2].SetButtonState(joystick.buttonR2State != 0);
    
    m_buttons[SBEAT_BUTTON_UP].SetButtonState(joystick.buttonUState != 0);
    m_buttons[SBEAT_BUTTON_DOWN].SetButtonState(joystick.buttonDState != 0);
    m_buttons[SBEAT_BUTTON_LEFT].SetButtonState(joystick.buttonLState != 0);
    m_buttons[SBEAT_BUTTON_RIGHT].SetButtonState(joystick.buttonRState != 0);

    m_buttons[SBEAT_BUTTON_START].SetButtonState(joystick.buttonStartState != 0);
    m_buttons[SBEAT_BUTTON_SELECT].SetButtonState(joystick.buttonSelectState != 0);


}

- (void) SendDirIfNeeded: (Gamepad60Beat*) pPad buttonID: (int) buttonID down:(Boolean) bDown
{
    
    switch(buttonID)
    {
            
        case SBEAT_BUTTON_UP:
            pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_UP, bDown);
            break;

        case SBEAT_BUTTON_DOWN:
            pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_DOWN, bDown);
            break;
            
        case SBEAT_BUTTON_LEFT:
            pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_LEFT, bDown);
            break;
            
        case SBEAT_BUTTON_RIGHT:
            pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_RIGHT, bDown);
            break;
        default:
            ;
    }
    
}

- (void) Update: (Gamepad60Beat*) pPad
{
    
    for (int i=0; i < SBEAT_BUTTON_COUNT; i++)
    {
        if (m_buttons[i].WasPressed())
        {
            //LogMsg("%d Was pressed for reals, sending", i);
            [self SendDirIfNeeded: pPad buttonID: i down: true];
            
            pPad->OnButton(true, i);
            continue;
        }
        if (m_buttons[i].WasReleased())
        {
            [self SendDirIfNeeded: pPad buttonID: i down: false];
            pPad->OnButton(false, i);
            continue;
        }
        
      //  m_buttons[i].OncePerFrameUpdate();
    }
}

- (void)joystickStatusChanged:(SBJoystick *)joystick
{
	if (joystick.enabled)
	{
        LogMsg("Joystick enabled");
    } else
    {
        LogMsg("Joystick disabled");
    }
}

- (void) Start
{
    assert (SBEAT_BUTTON_COUNT < GAMEPAD_MAX_BUTTONS && "Just, no.");
    // Set the delegate to receive joystick events
	[SBJoystick sharedInstance].delegate = self;
}

- (CGPoint) GetLeftStickPos
{
    return m_left;
}
- (CGPoint) GetRightStickPos
{
    return m_right;
}
@end

#endif
