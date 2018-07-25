#include "PlatformPrecomp.h"

#include "TextRenderComponent.h"
#include "BaseApp.h"

TextRenderComponent::TextRenderComponent()
{
	SetName("TextRender");
}

TextRenderComponent::~TextRenderComponent()
{
}

void TextRenderComponent::OnTextChanged(Variant *pDataObject)
{
	rtRectf rt;
	GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(&rt, *m_pText, m_pScale2d->x);
	GetParent()->GetVar("size2d")->Set(CL_Vec2f(rt.GetWidth(), rt.GetHeight()));
}

void TextRenderComponent::OnScaleChanged(Variant *pDataObject)
{
	//OPTIMIZE: this is lame, recomputing everything, but whatever
	OnTextChanged(NULL);
}

void TextRenderComponent::OnFontChanged(Variant *pDataObject)
{
	//OPTIMIZE: this is lame, recomputing everything, but whatever
	OnTextChanged(NULL);
}

void TextRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	m_pShadowColor = &GetVarWithDefault("shadowColor", Variant(MAKE_RGBA(0,0,0,0)))->GetUINT32();

	//our own stuff

	m_pEffectPower = &GetVarWithDefault("effectPower", Variant(8.0f))->GetFloat();
	m_pStyle = &GetVarWithDefault("style", Variant(uint32(STYLE_NORMAL)))->GetUINT32();

	//if "fileName" is set, we'll know about it here
	m_pText = &GetVar("text")->GetString(); //local to us
	GetVar("text")->GetSigOnChanged()->connect(1, boost::bind(&TextRenderComponent::OnTextChanged, this, _1));

	m_pFontID = &GetVarWithDefault("font", uint32(FONT_SMALL))->GetUINT32();
	GetVar("font")->GetSigOnChanged()->connect(1, boost::bind(&TextRenderComponent::OnFontChanged, this, _1));

	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(1, boost::bind(&TextRenderComponent::OnScaleChanged, this, _1));
	
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TextRenderComponent::OnRender, this, _1));
}

void TextRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TextRenderComponent::RenderAsWave(CL_Vec2f vPos, uint32 color)
{
	float offsetY = 0;

	string letter;
	rtRectf rSize;
	
	float dividePowerForSin = 35;
	for (unsigned int i=0; i < m_pText->length(); i++)
	{
		letter = m_pText->at(i);
		GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(&rSize, letter, m_pScale2d->x);
		if (vPos.x+rSize.GetWidth() < 0)
		{
			//don't waste cycles rendering this
			vPos.x += rSize.GetWidth();
			continue;
		}
		offsetY = sin( vPos.x/dividePowerForSin)* (*m_pEffectPower);
		GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x, vPos.y+offsetY, letter, m_pScale2d->x, color);
	
		vPos.x += rSize.GetWidth();
		if (vPos.x > GetScreenSizeX()) return;
	}

}

void TextRenderComponent::OnRender(VariantList *pVList)
{
	if (*m_pAlpha <= 0 || *m_pVisible == 0) return;

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	CL_Vec2f vRotationPt = vFinalPos;

	//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));
	
	if (vFinalPos.y < -m_pSize2d->y) return;
	if (vFinalPos.y > GetOrthoRenderSizeYf()) return;

	
	if (*m_pRotation != 0)
	{
		g_globalBatcher.Flush();
		SetupOrtho();
		PushRotationMatrix(*m_pRotation, vRotationPt);
		vFinalPos -= vRotationPt;
	}

	float alpha;

	if (*m_pDisabled)
	{
		alpha = rt_min(*m_pAlpha, 0.5f);
	} else
	{
		alpha = *m_pAlpha;
	}

	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, alpha);

	switch(*m_pStyle)
	{
	case STYLE_NORMAL:
		
		if (m_pShadowColor != 0)
		{
			GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaledSolidColor(vFinalPos.x+2, vFinalPos.y+2, *m_pText, m_pScale2d->x, ColorCombine(*m_pShadowColor, MAKE_RGBA(255,255,255,255), alpha));
		}
		GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vFinalPos.x, vFinalPos.y, *m_pText, m_pScale2d->x, color);
		break;
	
	case STYLE_EFFECT_SIN_WAVE:
		RenderAsWave(vFinalPos, color);
		break;
	}

	if (*m_pRotation != 0)
	{
		g_globalBatcher.Flush(); //force it to render now so our transformation matrix will work with it
		PopRotationMatrix();
	}
#ifdef _DEBUG

	//DrawRect(vFinalPos, *m_pSize2d); //useful for debugging our touch hotspot

#endif
}