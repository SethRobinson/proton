#include "PlatformPrecomp.h"

#include "RectRenderComponent.h"
#include "util/GLESUtils.h"
#include "BaseApp.h"

RectRenderComponent::RectRenderComponent()
{
	m_pSurf = NULL;
	SetName("RectRender");
}

RectRenderComponent::~RectRenderComponent()
{
}


void RectRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&RectRenderComponent::OnRender, this, _1));
	m_pBorderColor = &GetVarWithDefault("borderColor", Variant(MAKE_RGBA(255,255,255,0)))->GetUINT32();
	m_pVisualStyle = &GetVarWithDefault("visualStyle", uint32(STYLE_NORMAL))->GetUINT32();
	m_pBmpBorderFileName = &GetVar("m_pBmpBorderFileName")->GetString();
	GetVar("bmpBorderFileName")->GetSigOnChanged()->connect(boost::bind(&RectRenderComponent::OnUpdateBmpBorderFileName, this, _1));
}


void RectRenderComponent::OnUpdateBmpBorderFileName(Variant *pVariant)
{
	if (pVariant->GetString().empty())
	{
		m_pSurf = NULL;
	} else
	{
		m_pSurf = GetResourceManager()->GetSurfaceAnim(pVariant->GetString());
		if (m_pSurf)
		{
			if (m_pSurf->GetFramesX() == 1)
			{
				//hasn't been setup?  Let's do it ourself
				m_pSurf->SetupAnim(3,3);

			}
		}
	}

	
}

void RectRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void RectRenderComponent::OnRender(VariantList *pVList)
{

	if (*m_pAlpha > 0.01)
	{
		CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
		uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

		if (GET_ALPHA(color) == 0) return;

		CL_Vec2f vRotationPt = vFinalPos;

		g_globalBatcher.Flush();

		if (*m_pRotation != 0)
		{
			SetupOrtho();
			PushRotationMatrix(*m_pRotation, vRotationPt);
			vFinalPos -= vRotationPt;
		}

		if (m_pSurf)
		{
			CL_Rectf r(vFinalPos, *m_pSize2d);
			
			int borderColor = MAKE_RGBA( GET_RED(*m_pBorderColor), GET_GREEN(*m_pBorderColor), GET_BLUE(*m_pBorderColor),
				255**m_pAlpha);
			DrawFilledBitmapRect(r, color, borderColor,  m_pSurf, true);
		} else
		{
			CL_Rectf r = CL_Rectf(vFinalPos.x, vFinalPos.y, vFinalPos.x+ m_pSize2d->x, vFinalPos.y+m_pSize2d->y); 
			if (*m_pVisualStyle != STYLE_BORDER_ONLY)
			{
				DrawFilledRect(r, color);
			}
			
			if (GET_ALPHA(*m_pBorderColor) > 0)
			{
				DrawRect(r, *m_pBorderColor, 1);
			}

			if (*m_pVisualStyle == STYLE_3D)
			{
				int shadedColor = ColorCombineMix(color, MAKE_RGBA(0,0,0,255), 0.4f);
				DrawLine(shadedColor, vFinalPos.x, vFinalPos.y+m_pSize2d->y, vFinalPos.x+m_pSize2d->x,vFinalPos.y+m_pSize2d->y, 1);
				DrawLine(shadedColor, vFinalPos.x+m_pSize2d->x, vFinalPos.y, vFinalPos.x+m_pSize2d->x,vFinalPos.y+m_pSize2d->y, 1);

				shadedColor = ColorCombineMix(color, MAKE_RGBA(255,255,255 ,255), 0.4f);
				DrawLine(shadedColor, vFinalPos.x, vFinalPos.y, vFinalPos.x,vFinalPos.y+m_pSize2d->y, 1);
				DrawLine(shadedColor, vFinalPos.x, vFinalPos.y, vFinalPos.x+m_pSize2d->x,vFinalPos.y, 1);
			}
		}

		if (*m_pRotation != 0)
		{
			PopRotationMatrix();
		}

		
	}

}