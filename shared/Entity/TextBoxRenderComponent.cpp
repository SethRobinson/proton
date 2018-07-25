#include "PlatformPrecomp.h"

#include "TextBoxRenderComponent.h"
#include "BaseApp.h"

TextBoxRenderComponent::TextBoxRenderComponent()
{
	SetName("TextBoxRender");
}

TextBoxRenderComponent::~TextBoxRenderComponent()
{
}

void TextBoxRenderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVarWithDefault("size2d", CL_Vec2f(300,200))->GetVector2();
	m_pEnclosedSize2d = &GetParent()->GetVarWithDefault("enclosedSize2d", CL_Vec2f(0,0))->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pTimingSystem = &GetParent()->GetVarWithDefault("timingSystem", Variant(uint32(GetBaseApp()->GetActiveTimingSystem())))->GetUINT32();

	//our own stuff
	m_pFontScale = &GetVarWithDefault("fontScale", Variant(1.0f))->GetFloat();
	m_pEffectPower = &GetVarWithDefault("effectPower", Variant(30.0f))->GetFloat();
	m_pStyle = &GetVarWithDefault("style", Variant(uint32(STYLE_NORMAL)))->GetUINT32();
	
	//this controls the alignment of the wrapped text within the text rect, not the text rect itself!
	m_pTextAlignment = &GetVarWithDefault("textAlignment", (uint32)ALIGNMENT_UPPER_LEFT)->GetUINT32();
	GetVar("textAlignment")->GetSigOnChanged()->connect(1, boost::bind(&TextBoxRenderComponent::OnTextAlignmentChanged, this, _1));

	m_pText = &GetVar("text")->GetString(); //local to us
	GetVar("text")->GetSigOnChanged()->connect(1, boost::bind(&TextBoxRenderComponent::OnTextChanged, this, _1));

	m_pFontID = &GetVarWithDefault("font", uint32(FONT_SMALL))->GetUINT32();
	GetVar("font")->GetSigOnChanged()->connect(1, boost::bind(&TextBoxRenderComponent::OnFontChanged, this, _1));

	pEnt->GetVar("scale2d")->GetSigOnChanged()->connect(1, boost::bind(&TextBoxRenderComponent::OnScaleChanged, this, _1));

	//register ourselves to render if the parent does
	pEnt->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TextBoxRenderComponent::OnRender, this, _1));

	m_pShadowColor = &GetVarWithDefault("shadowColor", Variant(MAKE_RGBA(0,0,0,0)))->GetUINT32();

}

void TextBoxRenderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void TextBoxRenderComponent::OnTextChanged(Variant *pDataObject)
{
	//we need to breakdown the text and convert it into word wrapped lines in our deque
	m_lines.clear();
	GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureTextAndAddByLinesIntoDeque(*m_pSize2d, *m_pText, &m_lines, *m_pFontScale, *m_pEnclosedSize2d);
	m_typeTimer = GetTick(eTimingSystem(*m_pTimingSystem));
	m_lastLineRendered = 0;
	m_lastCharRendered = 0;
	GetVar("totalHeightInPixels")->Set(m_lines.size()*GetBaseApp()->GetFont(eFont(*m_pFontID))->GetLineHeight(*m_pFontScale));
	GetVar("totalLines")->Set(uint32(m_lines.size()));
	m_pSize2d->y = GetVar("totalHeightInPixels")->GetFloat();

	if (GetParent())
	{
		//in case anybody cares, send a signal that our size changed
		VariantList v(GetParent());
		GetParent()->GetFunction("OnSizeChanged")->sig_function(&v);
	}
	
}


void TextBoxRenderComponent::OnScaleChanged(Variant *pDataObject)
{
	//OPTIMIZE: this is lame, recomputing everything, but whatever
	OnTextChanged(NULL);
}

void TextBoxRenderComponent::OnTextAlignmentChanged(Variant *pDataObject)
{
	//not really needed yet, alignment isn't used during the computation process
	//OnTextChanged(NULL);
}


void TextBoxRenderComponent::OnFontChanged(Variant *pDataObject)
{
	//OPTIMIZE: this is lame, recomputing everything, but whatever
	OnTextChanged(NULL);
}


void TextBoxRenderComponent::DrawTextNormal(CL_Vec2f vPos)
{
	//go through all text and draw it
	if (vPos.x > GetScreenSizeX()) return; //no use drawing stuff that is off the screen to the right

	
	float lineHeight = GetBaseApp()->GetFont(eFont(*m_pFontID))->GetLineHeight(*m_pFontScale);
	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

	FontStateStack state;

	for (unsigned int i=0; i < m_lines.size(); i++)
	{
		
		if (vPos.y+lineHeight < 0) 
		{
			//it's above the screen and doesn't need to be drawn.. but we still want to process it for color information.
			GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaledFakeToUpdateState(m_lines[i],  color, &state);
	
			vPos.y += lineHeight;		
			continue; //no use drawing stuff that is above the screen
		}
		

		if (vPos.y > GetOrthoRenderSizeYf()) 
		{
			break; //no use drawing any more down here
		}

		float lineSizeX;

		switch (*m_pTextAlignment)
		{
		case ALIGNMENT_UPPER_CENTER:

			//center the text on its line.  No reason why we couldn't cache this size data, but I suspect it's not a big hit compared to the
			//actual rendering.
			lineSizeX = GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(m_lines[i], *m_pFontScale).x;
		
			
			if (*m_pShadowColor != 0)
			{
				GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaledSolidColor( (vPos.x + ( (m_pSize2d->x-lineSizeX)/2))+2, vPos.y+2, m_lines[i], *m_pFontScale,ColorCombine(*m_pShadowColor, MAKE_RGBA(255,255,255,255), (float)GET_ALPHA(color)/255), NULL, &g_globalBatcher);
			}

			
			GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x + ( (m_pSize2d->x-lineSizeX)/2), vPos.y, m_lines[i], *m_pFontScale, color, &state, &g_globalBatcher);
			break;

		default:

			if (*m_pShadowColor != 0)
			{
				GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaledSolidColor(vPos.x+2, vPos.y+2, m_lines[i], *m_pFontScale, ColorCombine(*m_pShadowColor, MAKE_RGBA(255,255,255,255), (float)GET_ALPHA(color)/255), NULL, &g_globalBatcher);
		
			}
			GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x, vPos.y, m_lines[i], *m_pFontScale, color, &state, &g_globalBatcher);

			break;
		}
	
		//advance to the next line
		vPos.y += lineHeight;
	}
	
}

void TextBoxRenderComponent::DrawTextType(CL_Vec2f vPos)
{
	//go through all text and draw it
	
	if (vPos.x > GetScreenSizeX()) return; //no use drawing stuff that is off the screen to the right
	unsigned int charsToRender = (unsigned int)((float(GetTick(eTimingSystem(*m_pTimingSystem))-m_typeTimer))/ *m_pEffectPower);
	
	float lineHeight = GetBaseApp()->GetFont(eFont(*m_pFontID))->GetLineHeight(*m_pFontScale);
	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);
	FontStateStack state;

	for (unsigned int i=0; i < m_lines.size(); i++)
	{
		if (vPos.y > GetOrthoRenderSizeYf()) return; //no use drawing any more down here
		
		if (m_lines[i].length() < charsToRender)
		{
			charsToRender -= m_lines[i].length();
		} else
		{
			if (m_lastLineRendered < i)
			{
				m_lastLineRendered = i;
				m_lastCharRendered = 0;
			}

			//scan this text for control codes that we need to fix the timing problem they create
			for (unsigned int c=m_lastCharRendered; c < charsToRender; c++)
			{
				if (m_lines[i][c] == '`')
				{
					if (c > 0 && m_lines[i][c-1] == '`')
					{
						//ignore this, it's actually after a real control code
					} else
					{
						m_typeTimer -= uint32((*m_pEffectPower)*2); //weird way of fast forwarding our timing by moving back the original timer start
					}
				}
			}
			m_lastCharRendered = charsToRender;
			
			float lineSizeX;

			switch (*m_pTextAlignment)
			{
			case ALIGNMENT_UPPER_CENTER:
				{
					//center the text on its line.  No reason why we couldn't cache this size data, but I suspect it's not a big hit compared to the
					//actual rendering.
					string textToShow = m_lines[i].substr(0, charsToRender);
					lineSizeX = GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureText(textToShow, *m_pFontScale).x;
					GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x + ( (m_pSize2d->x-lineSizeX)/2), vPos.y, textToShow, *m_pFontScale, color,  &state,  &g_globalBatcher);
				}
				break;

			default:
				GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x, vPos.y, m_lines[i].substr(0, charsToRender), *m_pFontScale, color,  &state,  &g_globalBatcher);

				break;
			}

				break; //all done
		}
		
		if (vPos.y+lineHeight < 0) 
		{
			vPos.y += lineHeight;		
			
			//well, actually there is a use...
			continue; //no use drawing stuff that is above the screen
		}


		if (vPos.y > GetOrthoRenderSizeYf()) 
		{
			break; //no use drawing any more down here
		}

		GetBaseApp()->GetFont(eFont(*m_pFontID))->DrawScaled(vPos.x, vPos.y, m_lines[i], *m_pFontScale, color,  NULL, &g_globalBatcher);

		//advance to the next line
		vPos.y += lineHeight;
	}

	//g_globalBatcher.Flush();
}

void TextBoxRenderComponent::OnRender(VariantList *pVList)
{

	if (*m_pAlpha <= 0) return;

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	//vFinalPos -= GetAlignmentOffset(*m_pEnclosedSize2d, eAlignment(*m_pAlignment));

	switch(*m_pStyle)
	{
	case STYLE_NORMAL:
		DrawTextNormal(vFinalPos);
		break;
	case STYLE_TYPE:
		DrawTextType(vFinalPos);
		break;
	}

#ifdef _DEBUG
	
	//useful for debugging where the text area rect really is
	//DrawRect(vFinalPos, *m_pSize2d, MAKE_RGBA(200,200,0,200**m_pAlpha));
#endif
}