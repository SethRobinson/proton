#include "PlatformPrecomp.h"
#include "GamepadProviderMoga.h"
#include "GamepadMoga.h"
#include "GamepadManager.h"


GamepadProviderMoga::GamepadProviderMoga()
{
	Kill();
}

GamepadProviderMoga::~GamepadProviderMoga()
{
}

bool GamepadProviderMoga::Init()
{
	LogMsg("Initting Moga gamepad provider");
	m_pPad = new GamepadMoga;
	m_pPad->SetProvider(this);
	GetGamepadManager()->AddGamepad(m_pPad);
	return true; //success
}

void GamepadProviderMoga::Kill()
{
}

void GamepadProviderMoga::Update()
{

}
