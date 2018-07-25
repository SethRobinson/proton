#include "PlatformPrecomp.h"

#include "UnderlineRenderComponent.h"
#include "BaseApp.h"

UnderlineRenderComponent::UnderlineRenderComponent()
{
	SetName("UnderlineRender");
}

UnderlineRenderComponent::~UnderlineRenderComponent()
{
}

void UnderlineRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&UnderlineRenderComponent::OnRender, this, _1));
}

void UnderlineRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void UnderlineRenderComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;

	if (*m_pAlpha > 0)
	{
		
		CL_Vec2f vRotationPt = vFinalPos;

		if (*m_pRotation != 0)
		{
			SetupOrtho();
			PushRotationMatrix(*m_pRotation, vRotationPt);
			vFinalPos -= vRotationPt;
		}

		
		//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));
		uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

		DrawLine(color, vFinalPos.x, vFinalPos.y+ m_pSize2d->y+1, vFinalPos.x + m_pSize2d->x, vFinalPos.y+ m_pSize2d->y+1, 2.0f);
	
		if (*m_pRotation != 0)
		{
			PopRotationMatrix();
		}

		//DrawRect(vFinalPos, *m_pSize2d); //useful for debugging our touch hotspot
	}
}