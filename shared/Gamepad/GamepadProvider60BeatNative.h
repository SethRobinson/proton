//
//  60BeatPadNative.h
//  
//
//  Created by Seth Robinson on 1/30/12.
//  Copyright (c) 2012 Robinson Technologies. All rights reserved.
//

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT

#import <Foundation/Foundation.h>
#import "SBJoystick.h"


//To avoid problems with losing button presses on poor framerates, we are going to keep track ourselves.
//Note:  The SBeat library doesn't really send events, it's more like we just poll at 30 fps and have to do stuff
//our self
class Gamepad60Beat;

enum eButton60BeatButtons
{
	SBEAT_BUTTON_1,
    SBEAT_BUTTON_2,
    SBEAT_BUTTON_3,
    SBEAT_BUTTON_4,
    SBEAT_BUTTON_UP,
    SBEAT_BUTTON_DOWN,
    SBEAT_BUTTON_LEFT,
    SBEAT_BUTTON_RIGHT,
    SBEAT_BUTTON_START,
    SBEAT_BUTTON_SELECT,
    SBEAT_BUTTON_L1,
    SBEAT_BUTTON_R1,
    SBEAT_BUTTON_L2,
    SBEAT_BUTTON_R2,

	//add buttons above here
	SBEAT_BUTTON_COUNT
};

class PadButtonStates
{
public:
    
    PadButtonStates()
    {
        m_bDown = false;
        m_bWasPressed = false;
        m_bWasReleased = false;
    }
    void OncePerFrameUpdate();
    bool WasPressed();
    bool WasReleased();
    
    void SetButtonState(bool bDown);

private:
    bool m_bDown;
    bool m_bWasPressed;
    bool m_bWasReleased;
    
};

@interface SBeatPadNative : NSObject <SBJoystickDelegate>
{
    unsigned int m_updateCount;
    PadButtonStates m_buttons[SBEAT_BUTTON_COUNT];
    
    CGPoint m_left, m_right;  //for polling the stick data

}

- (CGPoint) GetLeftStickPos;
- (CGPoint) GetRightStickPos;
- (void) Start;
- (void) Update: (Gamepad60Beat*) pPad;
- (void) SendDirIfNeeded: (Gamepad60Beat*) pPad buttonID: (int) buttonID down:(Boolean) bDown;

@end
#endif