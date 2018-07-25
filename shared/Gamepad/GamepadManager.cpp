#include "PlatformPrecomp.h"
#include "GamepadManager.h"

GamepadManager::GamepadManager()
{
	m_defaultGamepadID = GAMEPAD_ID_NONE;
}

GamepadManager::~GamepadManager()
{
	list<GamepadProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		(*itor)->Kill();
		delete *itor;
	}
	m_providers.clear();

	for (uint32 i=0; i < m_gamepads.size(); i++)
	{
		SAFE_DELETE(m_gamepads[i]);
	}
	//I know we're exiting and this kind of thing doesn't matter, but I plan to move it somewhere else later and don't want to
	//miss it -Seth
	m_gamepads.clear();
	m_defaultGamepadID = GAMEPAD_ID_NONE; 

}

GamepadProvider * GamepadManager::AddProvider( GamepadProvider *provider )
{

	if (!provider->Init())
	{
		LogError("Unable to init gamepad provider %s, killing it", provider->GetName().c_str());
		SAFE_DELETE(provider);
		return NULL;
	} else
	{
		m_providers.push_back(provider);
		LogMsg("Gamepad provider %s initialized.", provider->GetName().c_str());
	}

	return provider;
}

void GamepadManager::Update()
{
	list<GamepadProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		(*itor)->Update();
	}

	for (uint32 i=0; i < m_gamepads.size(); i++)
	{
		m_gamepads[i]->Update();
	}
}

void GamepadManager::AddGamepad( Gamepad * pad )
{
	if (pad->Init()) 
	{
		m_gamepads.push_back(pad);
		pad->SetID((eGamepadID) (m_gamepads.size()-1));
		LogMsg("Located gamepad %s", pad->GetName().c_str());
		if (m_defaultGamepadID == GAMEPAD_ID_NONE)
		{
			m_defaultGamepadID = pad->GetID();
		}
	} else
	{
		LogMsg("Unable to add pad %s", pad->GetName().c_str());
		SAFE_DELETE(pad);
	}
	
}

Gamepad * GamepadManager::GetDefaultGamepad()
{
	if (m_defaultGamepadID == GAMEPAD_ID_NONE) return NULL;
	return m_gamepads[m_defaultGamepadID];
}

eGamepadID GamepadManager::GetDefaultGamepadID()
{
	return m_defaultGamepadID;
}

void GamepadManager::ResetGamepads()
{
	for (uint32 i=0; i < m_gamepads.size(); i++)
	{
		m_gamepads[i]->Reset();
	}
}

Gamepad * GamepadManager::GetUnusedGamepad()
{
	//ask for the best one first

	if (m_defaultGamepadID != GAMEPAD_ID_NONE)
	{
		if (!GetDefaultGamepad()->GetIsUsed())
		{
			GetDefaultGamepad()->SetIsUsed(true);
			return GetDefaultGamepad();
		}
	}

	//no default or it's being used, scan the rest

	for (uint32 i=0; i < m_gamepads.size(); i++)
	{
		if (!m_gamepads[i]->GetIsUsed())
		{
			m_gamepads[i]->SetIsUsed(true);
			return m_gamepads[i];
		}
	}

	return NULL;
}

bool GamepadManager::RemoveProviderByName( string name )
{
	list<GamepadProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		if ( (*itor)->GetName() == name)
		{
			//remove all gamepads associated with this provider
			RemoveGamepadsByProvider((*itor));
			LogMsg("Gamepad provider %s removed", (*itor)->GetName().c_str());
			(*itor)->Kill();
            delete *itor;
			m_providers.erase(itor);
			return true;
		}
	}

	return false;
}

void GamepadManager::RemoveGamepadsByProvider( GamepadProvider *provider )
{
	for (uint32 i=0; i < m_gamepads.size(); i++)
	{
		if (m_gamepads[i]->GetProvider() == provider)
		{
			if (i == m_defaultGamepadID)
			{
				m_defaultGamepadID = GAMEPAD_ID_NONE;
			}
			delete m_gamepads[i];
			m_gamepads.erase(m_gamepads.begin()+i);
			i--;
		}
	}
}

Gamepad * GamepadManager::GetGamepad( eGamepadID id )
{
	if ((int)id < 0 || (int)id >= (int)m_gamepads.size() )
	{
		return NULL;
	}

	return m_gamepads[id];
}

GamepadProvider * GamepadManager::GetProviderByName( string name )
{
	list<GamepadProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		if ( (*itor)->GetName() == name)
		{
			return (*itor);
		}
	}

	return NULL;
}

void GamepadManager::SetDefaultGamepad( Gamepad *pPad )
{
	m_defaultGamepadID = pPad->GetID();
}