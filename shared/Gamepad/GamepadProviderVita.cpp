#include "PlatformPrecomp.h"
#include "GamepadProviderVita.h"
#include "GamepadVita.h"
#include "GamepadManager.h"

GamepadProviderVita::GamepadProviderVita()
{
    
}

GamepadProviderVita::~GamepadProviderVita()
{

}

bool GamepadProviderVita::Init()
{
    LogMsg("Initting Vita gamepad provider");

    GamepadVita* pPad = new GamepadVita;
	pPad->SetProvider(this);
	GetGamepadManager()->AddGamepad(pPad);    

    return true;
}

void GamepadProviderVita::Kill()
{

}

void GamepadProviderVita::Update()
{

}