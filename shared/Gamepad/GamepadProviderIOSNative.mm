#pragma once
#import "GamepadProviderIOSNative.h"
#import <GameController/GameController.h>
#include "GamepadIOS.h"
#include "GamepadManager.h"

@implementation GamepadProviderIOSNative

- (void) Start: (GamepadProviderIOS*) pProvider
{
    
    m_pProvider = pProvider;
    
    LogMsg("Starting native iOS controller monitoring");

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnControllerDidConnectNotification:) name:GCControllerDidConnectNotification object:nil];
  
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OnControllerDidDisconnectNotification:) name:GCControllerDidDisconnectNotification object:nil];

}

- (void) setupControllers
{
    m_controllerArray = [GCController controllers];

     if ([m_controllerArray count] > 0 )
     {
         LogMsg("%d controllers connnected", m_controllerArray.count);
        // If there are connected Controllers
         [[UIApplication sharedApplication]setIdleTimerDisabled:YES];
         
     } else
      {
         LogMsg("No connected controllers");
         // If there are no connected Controllers
         [[UIApplication sharedApplication]setIdleTimerDisabled:NO];
      }
}

- (void) SetupButton: (GCControllerButtonInput *) button buttonID: (int) buttonID index: (unsigned long) index
{
    [button setValueChangedHandler:^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed)
     {
#ifdef _DEBUG
        LogMsg("Pressed %s idx: %lu, value: %.2f", [[button sfSymbolsName] UTF8String], index, value);
#endif
        
        if (value < 0.4f) pressed = false; //for now, we don't handle analog triggers as floats, so this requires the player to press down kind of far before triggering, don't want a hair trigger on these
        
        //figure out which gamepad this came from
        GamepadIOS *pPad = (GamepadIOS*)GetGamepadManager()->GetGamepadByUniqueID(index);
        
        if (pPad)
        {
            pPad->OnButton(pressed, buttonID);
            
            //special situation where we treat the dpad as the concept of "directions".  Later we might want to disable that, then
            //and just read buttons directly for some cases, like down open your scope or whatever
            
            switch(buttonID)
            {
                case GP_DPAD_HAT_LEFT: pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_LEFT, pressed); break;
                case GP_DPAD_HAT_RIGHT: pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_RIGHT, pressed); break;
                case GP_DPAD_HAT_UP: pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_UP, pressed); break;
                case GP_DPAD_HAT_DOWN: pPad->SendArcadeDirectionByKey(VIRTUAL_KEY_DIR_DOWN, pressed); break;
                    
                default: ;
                    
            }
        } else
        {
            LogMsg("Getting messages from unknown iOS gamepad ID %lu", index);
        }
        
      
        //GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float) buttonID, (float) (pressed));
     }
    ];
}


- (void) SetupAxis: (GCControllerAxisInput*) axisInput axis: (int) axis index: (unsigned long) index
{
    [axisInput setValueChangedHandler:^(GCControllerAxisInput * _Nonnull axisInput, float value)
     {
#ifdef _DEBUG
        LogMsg("Pressed %s idx: %lu, Axis %d, value: %.2f", [[axisInput sfSymbolsName] UTF8String], index, axis, value);
#endif
        //figure out which gamepad this came from
        GamepadIOS *pPad = (GamepadIOS*)GetGamepadManager()->GetGamepadByUniqueID(index);
        
        if (pPad)
        {
            
            //harded to reverse the Y on ios sticks, otherwise you push up and they go down
            if (axis == 1 || axis == 3)
            {
                value = (2.0f - (value + 1.0f));
                value -= 1.0f;
            }
            //LogMsg("AXis %d Changed to %.2f", axis, value);
            pPad->SetAxis(axis, value);
        } else
        {
            LogMsg("Getting messages from unknown iOS gamepad ID %lu", index);
        }
       
     }
    ];
}

- (void)OnControllerDidConnectNotification:(NSNotification *)note
{
    GCController *controller = (GCController *)note.object;
    GCExtendedGamepad *x = [controller extendedGamepad];
    [controller setPlayerIndex:0];
    if (x != nil)
    {
       
        unsigned long idx = (long)controller; //using memory address as a controller ID, I can't find anything else to use!
        
        LogMsg("Extended controller found.");
        
        //Note:  There is currently a bug in ios where XBOX XB1X pads have X and B reversed, but other Xbox pads are correct,
        //all show up here as "Xbox Wireless Controller" so unless we hit a private func to grab product ids we can't
        //map around it, and we probably shouldn't as it will likely be fixed anyway.
        
        
        [self SetupButton: x.buttonX buttonID: GP_DPAD_BUTTON_LEFT index: idx];
        [self SetupButton: x.buttonY buttonID: GP_DPAD_BUTTON_UP index: idx];
        [self SetupButton: x.buttonB buttonID: GP_DPAD_BUTTON_RIGHT index: idx];
        [self SetupButton: x.buttonA buttonID: GP_DPAD_BUTTON_DOWN index: idx];
        [self SetupButton: x.buttonMenu buttonID: GP_DPAD_START index: idx];
        [self SetupButton: x.buttonOptions buttonID: GP_DPAD_SELECT index: idx];
        [self SetupButton: x.rightShoulder buttonID: GP_DPAD_RBUTTON index: idx];
        [self SetupButton: x.leftShoulder buttonID: GP_DPAD_LBUTTON index: idx];
        [self SetupButton: x.rightTrigger buttonID: GP_DPAD_RTRIGGER index: idx];
        [self SetupButton: x.leftTrigger buttonID: GP_DPAD_LTRIGGER index: idx];
        [self SetupButton: x.leftThumbstickButton buttonID: GP_JOYSTICK_BUTTON_LEFT index: idx];
        [self SetupButton: x.rightThumbstickButton buttonID: GP_JOYSTICK_BUTTON_RIGHT index: idx];

          //for analog stick values
        [self SetupAxis: x.leftThumbstick.xAxis axis:0 index:idx];
        [self SetupAxis: x.leftThumbstick.yAxis axis:1 index:idx];
        [self SetupAxis: x.rightThumbstick.xAxis axis:2 index:idx];
        [self SetupAxis: x.rightThumbstick.yAxis axis:3 index:idx];
    
       
        [self SetupButton: x.dpad.left buttonID:GP_DPAD_HAT_LEFT index:idx];
        [self SetupButton: x.dpad.right buttonID:GP_DPAD_HAT_RIGHT index:idx];
        [self SetupButton: x.dpad.up buttonID:GP_DPAD_HAT_UP index:idx];
        [self SetupButton: x.dpad.down buttonID:GP_DPAD_HAT_DOWN index:idx];
        
        LogMsg("Controller connected (%s) ID %lu and setup", [controller.vendorName UTF8String], (long)controller);
    
        //add the pad
   
          GamepadIOS *pPad = new GamepadIOS;
          pPad->SetProvider(m_pProvider);
          GetGamepadManager()->AddGamepad(pPad, (long)controller);
    } else
    {
        LogMsg("Controller connected (%s), but we don't support that kind.", [controller.vendorName UTF8String]);

    }
    [self setupControllers];
}

- (void)OnControllerDidDisconnectNotification:(NSNotification *)note
{
    GCController *controller = (GCController *)note.object;
    LogMsg("Controller disconnected (%s) ID %lu", [controller.vendorName UTF8String], (long)controller);
    
    GetGamepadManager()->RemoveGamepadByUniqueID((eGamepadID) controller);
    
    [self setupControllers];
}

@end
