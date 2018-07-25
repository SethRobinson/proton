#include "PlatformPrecomp.h"

#include "OverlayRenderComponent.h"
#include "BaseApp.h"

OverlayRenderComponent::OverlayRenderComponent()
{
	m_bDeleteSurface = false;
	SetName("OverlayRender");
}

OverlayRenderComponent::~OverlayRenderComponent()
{

	if (GetVar("unloadImageAtOnKill")->GetUINT32() != 0)
	{
		if (m_pTex)
		{
			m_pTex->Kill();
		}
	}

	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}

}

void OverlayRenderComponent::UpdateSizeVar()
{
	if (m_pTex && m_pTex->IsLoaded())
	{
		GetParent()->GetVar("size2d")->Set(m_pTex->GetFrameSize() * (*m_pScale2d));
	}
}

void OverlayRenderComponent::UpdateFrameSizeVar()
{
	if (m_pTex && m_pTex->IsLoaded())
	{
		GetVar("frameSize2d")->Set(m_pTex->GetFrameSize());
	} else {
		GetVar("frameSize2d")->Set(0.0f, 0.0f);
	}
}

void OverlayRenderComponent::SetSurface( SurfaceAnim *pSurf, bool bDeleteSurface )
{
	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}
	m_pTex = pSurf;
	m_bDeleteSurface = bDeleteSurface;

	UpdateSizeVar();
	UpdateFrameSizeVar();
}

void OverlayRenderComponent::OnFileNameChanged(Variant *pDataObject)
{
	if (m_bDeleteSurface)
	{
		m_bDeleteSurface = false;
		SAFE_DELETE(m_pTex);
	}
	
	bool bAddBasePath = true;

	if (GetVar("dontAddBasePath")->GetUINT32() != 0)
	{
		bAddBasePath = false;
	}

	m_pTex = GetResourceManager()->GetSurfaceAnim(pDataObject->GetString(), bAddBasePath);
	
	UpdateSizeVar();
	UpdateFrameSizeVar();
}

void OverlayRenderComponent::OnScaleChanged(Variant *pDataObject)
{
	UpdateSizeVar();
}

void OverlayRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	m_pTex = NULL;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees
	m_pRotationCenter = &GetParent()->GetVarWithDefault("rotationCenter", Variant(0.5f, 0.5f))->GetVector2();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();
	
	m_pFrameX = &GetVar("frameX")->GetUINT32(); //applicable if SetupAnim was used
	m_pFrameY = &GetVar("frameY")->GetUINT32(); //applicable if SetupAnim was used
	
	m_pFlipX = &GetVar("flipX")->GetUINT32(); 
	m_pFlipY = &GetVar("flipY")->GetUINT32(); 

	m_pFileName = &GetVar("fileName")->GetString(); //local to us
	
	GetVarWithDefault("frameSize2d", Variant(0.0f, 0.0f));

	//any post var data you want to send, must send it before the Init
	GetFunction("SetupAnim")->sig_function.connect(1, boost::bind(&OverlayRenderComponent::SetupAnim, this, _1));

	//if "fileName" is set, we'll know about it here
	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&OverlayRenderComponent::OnFileNameChanged, this, _1));
	pEnt->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&OverlayRenderComponent::OnScaleChanged, this, _1));
	
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&OverlayRenderComponent::OnRender, this, _1));
}


void OverlayRenderComponent::SetupAnim(VariantList *pVList)
{
	//send in framesx, framesy as uint32's
	if (!m_pTex)
	{
		LogError("OverlayRenderComponent::SetupAnim: Must load an image first");
		assert(0);
		return;
	}
	
	GetVar("totalFramesX")->Set(pVList->m_variant[0].GetUINT32());
	GetVar("totalFramesY")->Set(pVList->m_variant[1].GetUINT32());
	
	m_pTex->SetupAnim(pVList->m_variant[0].GetUINT32(), pVList->m_variant[1].GetUINT32());

	UpdateSizeVar();
	UpdateFrameSizeVar();
}

void OverlayRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void OverlayRenderComponent::OnRender(VariantList *pVList)
{
	if (*m_pVisible == 0) return;

	if (m_pTex && m_pTex->IsLoaded() && *m_pAlpha > 0.01)
	{
		CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	
		unsigned int finalColor = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

		if (GET_ALPHA(finalColor) == 0) return;

		if (vFinalPos.y < -m_pSize2d->y && *m_pRotation == 0) return; //if rotation is enabled, we don't do this early exit check as it could be incorrect
		if (vFinalPos.y > GetOrthoRenderSizeYf() && *m_pRotation == 0) return; //if rotation is enabled, we don't do this early exit check as it could be incorrect
		CL_Vec2f vRotationPt = vFinalPos;
		
		vRotationPt.x += (m_pTex->GetFrameSize().x* (m_pScale2d->x)) * m_pRotationCenter->x;
		vRotationPt.y += (m_pTex->GetFrameSize().y* (m_pScale2d->y)) * m_pRotationCenter->y;
		
		
		if (m_pScale2d->x != 1 || m_pScale2d->y != 1 || *m_pFlipX != 0 || *m_pFlipY != 0)
		{
			
			if (m_pScale2d->x != 0 && m_pScale2d->y != 0)
			{
				m_pTex->BlitScaledAnim(vFinalPos.x, vFinalPos.y,  *m_pFrameX, *m_pFrameY,*m_pScale2d, ALIGNMENT_UPPER_LEFT, finalColor, *m_pRotation, vRotationPt,
					*m_pFlipX != 0, *m_pFlipY != 0);
			}
		} else
		{
			m_pTex->BlitAnim(vFinalPos.x, vFinalPos.y, *m_pFrameX, *m_pFrameY, finalColor, *m_pRotation, vRotationPt);
		}
	}

}
