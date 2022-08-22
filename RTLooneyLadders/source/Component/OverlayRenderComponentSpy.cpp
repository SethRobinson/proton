#include "PlatformPrecomp.h"

#include "OverlayRenderComponentSpy.h"
#include "BaseApp.h"

OverlayRenderComponentSpy::OverlayRenderComponentSpy()
{
	m_bDeleteSurface = false;
	SetName("OverlayRender");
}

OverlayRenderComponentSpy::~OverlayRenderComponentSpy()
{
	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}
}

void OverlayRenderComponentSpy::SetSurface( SurfaceAnim *pSurf, bool bDeleteSurface )
{
	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}
	m_pTex = pSurf;
	m_bDeleteSurface = bDeleteSurface;

	if (m_pTex)
	{
		*m_pSize2d = CL_Vec2f((float)m_pTex->GetFrameWidth()* m_pScale2d->x, (float)m_pTex->GetFrameHeight()* m_pScale2d->y);
	}
}

void OverlayRenderComponentSpy::OnFileNameChanged(Variant *pDataObject)
{
	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}
	
	m_pTex = GetResourceManager()->GetSurfaceAnim(pDataObject->GetString());
	
	if (m_pTex)
	{
		*m_pSize2d = CL_Vec2f((float)m_pTex->GetFrameWidth()* m_pScale2d->x, (float)m_pTex->GetFrameHeight()* m_pScale2d->y);
	}
}

void OverlayRenderComponentSpy::OnScaleChanged(Variant *pDataObject)
{
	if (m_pTex && m_pTex->IsLoaded())
	{
		*m_pSize2d = CL_Vec2f((float)m_pTex->GetFrameWidth()* m_pScale2d->x, (float)m_pTex->GetFrameHeight()* m_pScale2d->y);
	}
}

void OverlayRenderComponentSpy::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	m_pTex = NULL;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetShared()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees
	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	
	m_pFrameX = &GetVar("frameX")->GetUINT32(); //applicable if SetupAnim was used
	m_pFrameY = &GetVar("frameY")->GetUINT32(); //applicable if SetupAnim was used
	
	m_pFlipX = &GetVar("flipX")->GetUINT32(); 
	m_pFlipY = &GetVar("flipY")->GetUINT32(); 

	m_pFileName = &GetVar("fileName")->GetString(); //local to us
	
	//any post var data you want to send, must send it before the Init
	GetFunction("SetupAnim")->sig_function.connect(1, boost::bind(&OverlayRenderComponentSpy::SetupAnim, this, _1));

	//if "fileName" is set, we'll know about it here
	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&OverlayRenderComponentSpy::OnFileNameChanged, this, _1));
	pEnt->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&OverlayRenderComponentSpy::OnScaleChanged, this, _1));
	
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&OverlayRenderComponentSpy::OnRender, this, _1));
}


void OverlayRenderComponentSpy::SetupAnim(VariantList *pVList)
{
	//send in framesx, framesy as uint32's
	if (!m_pTex)
	{
		LogError("OverlayRenderComponentSpy::SetupAnim: Must load an image first");
		assert(0);
		return;
	}
	
	GetVar("totalFramesX")->Set(pVList->m_variant[0].GetUINT32());
	GetVar("totalFramesY")->Set(pVList->m_variant[1].GetUINT32());
	
	m_pTex->SetupAnim(pVList->m_variant[0].GetUINT32(), pVList->m_variant[1].GetUINT32());
	*m_pSize2d = CL_Vec2f((float)m_pTex->GetFrameWidth()* m_pScale2d->x, (float)m_pTex->GetFrameHeight()* m_pScale2d->y);
}

void OverlayRenderComponentSpy::OnRemove()
{
	EntityComponent::OnRemove();
}

void OverlayRenderComponentSpy::OnRender(VariantList *pVList)
{

	if (m_pTex && m_pTex->IsLoaded() && *m_pAlpha > 0.01)
	{
		CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
		vFinalPos = *m_pPos2d;

		CL_Vec2f vScale = *GetBuilding()->GetScale2D();
		vScale.x *= m_pScale2d->x;
		vScale.y *= m_pScale2d->y;

		//vFinalPos.y += 160;
		vFinalPos = WorldToScreenPos(vFinalPos);
		unsigned int finalColor = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

		if (GET_ALPHA(finalColor) == 0) return;

		if (vScale.x != 1 || vScale.y != 1 || *m_pAlignment != ALIGNMENT_UPPER_CENTER)
		{
			CL_Vec2f vRotationPt = vFinalPos+m_pTex->GetFrameSize()/2;
			
			if (vFinalPos.y < -m_pSize2d->y*vScale.y) return;
			if (vFinalPos.y-(m_pSize2d->y*vScale.y) > GetOrthoRenderSizeYf()) return;
			
			if (vScale.x != 0 && vScale.y != 0)
			{
				m_pTex->BlitScaledAnim(vFinalPos.x, vFinalPos.y,  *m_pFrameX, *m_pFrameY,*GetBuilding()->GetScale2D(), (eAlignment)*m_pAlignment, finalColor, *m_pRotation, vRotationPt,
					*m_pFlipX != 0, *m_pFlipY != 0);
			}
		} else
		{
			CL_Vec2f vRotationPt = vFinalPos+m_pTex->GetFrameSize()/2;
			if (vFinalPos.y < -m_pSize2d->y) return;
			if (vFinalPos.y > GetOrthoRenderSizeYf()) return;
			
			
			
			m_pTex->BlitAnim(vFinalPos.x, vFinalPos.y, *m_pFrameX, *m_pFrameY, finalColor, *m_pRotation, vRotationPt);
		}
	}

}

