#include "PlatformPrecomp.h"
#include "TemplateComponent.h"
#include "Entity/EntityUtils.h"


TemplateComponent::TemplateComponent()
{
	SetName("Template");
}

TemplateComponent::~TemplateComponent()
{

}


void TemplateComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	
	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TemplateComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&TemplateComponent::OnUpdate, this, _1));
}

void TemplateComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TemplateComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void TemplateComponent::OnUpdate(VariantList *pVList)
{
	
}