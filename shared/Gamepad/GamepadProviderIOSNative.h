#pragma once

#include "PlatformPrecomp.h"
#include "GamepadProviderIOS.h"

#import <Foundation/Foundation.h>


@interface GamepadProviderIOSNative : NSObject
{
    NSArray * m_controllerArray;
    GamepadProviderIOS *m_pProvider; //remember our parent
}

- (void) Start: (GamepadProviderIOS*) pProvider;

@end
