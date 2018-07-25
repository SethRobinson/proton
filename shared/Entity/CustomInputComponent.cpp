#include "PlatformPrecomp.h"
#include "CustomInputComponent.h"
#include "EntityUtils.h"

CustomInputComponent::CustomInputComponent()
{///****************
	SetName("CustomInput");
}

CustomInputComponent::~CustomInputComponent()
{
}

void CustomInputComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//first setup to listen to keyboard messages
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&CustomInputComponent::OnInput, this, _1)); //used for keyboard keys on Win
	GetParent()->GetFunction("OnInputRaw")->sig_function.connect(1, boost::bind(&CustomInputComponent::OnInputRaw, this, _1));

	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();

	m_pKeys = &GetVar("keys")->GetString(); //local to us
	m_pKeyCode = &GetVar("keycode")->GetUINT32(); //local to us

	//if keys and keycode are not set, it will be activated on "any key"
	//if either are set, it will only activate if those key/keys are hit.

	//you should attach to its "OnActivated" function to know when a key is hit
}

void CustomInputComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void CustomInputComponent::OnActivated()
{
	VariantList v;
	GetFunction("OnActivated")->sig_function(&v);
}

void CustomInputComponent::OnReleased()
{
	VariantList v;
	GetFunction("OnReleased")->sig_function(&v);
}

bool CustomInputComponent::isKeyAcceptable(uint32 keycode) const
{
	bool accepted = false;

	if (*m_pKeyCode != 0 && keycode == *m_pKeyCode)
	{
		accepted = true;
	}
	else if (m_pKeys->find((char)keycode) != string::npos)
	{
		accepted = true;
	}
	else if (*m_pKeyCode == 0 && m_pKeys->empty())
	{
		accepted = true;
	}

	return accepted;
}

void CustomInputComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset, 2 = char, 3 reserved for filtering control messages

	if (*m_pDisabled == 1 || eMessageType(int(pVList->Get(0).GetFloat())) != MESSAGE_TYPE_GUI_CHAR)
	{
		return;
	}

	if (isKeyAcceptable(pVList->Get(2).GetUINT32()))
	{
		OnActivated();
	}
}

void CustomInputComponent::OnInputRaw( VariantList *pVList )
{
	//0 = keycode, 1 = pressed or released

	if (*m_pDisabled == 1 || pVList->Get(1).GetUINT32() != VIRTUAL_KEY_RELEASE)
	{
		return;
	}

	if (isKeyAcceptable(pVList->Get(0).GetUINT32()))
	{
		OnReleased();
	}
}
