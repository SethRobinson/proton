#include "PlatformPrecomp.h"

#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT

#include "GamepadProvider60Beat.h"
#include "Gamepad60Beat.h"
#include "GamepadManager.h"
#import "SBJoystick.h"
#import "GamepadProvider60BeatNative.h"

id m_padDriver = nil;


GamepadProvider60Beat::GamepadProvider60Beat()
{
	Kill();
}

GamepadProvider60Beat::~GamepadProvider60Beat()
{
}

bool GamepadProvider60Beat::Init()
{

#ifndef RT_IOS_60BEAT_GAMEPAD_SUPPORT
    assert(!"You really should define RT_IOS_60BEAT_GAMEPAD_SUPPORT in your xCode project, so the FMOD SoundManager will do mixing in a compatible way with 60beat's stuff");
#endif
	
    LogMsg("Initting 60Beat gamepad provider");
    [SBJoystick sharedInstance].loggingEnabled = YES;
    [SBJoystick sharedInstance].enabled = YES;
    
    assert(m_padDriver == 0);
    m_padDriver = [SBeatPadNative new];
    [m_padDriver Start];
    
    m_pPad = new Gamepad60Beat;
    m_pPad->SetProvider(this);
    GetGamepadManager()->AddGamepad(m_pPad);
    
    //get notified on suspend/resume    
    GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&GamepadProvider60Beat::OnEnterForeground, this, _1));
	GetBaseApp()->m_sig_enterbackground.connect(1, boost::bind(&GamepadProvider60Beat::OnEnterBackground, this, _1));

    return true; //success
}

void GamepadProvider60Beat::Kill()
{
	[SBJoystick sharedInstance].enabled = NO;
    [m_padDriver release];
    m_padDriver = nil;
}


void GamepadProvider60Beat::Update()
{
	[m_padDriver Update: m_pPad];
}

CL_Vec2f GamepadProvider60Beat::GetLeftStickPos()
{
    
    return CL_Vec2f([m_padDriver GetLeftStickPos].x,[m_padDriver GetLeftStickPos].y);
}

CL_Vec2f GamepadProvider60Beat::GetRightStickPos()
{
    
     return CL_Vec2f([m_padDriver GetRightStickPos].x,[m_padDriver GetRightStickPos].y);
}

void GamepadProvider60Beat::OnEnterBackground(VariantList *pVList)
{
    [[SBJoystick sharedInstance] beginInterruption];
}

void GamepadProvider60Beat::OnEnterForeground(VariantList *PVList)
{
    [[SBJoystick sharedInstance] endInterruption];
}

#endif