#include "PlatformPrecomp.h"
#include "LogDisplayComponent.h"
#include "EntityUtils.h"

//needed for the scroll bar
#include "ScrollBarRenderComponent.h"
#include "TouchDragComponent.h"
#include "RenderScissorComponent.h"

#define C_SCROLL_SPEED_NEEDED_TO_PAUSE_UPDATES 1.0f
LogDisplayComponent::LogDisplayComponent()
{
	SetName("LogDisplay");
	m_pActiveConsole = NULL;
	m_pInternalConsole = NULL;
	m_pScrollBarComp = NULL;
	m_bIsDraggingLook= false;
	m_curLine = 0;
	m_bUsingCustomConsole = false;

}

LogDisplayComponent::~LogDisplayComponent()
{
		SAFE_DELETE(m_pInternalConsole);
}

void LogDisplayComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pFontID = &GetVarWithDefault("font", uint32(FONT_SMALL))->GetUINT32();
	
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();

	//our own stuff
	m_pFontScale = &GetVarWithDefault("fontScale", Variant(1.0f))->GetFloat();
	
	//for the momentum stuff
	m_pFriction = &GetVarWithDefault("friction", 0.1f)->GetFloat();
	m_pMaxScrollSpeed = &GetVarWithDefault("maxScrollSpeed", float(7))->GetFloat();
	m_pPowerMod = &GetVarWithDefault("powerMod", float(0.15))->GetFloat();

	m_pText = &GetVar("text")->GetString(); //local to us
	GetVar("text")->GetSigOnChanged()->connect(1, boost::bind(&LogDisplayComponent::OnTextChanged, this, _1));

	m_pEnableScrolling = &GetVar("enableScrolling")->GetUINT32();
	GetVar("enableScrolling")->GetSigOnChanged()->connect(boost::bind(&LogDisplayComponent::OnEnableScrollingChanged, this, _1));

	if (!m_pActiveConsole) 
	{
		m_pActiveConsole = GetBaseApp()->GetConsole();
		m_pActiveConsole->m_sig_on_text_added.connect(boost::bind(&LogDisplayComponent::OnTextAdded, this));

	}
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&LogDisplayComponent::OnRender, this, _1));
	GetFunction("AddLine")->sig_function.connect(1, boost::bind(&LogDisplayComponent::AddLine, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&LogDisplayComponent::OnUpdate, this, _1));

	if (*m_pEnableScrolling)
	{
		//turn this on now
		OnEnableScrollingChanged(GetVar("enableScrolling"));
	}

	OnTextAdded();
}

void LogDisplayComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void LogDisplayComponent::InitInternalConsoleIfNeeded()
{
	if (!m_bUsingCustomConsole && !m_pInternalConsole)
	{
		m_pInternalConsole = new Console;
		m_pActiveConsole = m_pInternalConsole;
	}
}

void LogDisplayComponent::OnTextChanged(Variant *pDataObject)
{
	InitInternalConsoleIfNeeded();
	m_pActiveConsole->Clear(); //erase everything
	VariantList vList(*pDataObject); //yes, we're doing an ugly extra copy here
	AddLine(&vList);
}

void LogDisplayComponent::AddLine(VariantList *pVList)
{
	InitInternalConsoleIfNeeded();
	//word wrap it into lines if needed
	CL_Vec2f enclosedSize2d;
	GetBaseApp()->GetFont(eFont(*m_pFontID))->MeasureTextAndAddByLinesIntoDeque(*m_pSize2d, pVList->Get(0).GetString(), &m_queuedLines, *m_pFontScale, enclosedSize2d);
}

void LogDisplayComponent::OnTextAdded()
{
	if (!m_bIsDraggingLook)
	{
		m_curLine = (float)m_pActiveConsole->GetTotalLines(); //move to last line.  Cur means the last line that we can see.
	}
	UpdateScrollBar();
}

void LogDisplayComponent::OnEnableScrollingChanged(Variant *pVariant)
{
	if (pVariant->GetUINT32() != 1)
	{
		assert(!"Actually we don't support turning this off if it was already on.  Hope you know that!");
		return;
	}

	if (m_pScrollBarComp)
	{
		//already initted
		return;
	}

	//to get the OnOverStart and OnOverEnd messages
	GetParent()->AddComponent(new TouchHandlerComponent);
	//to get drag messages
	EntityComponent *pDragComp = GetParent()->AddComponent(new TouchDragComponent);
	GetParent()->AddComponent(new RenderScissorComponent);
	//add touch drag handler, so we can respond when the user drags the screen
	pDragComp->GetFunction("OnTouchDragUpdate")->sig_function.connect(1, boost::bind(&LogDisplayComponent::OnTouchDragUpdate, this, _1));	
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&LogDisplayComponent::OnOverStart, this, _1));	
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&LogDisplayComponent::OnOverEnd, this, _1));	

	//to see the scroll bar, we'll create a child entity and throw a scroll bar render in it
	Entity *pScrollEnt = GetParent()->AddEntity(new Entity("Scrollbar"));
	m_pScrollBarComp = pScrollEnt->AddComponent(new ScrollBarRenderComponent); 	//add a visual way to see the scroller position

	//set the size to match ours
	pScrollEnt->GetVar("size2d")->Set(*m_pSize2d);

	//let its size update when ours does
	GetParent()->GetVar("size2d")->GetSigOnChanged()->connect(boost::bind(&Variant::SetVariant, pScrollEnt->GetVar("size2d"), _1));
	m_curLine = (float)m_pActiveConsole->GetTotalLines(); //move to last line.  Cur means the last line that we can see.
	UpdateScrollBar();
}

void LogDisplayComponent::OnTouchDragUpdate(VariantList *pVList)
{
	CL_Vec2f vMovement = pVList->Get(1).GetVector2();

#ifdef _DEBUG
	//LogMsg("offset %s", PrintVector2(vMovement).c_str());
#endif

	//for exact movement we'd do this:
	//ModCurLine(-(vMovement.y*0.07f));

	//for momentum mode:
	m_vecDisplacement += vMovement* *m_pPowerMod;
}

void LogDisplayComponent::OnUpdate(VariantList *pVList)
{
	if (m_pScrollBarComp)
	{
		//handle update for momentum movement of the scroller
		ModByDistance(-(m_vecDisplacement.y*GetBaseApp()->GetDelta()));
		m_vecDisplacement *= (1- (*m_pFriction*GetBaseApp()->GetDelta()));
	}

	if (!m_bIsDraggingLook &&
		(!m_pScrollBarComp || m_vecDisplacement.length() < C_SCROLL_SPEED_NEEDED_TO_PAUSE_UPDATES))
	{
		for (;m_queuedLines.size();)
		{
			m_pActiveConsole->AddLine(m_queuedLines.front());
			m_queuedLines.pop_front();
		}
	}
}

void LogDisplayComponent::ModByDistance(float mod)
{
	RTFont *pFont = GetBaseApp()->GetFont(eFont(*m_pFontID));
	float fontHeight = pFont->GetLineHeight(*m_pFontScale);
	ModCurLine(mod/fontHeight);
}

void LogDisplayComponent::ModCurLine(float mod)
{
	m_curLine += mod;
	//make sure we're within valid ranges
	
	CL_Rectf vTotalBounds = CL_Rectf(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));

	RTFont *pFont = GetBaseApp()->GetFont(eFont(*m_pFontID));
	float fontHeight = pFont->GetLineHeight(*m_pFontScale);
	float linePerScreen = vTotalBounds.get_height()/fontHeight;

	if (m_curLine < linePerScreen) m_curLine = linePerScreen;
	if (m_curLine > m_pActiveConsole->GetTotalLines())
	{
		m_curLine = (float)m_pActiveConsole->GetTotalLines();
	}
	UpdateScrollBar();
}

void LogDisplayComponent::OnOverStart(VariantList *pVList)
{
	m_bIsDraggingLook = true;
}

void LogDisplayComponent::OnOverEnd(VariantList *pVList)
{
	m_bIsDraggingLook = false;
}

void LogDisplayComponent::UpdateScrollBar()
{
	if (!m_pScrollBarComp) return;

	CL_Rectf vTotalBounds = CL_Rectf(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));

	eFont fontID = eFont(*m_pFontID);
	RTFont *pFont = GetBaseApp()->GetFont(fontID);
	float fontHeight = pFont->GetLineHeight(*m_pFontScale);
	float linePerScreen =  (vTotalBounds.get_height()/fontHeight);
	vTotalBounds.bottom = vTotalBounds.top;
	vTotalBounds.top = vTotalBounds.bottom - fontHeight*m_pActiveConsole->GetTotalLines();

	vTotalBounds.right =0; //not allowing horizontal scrolling at all
	float percent = (m_curLine-linePerScreen) / ((float) (m_pActiveConsole->GetTotalLines()-linePerScreen));

//	LogMsg("Percent is %.2f", percent);
	m_pScrollBarComp->GetParent()->GetVar("boundsRect")->Set(vTotalBounds);
	m_pScrollBarComp->GetParent()->GetVar("progress2d")->Set(CL_Vec2f(0, percent));
}

void LogDisplayComponent::OnRender(VariantList *pVList)
{
	CHECK_GL_ERROR();	
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;

	if (*m_pAlpha == 0) return;

	uint32 color = ColorCombine(*m_pColor, *m_pColorMod, *m_pAlpha);

	eFont fontID = eFont(*m_pFontID);
	RTFont *pFont = GetBaseApp()->GetFont(fontID);
	float fontHeight = pFont->GetLineHeight(*m_pFontScale);
	if (m_curLine > m_pActiveConsole->GetTotalLines())
	{
		//our log got smaller?  Well, ok, deal with it
		m_curLine = 0;
	}
	
	float remainder = m_curLine - (int)m_curLine;
	float y = (vFinalPos.y + m_pSize2d->y);

	y-=remainder* fontHeight; //start a line lower to cover half lines during scrolling, hopefully we have a glscissor component
	
	//if (remainder != 0)
	{
		vFinalPos.y -= fontHeight;
	}
	assert(m_pSize2d->y > 0);
	y -= fontHeight;

	int curLine = (int)m_curLine-1;
	RenderBatcher b;

	//first count how many lines we'll be able to display

	FontStateStack fontState;

	int linesToDraw = 0;

	while (y >= vFinalPos.y && curLine >= 0)
	{
		//pFont->DrawScaled(vFinalPos.x, y, m_pActiveConsole->GetLine(curLine), *m_pFontScale,
		//	color, &fontState, &b);
		curLine--;
		y -= fontHeight;
		linesToDraw++;
	}

	if (remainder != 0)
	{
		linesToDraw++;
	}
	//that was fun practice, now let's draw the real thing
	CHECK_GL_ERROR();	
	for (int i=0; i < linesToDraw; i++)
	{
		curLine++;
		y += fontHeight;
		pFont->DrawScaled(vFinalPos.x, y, m_pActiveConsole->GetLine(curLine), *m_pFontScale,
			color, &fontState, &b);
	}

	b.Flush();
}

void LogDisplayComponent::SetConsole( Console *pConsole )
{
	if (m_pActiveConsole)
	{
		//remove any old signals we had
		m_pActiveConsole->m_sig_on_text_added.disconnect(boost::bind(&LogDisplayComponent::OnTextAdded, this));
	}
	SAFE_DELETE(m_pInternalConsole);
	m_pActiveConsole = pConsole;
	if (pConsole)
	{
		m_bUsingCustomConsole = true;
	} else
	{
		m_bUsingCustomConsole = false;
	}
	m_pActiveConsole->m_sig_on_text_added.connect(boost::bind(&LogDisplayComponent::OnTextAdded, this));

}

void SetConsole(bool bOn, bool bEnableScrollbars)
{
	Entity *pConsole = GetEntityRoot()->GetEntityByName("ConsoleEnt");

	if (bOn && pConsole) return; //already on
	if (!bOn && !pConsole) return; //already off

	if (pConsole)
	{
		//kill it
		KillEntity(pConsole);
	} else
	{
		pConsole = GetEntityRoot()->AddEntity(new Entity("ConsoleEnt"));
		pConsole->GetVar("pos2d")->Set(CL_Vec2f(GetScreenSizeXf()/2, 80));
		pConsole->GetVar("size2d")->Set(CL_Vec2f(GetScreenSizeXf()/2, GetScreenSizeYf()/2));

		EntityComponent *pComp = new LogDisplayComponent;
		if (bEnableScrollbars)
		{
			pComp->GetVar("enableScrolling")->Set(uint32(1));
		}
		
		pConsole->AddComponent(pComp);
		AddFocusIfNeeded(pConsole);
	}
}

void ToggleConsole()
{
	Entity *pConsole = GetEntityRoot()->GetEntityByName("ConsoleEnt");

	if (pConsole)
	{
		//kill it
		SetConsole(false);
	} else
	{
		SetConsole(true);
	}
}

Entity * CreateLogDisplayEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale)
{
	Entity *pText = pParent->AddEntity(new Entity(entName));

	EntityComponent *pLogComp = pText->AddComponent(new LogDisplayComponent);
	SetupTextEntity(pText, FONT_SMALL, scale);
	pText->GetVar("size2d")->Set(vTextAreaSize);
	pLogComp->GetVar("text")->Set(msg);
	pText->GetVar("pos2d")->Set(vPos);
	//pComp->SetConsole(GetApp()->GetGlobalLog());
	pLogComp->GetVar("enableScrolling")->Set(uint32(1));

	return pText;
}