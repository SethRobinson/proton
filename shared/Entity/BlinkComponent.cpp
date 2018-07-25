#include "PlatformPrecomp.h"
#include "BlinkComponent.h"
#include "Entity/EntityUtils.h"


BlinkComponent::BlinkComponent()
{
	SetName("Blink");
	m_timeOfLastBlink = 0;
}

BlinkComponent::~BlinkComponent()
{

}

void BlinkComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();
	m_pBlinkSpeedMS = &GetParent()->GetVarWithDefault("blinkSpeedMS", uint32(300))->GetUINT32();

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&BlinkComponent::OnUpdate, this, _1));
}

void BlinkComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void BlinkComponent::OnUpdate(VariantList *pVList)
{
	if (m_timeOfLastBlink+*m_pBlinkSpeedMS<GetTick())
	{
		GetParent()->GetVar("visible")->Set(uint32(!GetParent()->GetVar("visible")->GetUINT32()));
		m_timeOfLastBlink = GetTick();
	}
	
}