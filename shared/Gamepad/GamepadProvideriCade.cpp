#include "PlatformPrecomp.h"
#include "GamepadProvideriCade.h"
#include "GamepadiCade.h"
#include "GamepadManager.h"


GamepadProvideriCade::GamepadProvideriCade()
{
	Kill();
}

GamepadProvideriCade::~GamepadProvideriCade()
{
}

bool GamepadProvideriCade::Init()
{
	LogMsg("Initting iCade gamepad provider");
	m_pPad = new GamepadiCade;
	m_pPad->SetProvider(this);
	GetGamepadManager()->AddGamepad(m_pPad);
	return true; //success
}

void GamepadProvideriCade::Kill()
{
}

void GamepadProvideriCade::Update()
{
	
}

void GamepadProvideriCade::OnLostiCadeConnection()
{
	//this can only happen on iOS
	LogMsg("Noticed we lost (or couldn't get) iCade connection.");
	m_sig_failed_to_connect(this);
}
