#include "PlatformPrecomp.h"

#include "GamepadProviderIOS.h"
#include "GamepadIOS.h"
#include "GamepadManager.h"
#import <GameController/GameController.h>
#import "GamepadProviderIOSNative.h"


id m_IOSGamepadNative = nil;

GamepadProviderIOS::GamepadProviderIOS()
{
	Kill();
}

GamepadProviderIOS::~GamepadProviderIOS()
{

    Kill();
}

bool GamepadProviderIOS::Init()
{

#ifndef RT_IOS_IOS_GAMEPAD_SUPPORT
   // assert(!"You really should define RT_IOS_IOS_GAMEPAD_SUPPORT in your xCode project!");
#endif
	
  assert(m_IOSGamepadNative == 0);
    m_IOSGamepadNative = [GamepadProviderIOSNative new];
    [m_IOSGamepadNative Start: this];
    
    LogMsg("Initting IOS gamepad provider");
    
    printf("controllers %lu\n", [[GCController controllers] count]);
  	
    //get notified on suspend/resume    
    GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&GamepadProviderIOS::OnEnterForeground, this, _1));
	GetBaseApp()->m_sig_enterbackground.connect(1, boost::bind(&GamepadProviderIOS::OnEnterBackground, this, _1));

    return true; //success
}


void GamepadProviderIOS::Kill()
{
    if (m_IOSGamepadNative != 0)
    {
        [m_IOSGamepadNative release];
        m_IOSGamepadNative = nil;
    }
}


void GamepadProviderIOS::Update()
{
	//[m_padDriver Update: m_pPad];
}


void GamepadProviderIOS::OnEnterBackground(VariantList *pVList)
{

}

void GamepadProviderIOS::OnEnterForeground(VariantList *PVList)
{

}

