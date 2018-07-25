#include "PlatformPrecomp.h"
#include "SmartPositionerComponent.h"
#include "EntityUtils.h"

SmartPositionerComponent::SmartPositionerComponent()
{
}

SmartPositionerComponent::~SmartPositionerComponent()
{
}


void SmartPositionerComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPosScreenPercent2D = &GetParent()->GetVar("posScreenPercent2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	//-1,-1 means "not used" below
	m_pScaleScreenPercent2D = &GetParent()->GetVarWithDefault("scaleScreenPercent2d", CL_Vec2f(-1.0f, -1.0f))->GetVector2();
	m_pAspectMode = &GetVarWithDefault("aspectMode", (uint32)ASPECT_NONE)->GetUINT32();
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();

	//register to get notified when certain things change
	GetParent()->GetVar("scaleScreenPercent2d")->GetSigOnChanged()->connect(boost::bind(&SmartPositionerComponent::OnScaleScreenPercentChanged, this, _1));
	GetParent()->GetVar("posScreenPercent2d")->GetSigOnChanged()->connect(boost::bind(&SmartPositionerComponent::OnPosScreenPercentChanged, this, _1));

	//register to get updated every frame
	//GetBaseApp()->m_sig_render.connect(1, boost::bind(&FocusRenderComponent::OnRender, this, _1));

	GetBaseApp()->m_sig_onScreenSizeChanged.connect(1, boost::bind(&SmartPositionerComponent::OnScreenSizeChanged, this));
}

void SmartPositionerComponent::OnScreenSizeChanged()
{
	CalculatePosition();
}

void SmartPositionerComponent::OnScaleScreenPercentChanged(Variant *pVariant)
{
	CalculatePosition();
}

void SmartPositionerComponent::OnPosScreenPercentChanged(Variant *pVariant)
{
	CalculatePosition();
}

void SmartPositionerComponent::CalculatePosition()
{
	CL_Vec2f vSize;
	
	if (*m_pScaleScreenPercent2D != CL_Vec2f(-1, -1))
	{
		
		vSize = *m_pSize2d;

		if (m_pScaleScreenPercent2D->x != -1)
			vSize.x = GetScreenSizeXf()* m_pScaleScreenPercent2D->x;
		
		if (m_pScaleScreenPercent2D->y != -1)
			vSize.y = GetScreenSizeYf()* m_pScaleScreenPercent2D->y;
		
		if (*m_pAspectMode == ASPECT_NONE)
		{
			*m_pSize2d = vSize;
		}
		else
		{
			//pull aspect ratio from image
			*m_pSize2d = GetImageSize2DEntity(GetParent());
			if (*m_pSize2d == CL_Vec2f(-1, -1))
			{
				//uh oh...
				*m_pSize2d = vSize;
			}
		}

		*m_pScale2d = CL_Vec2f(1.0f, 1.0f);
		
		EntitySetScaleBySizeAndAspectMode(GetParent(), vSize, (eAspect)*m_pAspectMode);

	}
	else
	{
		//not scaling by screen size, use normal scale
	}

	CL_Vec2f vPos = GetScreenSize();
	
	if (m_pPosScreenPercent2D->x != -1)
	{
		vPos.x *= m_pPosScreenPercent2D->x;
	}
	else
	{
		vPos.x = m_pPos2d->x;
	}
		
	if (m_pPosScreenPercent2D->y != -1)
	{
		vPos.y *= m_pPosScreenPercent2D->y;
	}
	else
	{
		vPos.y = m_pPos2d->y;
	}

	if (*m_pPosScreenPercent2D != CL_Vec2f(-1,-1))
	{
		SetPos2DEntity(GetParent(), vPos);
	}
	SetSize2DEntity(GetParent(), *m_pSize2d); //it's already set, but allow other things to get notification

}

void SmartPositionerComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void SmartPositionerComponent::SetSmartScale(CL_Vec2f vScalePercents, eAspect aspectMode)
{
	*m_pAspectMode = aspectMode;
	GetParent()->GetVar("scaleScreenPercent2d")->Set(vScalePercents);
}

SmartPositionerComponent * AddSmartPositioner(Entity *pEnt, eAlignment align, float xPercent, float yPercent)
{
	SmartPositionerComponent *pSmartPosComp = new SmartPositionerComponent;
	pEnt->AddComponent(pSmartPosComp);
	SetAlignmentEntity(pEnt, align);
	pSmartPosComp->GetParent()->GetVar("posScreenPercent2d")->Set(CL_Vec2f(xPercent, yPercent));
	
	return pSmartPosComp;
}
