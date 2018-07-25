#include "PlatformPrecomp.h"
#include "SelectButtonWithCustomInputComponent.h"
#include "EntityUtils.h"

SelectButtonWithCustomInputComponent::SelectButtonWithCustomInputComponent()
{
	SetName("SelectButtonWithCustomInput");
}

SelectButtonWithCustomInputComponent::~SelectButtonWithCustomInputComponent()
{
}

void SelectButtonWithCustomInputComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//first setup to listen to keyboard messages
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&SelectButtonWithCustomInputComponent::OnInput, this, _1)); //used for keyboard keys on Win
	GetBaseApp()->m_sig_raw_keyboard.connect(1, boost::bind(&SelectButtonWithCustomInputComponent::OnInputRaw, this, _1)); //used for keyboard keys on Win
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();

	m_pKeys = &GetVar("keys")->GetString(); //local to us
	m_pKeyCode = &GetVar("keycode")->GetUINT32(); //local to us
	
	//if keys and keycode are not set, it will be activated on "any key"
	//if either are set, it will only activate if those key/keys are hit.

	//you should attach to its "OnActivated" function to know whey a key is hit
}

void SelectButtonWithCustomInputComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void SelectButtonWithCustomInputComponent::ClickButton()
{
	FakeClickAnEntity(GetParent());
}

void SelectButtonWithCustomInputComponent::OnInputRaw( VariantList *pVList )
{

	if (*m_pDisabled == 1) return;
	bool bDown = pVList->Get(1).GetUINT32() != 0;
	if (!bDown)	 return;

	int key = pVList->Get(0).GetUINT32();
	//LogMsg("Got raw char %d, down is %d", key, int(bDown));
	
	if (*m_pKeyCode != 0)
	{
		if (key == *m_pKeyCode)
		{
			ClickButton();
			return;
		}
	}
}


void SelectButtonWithCustomInputComponent::OnInput( VariantList *pVList )
{
	
	if (*m_pDisabled == 1) return;
	
	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	
	case MESSAGE_TYPE_GUI_CHAR_RAW:
		#ifdef _DEBUG
				LogMsg("Got raw char %d", pVList->Get(2).GetUINT32());
		#endif
		break;

	case MESSAGE_TYPE_GUI_CHAR:
		{
		if (*m_pKeyCode != 0)
		{
			if (pVList->Get(2).GetUINT32() == *m_pKeyCode)
			{
				ClickButton();
				return;
			}
		
			if (m_pKeys->empty()) return;
		}
		char c = (char)pVList->Get(2).GetUINT32();
	

		if (m_pKeys->size() > 0)
		{
			
			bool bFound = false;
			for (unsigned int i=0; i < m_pKeys->size(); i++)
			{
			
				if (c == m_pKeys->at(i))
				{
					bFound = true;
					break;
				}

			}

			if (!bFound)
			{
				return;//none of the accepted keys are here.  Bye
			}
		}
		ClickButton();	
		}
		break;

        default:;
		//LogMsg("Got msg %d", int(pVList->Get(0).GetFloat()));

	}	

}
