#include "PlatformPrecomp.h"
#include "EmitVirtualKeyComponentAdvanced.h"
#include "BaseApp.h"

EmitVirtualKeyComponentAdvanced::EmitVirtualKeyComponentAdvanced()
{
	SetName("EmitVirtualKey");
}

EmitVirtualKeyComponentAdvanced::~EmitVirtualKeyComponentAdvanced()
{
	
}


void EmitVirtualKeyComponentAdvanced::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();

	m_pKeyCode = &GetVar("keycode")->GetUINT32(); //local to us
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&EmitVirtualKeyComponentAdvanced::OnOverEnd, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&EmitVirtualKeyComponentAdvanced::OnOverStart, this, _1));
	//GetParent()->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&EmitVirtualKeyComponentAdvanced::OnOverStart, this, _1));

	GetVar("disabled")->GetSigOnChanged()->connect(1, boost::bind(&EmitVirtualKeyComponentAdvanced::OnDisabledChanged, this, _1));
}

void EmitVirtualKeyComponentAdvanced::OnRemove()
{ 
	EntityComponent::OnRemove();
}


void EmitVirtualKeyComponentAdvanced::OnDisabledChanged(Variant *pDataObject)
{
	if (pDataObject->GetUINT32() == 1)
	{
		//we just got disabled

	}
}

void EmitVirtualKeyComponentAdvanced::OnOverStart(VariantList *pVList)
{
	
    //LogMsg("Starting finger %d in %s ", pVList->Get(2).GetUINT32(), GetParent()->GetName().c_str());
   
	if (*m_pDisabled == 1) 
	{
		//we're disabled don't start anything
		return;
	}
	VariantList v;
	v.Get(0).Set(*m_pKeyCode);
	v.Get(1).Set(uint32(VIRTUAL_KEY_PRESS)); 
	GetBaseApp()->m_sig_arcade_input(&v);

}

void EmitVirtualKeyComponentAdvanced::OnOverEnd(VariantList *pVList)
{

    //LogMsg("Ending finger %d in %s ", pVList->Get(2).GetUINT32(), GetParent()->GetName().c_str());
    
	VariantList v;
		v.Get(0).Set(*m_pKeyCode);
		v.Get(1).Set(uint32(VIRTUAL_KEY_RELEASE)); 
		GetBaseApp()->m_sig_arcade_input(&v);
	
}

