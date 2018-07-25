#include "PlatformPrecomp.h"
#include "EmitVirtualKeyComponent.h"
#include "BaseApp.h"

EmitVirtualKeyComponent::EmitVirtualKeyComponent()
{
	SetName("EmitVirtualKey");
}

EmitVirtualKeyComponent::~EmitVirtualKeyComponent()
{

}


void EmitVirtualKeyComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pKeyCode = &GetVar("keycode")->GetUINT32(); //local to us
	GetParent()->GetFunction("OnTouchEnd")->sig_function.connect(1, boost::bind(&EmitVirtualKeyComponent::OnTouchEnd, this, _1));
	GetParent()->GetFunction("OnTouchStart")->sig_function.connect(1, boost::bind(&EmitVirtualKeyComponent::OnTouchStart, this, _1));
	
}

void EmitVirtualKeyComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void EmitVirtualKeyComponent::OnTouchStart(VariantList *pVList)
{
	VariantList v;
	v.Get(0).Set(*m_pKeyCode);
	v.Get(1).Set(uint32(VIRTUAL_KEY_PRESS)); 
	GetBaseApp()->m_sig_arcade_input(&v);

}

void EmitVirtualKeyComponent::OnTouchEnd(VariantList *pVList)
{
	VariantList v;
	v.Get(0).Set(*m_pKeyCode);
	v.Get(1).Set(uint32(VIRTUAL_KEY_RELEASE)); 
	GetBaseApp()->m_sig_arcade_input(&v);


}

