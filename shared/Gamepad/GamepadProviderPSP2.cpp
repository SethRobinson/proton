#include "PlatformPrecomp.h"
#include "GamepadProviderPSP2.h"
#include "GamepadPSP2.h"
#include "GamepadManager.h"

GamepadProviderPSP2::GamepadProviderPSP2()
{
    
}

GamepadProviderPSP2::~GamepadProviderPSP2()
{

}

bool GamepadProviderPSP2::Init()
{
    LogMsg("Initting Vita gamepad provider");

    GamepadPSP2* pPad = new GamepadPSP2;
	pPad->SetProvider(this);
	GetGamepadManager()->AddGamepad(pPad);    

    return true;
}

void GamepadProviderPSP2::Kill()
{

}

void GamepadProviderPSP2::Update()
{

}