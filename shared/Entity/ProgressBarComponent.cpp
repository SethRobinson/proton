#include "PlatformPrecomp.h"

#include "ProgressBarComponent.h"
#include "util/GLESUtils.h"
#include "BaseApp.h"

ProgressBarComponent::ProgressBarComponent()
{
	SetName("ProgressBar");
}

ProgressBarComponent::~ProgressBarComponent()
{
}

float ProgressBarComponent::GetVisualProgress()
{
	int timePassed = GetBaseApp()->GetGameTick()-m_timeOfLastSet;
	float progress = ( float(timePassed)/ float(*m_pInterpolationTimeMS));

	if (progress < 0) progress = 0;
	if (progress > 1) progress = 1;

	return progress;
}

float ApplyInterpolationToFloat(eInterpolateType type, float f)
{
	switch (type)
	{
	case INTERPOLATE_SMOOTHSTEP: return SMOOTHSTEP(f);
	case INTERPOLATE_LINEAR: return f;
	case INTERPOLATE_EASE_FROM: return EASE_FROM(f);
	case INTERPOLATE_EASE_TO: return EASE_TO(f);
	
	default:
		assert(!"Unhandled interpolation type");
		return f;
	}
	return f;
}

void ProgressBarComponent::OnProgressChanged(Variant *pDataObject)
{
	m_baseProgress = m_baseProgress+  ( ( (*m_pProgressOfLastSet)-m_baseProgress) * ApplyInterpolationToFloat(eInterpolateType(*m_pInterpolateType), GetVisualProgress()));
	*m_pProgressOfLastSet = *m_pProgress;
	m_timeOfLastSet = GetBaseApp()->GetGameTick();
}

void ProgressBarComponent::OnVisualProgressChanged(Variant *pDataObject)
{
	m_baseProgress = *m_pProgress;
	m_timeOfLastSet = GetBaseApp()->GetGameTick();
}

void ProgressBarComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pSurf = NULL;
	m_timeOfLastSet = 0;
	m_baseProgress = 0;

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", 1.0f)->GetFloat();
	m_pType = &GetVarWithDefault("type", uint32(TYPE_HORIZONTAL))->GetUINT32();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();

	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&ProgressBarComponent::OnScaleChanged, this, _1));
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&ProgressBarComponent::OnRender, this, _1));

	//personal vars
	m_pInterpolationTimeMS = &GetVarWithDefault("interpolationTimeMS", Variant(uint32(1000)))->GetUINT32();
	m_pInterpolateType = &GetVarWithDefault("interpolation", Variant(uint32(INTERPOLATE_SMOOTHSTEP)))->GetUINT32();
	m_pBorderColor = &GetVarWithDefault("borderColor", Variant(MAKE_RGBA(255,255,255,0)))->GetUINT32();
	m_pBackgroundColor = &GetVarWithDefault("backgroundColor", Variant(MAKE_RGBA(255,255,255,0)))->GetUINT32();
	m_pProgress = &GetVarWithDefault("progress", Variant(0.0f))->GetFloat();
	m_pVisualProgress = &GetVarWithDefault("visualProgress", Variant(0.0f))->GetFloat();
	m_pProgressOfLastSet = &GetVarWithDefault("progressOfLastSet", Variant(0.0f))->GetFloat(); //don't really want this shared, but too lazy to convert it back
	m_pFileName = &GetVar("fileName")->GetString(); //local to us
	
	m_pFlipX = &GetVar("flipX")->GetUINT32(); 
	m_pFlipY = &GetVar("flipY")->GetUINT32(); 

	//if "fileName" is set, we'll know about it here
	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&ProgressBarComponent::OnFileNameChanged, this, _1));
	GetVar("progress")->GetSigOnChanged()->connect(boost::bind(&ProgressBarComponent::OnProgressChanged, this, _1));
	GetVar("visualProgress")->GetSigOnChanged()->connect(boost::bind(&ProgressBarComponent::OnVisualProgressChanged, this, _1));
	//GetShared()->GetVar("progress")->Set(1.0f); //for testing
}

void ProgressBarComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ProgressBarComponent::OnFileNameChanged(Variant *pDataObject)
{
	m_pSurf = GetResourceManager()->GetSurfaceAnim(pDataObject->GetString());
	if (m_pSurf)
	{
		*m_pSize2d = CL_Vec2f((float)m_pSurf->GetFrameWidth()* m_pScale2d->x, (float)m_pSurf->GetFrameHeight()* m_pScale2d->y);
	}
}

void ProgressBarComponent::OnScaleChanged(Variant *pDataObject)
{
	if (m_pSurf && m_pSurf->IsLoaded())
	{
		*m_pSize2d = CL_Vec2f((float)m_pSurf->GetFrameWidth()* m_pScale2d->x, (float)m_pSurf->GetFrameHeight()* m_pScale2d->y);
	}
}

void ProgressBarComponent::OnRender(VariantList *pVList)
{

	if (*m_pAlpha > 0.01)
	{
		CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
		float progress = m_baseProgress +  ( (*m_pProgress-m_baseProgress) * ApplyInterpolationToFloat(eInterpolateType(*m_pInterpolateType), GetVisualProgress()) );
		if (progress == 0) return;
		*m_pVisualProgress = progress; //save it so outsiders can check and see what we're displaying
		uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);
		uint32 borderColor = ColorCombine(*m_pBorderColor, *m_pColorMod, *m_pAlpha);
		uint32 backgroundColor = ColorCombine(*m_pBackgroundColor, *m_pColorMod, *m_pAlpha);

		//here, we assume the progress bar is horizontal and going left to right.. later, add a var to control this if needed and do a switch statement here

		float progressX = 1;
		float progressY = 1;

		if (*m_pType == TYPE_HORIZONTAL)
		{
			progressX = progress;
		} else
		{
			progressY = progress;
		}

			if (m_pSurf && m_pSurf->IsLoaded())
			{
				//bitmap version of a progress bar

				rtRectf src = rtRectf(0,0,progressX*m_pSurf->GetFrameWidth(), progressY*m_pSurf->GetFrameHeight());
				
				assert(*m_pFlipX == 0 && "Need to add support for flipx with bitmaps here");
				rtRectf dst = src;
				dst.AdjustPosition(vFinalPos.x, vFinalPos.y);
				m_pSurf->BlitEx(dst, src, color); 

			} else
			{
				//manual rectangle version of a progress bar
				CL_Rectf r = CL_Rectf(vFinalPos.x, vFinalPos.y, vFinalPos.x+ (m_pSize2d->x), vFinalPos.y+m_pSize2d->y); 

				//flip y?  //default is yes, which is bottom up

				if (GET_ALPHA(backgroundColor) > 0)
				{
					DrawFilledRect(r, backgroundColor);
					if (GET_ALPHA(borderColor) > 0)
					{
						DrawRect(r, borderColor, 1);
					}
				}
				r = CL_Rectf(vFinalPos.x, vFinalPos.y, vFinalPos.x+ (m_pSize2d->x*progressX), vFinalPos.y+m_pSize2d->y*progressY); 

				if (*m_pType == TYPE_VERTICAL && *m_pFlipY == 0)
				{
					r.bottom = vFinalPos.y + m_pSize2d->y;
					r.top = r.bottom - m_pSize2d->y*progressY;
				} else 	if (*m_pType == TYPE_HORIZONTAL && *m_pFlipX == 1)
				{
					r.right = vFinalPos.x + m_pSize2d->x;
					r.left = r.right - m_pSize2d->x*progressX;
				}
			
				if (*m_pType == TYPE_VERTICAL && *m_pFlipY == 0)
				{
					r.bottom = vFinalPos.y + m_pSize2d->y;
					r.top = r.bottom - m_pSize2d->y*progressY;
				} else 	if (*m_pType == TYPE_HORIZONTAL && *m_pFlipX == 1)
				{
					r.right = vFinalPos.x + m_pSize2d->x;
					r.left = r.right - m_pSize2d->x*progressX;
				}


				if (GET_ALPHA(color) > 0)
				{
					DrawFilledRect(r, color);
				}

				if (GET_ALPHA(borderColor) > 0)
				{
					DrawRect(r, borderColor, 1);
				}
			}
		
	
		
	}

}