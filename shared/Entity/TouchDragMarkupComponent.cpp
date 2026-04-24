#include "PlatformPrecomp.h"
#include "TouchDragMarkupComponent.h"
#include "EntityUtils.h"


int g_markupPenSize = 10;

void SetGlobalMarkupPenSize(int size)
{
	g_markupPenSize = size;
}

int GetGlobalMarkupPenSize()
{
	return g_markupPenSize;
}

TouchDragMarkupComponent::TouchDragMarkupComponent()
{
	SetName("TouchDragMarkup");
}

TouchDragMarkupComponent::~TouchDragMarkupComponent()
{
	m_pEntityToFollow = NULL;
}

void TouchDragMarkupComponent::ResetTouch()
{
	if (m_bIsDraggingLook)
	{
		//m_bIgnoreNextMove = true;
	}
}

void TouchDragMarkupComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVar("scale2d")->GetVector2();

	Variant v;
	
	m_pEntityToFollow = GetVarWithDefault("entityToFollow", (Entity*) NULL);
	m_pEntityToFollow->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnEntityToFollowChanged, this, _1));

	EntityComponent *pDragComponent = GetParent()->GetComponentByName("TouchDrag");
	
	if (!pDragComponent)
	{
		assert(0 && "TouchDragMarkupComponent requires a TouchDragComponent to be added first");
		return;
	}

	pDragComponent->GetFunction("OnTouchDragUpdate")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnTouchDragUpdate, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("ClearActiveMarkups")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnClearActiveMarkups, this, _1));

	//register to get notified if scalexy changes
	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnScaleChanged, this, _1));
	
	//register to get notified if sizexy changes
	//GetParent()->GetVar("size2d")->GetSigOnChanged()->connect(1, boost::bind(&TouchDragMarkupComponent::OnSizeChanged, this, _1));

	//register to get updated every frame
	//GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchDragComponent::OnInput, this, _1));

	InitMarkupBoard();
	m_surface.InitFromSoftSurface(&m_softSurf);
	m_surface.SetUsesAlpha(true);
	OverlayRenderComponent *m_pOverlayComp = (OverlayRenderComponent *) GetParent()->AddComponent(new OverlayRenderComponent());
	m_pOverlayComp->SetSurface(&m_surface, false);
	m_surface.SetupSignalsForUnloadingAndLoadingTextures();
	//we want to get notified as well
	GetBaseApp()->m_sig_enterforeground.connect(1, boost::bind(&TouchDragMarkupComponent::OnEnterForeground, this, _1));
	GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&TouchDragMarkupComponent::OnLoadSurfaces, this));
	//register to get updated every frame
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnUpdate, this, _1));
}

void TouchDragMarkupComponent::OnEnterForeground(VariantList* pVList)
{
	/*
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
	*/
}
void TouchDragMarkupComponent::OnLoadSurfaces()
{
	
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
	
}


void TouchDragMarkupComponent::OnEntityToFollowChanged(Variant* pDataObject)
{
	
	if (m_pEntityToFollow->GetEntity() != NULL)
	{
		LogMsg("Entity to follow changed to %s", m_pEntityToFollow->GetEntity()->GetName().c_str());
		m_pEntityToFollow->GetEntity()->GetVar("MarkupComponentFollowingUs")->Set(this);//in case they care
		//if this entity is destroyed, destroy us too
		m_pEntityToFollow->GetEntity()->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&TouchDragMarkupComponent::OnEntityWeFollowGotDeleted, this, _1));
	} else
	{
		//We should remove signals to/from it
	} 
}

void TouchDragMarkupComponent::OnEntityWeFollowGotDeleted(VariantList* pVList)
{
	LogMsg("Entity we follow got deleted, deleting us too");
	m_pEntityToFollow->Set((Entity*)NULL);
	GetParent()->SetTaggedForDeletion();
}

void TouchDragMarkupComponent::OnUpdate(VariantList* pVList)
{
	if (!m_pEntityToFollow || GetParent()->GetTaggedForDeletion()) return;
	
	Entity* pEntToFollow = m_pEntityToFollow->GetEntity();

	if (!pEntToFollow) return;

	SetPos2DEntity(GetParent(), pEntToFollow->GetVar("pos2d")->GetVector2());
	
	//GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	GetParent()->GetParent()->MoveEntityToTopOfOtherEntityByAddress(GetParent(), pEntToFollow);
	EntitySetScaleBySize(GetParent(), pEntToFollow->GetVar("size2d")->GetVector2());
	//LogMsg("Update: Setting size to %s", PrintVector2(m_pEntityToFollow->GetEntity()->GetVar("size2d")->GetVector2()).c_str() );
	//LogMsg("Entity's scale is %s, my scale is %s", PrintVector2(pEntToFollow->GetVar("scale2d")->GetVector2()).c_str(), PrintVector2(GetParent()->GetVar("scale2d")->GetVector2()).c_str());
}

void TouchDragMarkupComponent::InitMarkupBoard()
{
	m_softSurf.Init(2048, 2048, SoftSurface::SURFACE_RGBA);
	m_softSurf.FillColor(glColorBytes(255, 255, 255, 0));
}

void TouchDragMarkupComponent::OnScaleChanged(Variant* pDataObject)
{
	//LogMsg("Markup surface changed to %s", PrintVector2(*m_pSize2d).c_str());
	ResetTouch();
}

void TouchDragMarkupComponent::OnSizeChanged(Variant* pDataObject)
{
	//LogMsg("Size:  %s  Scale: %s", PrintVector2(*m_pSize2d).c_str(), PrintVector2(*m_pScale2d).c_str());
}

void TouchDragMarkupComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TouchDragMarkupComponent::UpdateStatusMessage(string msg)
{
	int timeMS = 1000;

	Entity *pOldEnt = GetParent()->GetEntityByName("DebugText");
	
	if (pOldEnt)
	{
		pOldEnt->SetName("");
		pOldEnt->SetTaggedForDeletion();
	}

	Entity* pEnt = CreateTextLabelEntity(GetParent(), "DebugText", 0, 0, msg);
	SetupTextEntity(pEnt, FONT_LARGE, 0.66f);
	FadeOutAndKillEntity(pEnt, true, 100, timeMS);
}

void TouchDragMarkupComponent::OnTouchDragUpdate(VariantList* pVList)
{
	int fingerID = pVList->Get(2).GetUINT32(); //0 is left mouse button
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag

	CL_Vec2f vMovement = pVList->Get(1).GetVector2();
	//CL_Vec2f vCurPos = pVList->Get(3).GetVector2();
	CL_Vec2f vCurPos = GetBaseApp()->GetTouch(fingerID)->GetPos();

#ifdef _DEBUG
	//LogMsg("pos: %s, offset %s", PrintVector2(vCurPos).c_str(), PrintVector2(vMovement).c_str());
#endif
 	if (m_bIsDraggingLook)
	{
		//*m_pPos2d += vMovement;
	
		if (!m_bIgnoreNextMove)
		{
		
			CL_Vec2f vPos = vCurPos+ vMovement;
			eAlignment align = eAlignment(GetParent()->GetVar("alignment")->GetUINT32());
			vPos -= *m_pPos2d - GetAlignmentOffset(*m_pSize2d, align);

			Draw(vPos);

			//UpdateStatusMessage("Drawing at X: " + toString(vPos.x) + " Y: " + toString(vPos.y));
			//m_startPos = m_startPos + vMovement;
		}

		m_bIgnoreNextMove = false;
	}
}

void TouchDragMarkupComponent::OnOverStart(VariantList* pVList)
{

	int fingerID =  pVList->Get(2).GetUINT32(); //0 is left mouse button
	
	//LogMsg("Detected fingerID %d", fingerID);
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag
	m_lastPosDrawn = CL_Vec2f(-1, -1);

	//Reset smoothing state so this stroke starts fresh.
	m_bHavePrev = false;
	m_bHaveCurr = false;

	m_bIsDraggingLook = true;

	//move it to the top layer
	//GetParent()->GetParent()->MoveEntityToTopByAddress(GetParent());
	
	CL_Vec2f vPos = pVList->Get(0).GetVector2();

	vPos -= *m_pPos2d;
	m_startPos = vPos;
	Draw(vPos);

}
void TouchDragMarkupComponent::Draw(CL_Vec2f vPos)
{
	// Convert to local coordinates of our surface
	vPos.x = vPos.x / m_pScale2d->x;
	vPos.y = vPos.y / m_pScale2d->y;

	//FPS-stutter / pen-skip protection: only break the stroke when BOTH wall-clock
	//time AND distance suggest a real teleport (mouse flicked far during a freeze).
	//A long time gap with the cursor still near where it was is just a slow frame --
	//bridging it is correct and expected.  Otherwise heavy drawing work itself can
	//cause >100ms frames and the stutter check would erroneously break the stroke.
	const unsigned int now = GetBaseApp()->GetTick();
	const unsigned int kMaxStutterMS = 250;                       //~15 missed frames at 60Hz
	const float        kMaxStutterDistanceSq = 300.0f * 300.0f;   //surface-local pixels squared
	if (m_bHaveCurr)
	{
		const unsigned int dt = now - m_lastSampleTimeMS;
		const float dx = vPos.x - m_inputCurr.x;
		const float dy = vPos.y - m_inputCurr.y;
		if (dt > kMaxStutterMS && (dx * dx + dy * dy) > kMaxStutterDistanceSq)
		{
			m_bHavePrev = false;
			m_bHaveCurr = false;
		}
	}
	m_lastSampleTimeMS = now;

	const float radius = (float)GetGlobalMarkupPenSize();
	const glColorBytes col(255, 0, 0, 255);

	if (!m_bHaveCurr)
	{
		//First sample of the stroke: stamp a dot so a single click/tap still leaves a mark.
		m_softSurf.DrawCircleAASafe(vPos.x, vPos.y, col, radius);
		m_inputCurr = vPos;
		m_bHaveCurr = true;
	}
	else
	{
		//Exponential-moving-average low-pass filter on the input point.  The Bezier
		//pipeline already gives C1 continuity between segments, but each segment still
		//passes through the raw points -- so sub-pixel hand jitter and OS pointer
		//quantization show up as tiny visible kinks.  Blending each new raw point with
		//the last smoothed point removes that high-frequency noise.  Alpha is the
		//weight of the new raw input: 1.0 = no smoothing, lower = more smoothing/lag.
		const float kInputSmoothingAlpha = 0.5f;
		vPos = m_inputCurr * (1.0f - kInputSmoothingAlpha) + vPos * kInputSmoothingAlpha;

		if (!m_bHavePrev)
		{
			//Second sample: shift and wait for a third so we have enough points to form a curve.
			//One frame of latency at the very start of a stroke is not visually noticeable.
			m_inputPrev = m_inputCurr;
			m_inputCurr = vPos;
			m_bHavePrev = true;
		}
		else
		{
			//Third+ sample: emit a smooth quadratic segment from midpoint(prev,curr) to
			//midpoint(curr,vPos) using curr as the control point.  Successive segments
			//are C1-continuous because they share the midpoint, so even fast mouse motion
			//draws a smooth curve instead of a polygon.
			StampStroke(m_inputPrev, m_inputCurr, vPos);
			m_inputPrev = m_inputCurr;
			m_inputCurr = vPos;
		}
	}

	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();
}

void TouchDragMarkupComponent::StampStroke(CL_Vec2f a, CL_Vec2f b, CL_Vec2f c)
{
	//Quadratic Bezier from midpoint(a,b) to midpoint(b,c) with b as the control point.
	CL_Vec2f mAB = (a + b) * 0.5f;
	CL_Vec2f mBC = (b + c) * 0.5f;

	float lenAB_x = mAB.x - b.x;
	float lenAB_y = mAB.y - b.y;
	float lenBC_x = b.x - mBC.x;
	float lenBC_y = b.y - mBC.y;
	float curveLength = sqrtf(lenAB_x * lenAB_x + lenAB_y * lenAB_y)
	                  + sqrtf(lenBC_x * lenBC_x + lenBC_y * lenBC_y);

	const float radius = (float)GetGlobalMarkupPenSize();
	float spacing = radius * 0.25f;
	if (spacing < 1.0f) spacing = 1.0f;

	int steps = 1;
	if (curveLength > 0.0f) steps = (int)ceilf(curveLength / spacing);
	if (steps < 1) steps = 1;

	const glColorBytes col(255, 0, 0, 255);

	for (int i = 0; i <= steps; i++)
	{
		float t = (float)i / (float)steps;
		float u = 1.0f - t;
		CL_Vec2f pt = mAB * (u * u) + b * (2.0f * u * t) + mBC * (t * t);
		m_softSurf.DrawCircleAASafe(pt.x, pt.y, col, radius);
	}
}

void TouchDragMarkupComponent::OnOverEnd(VariantList* pVList)
{
	int fingerID = pVList->Get(2).GetUINT32(); //0 is left mouse button
	if (fingerID != m_fingerIDToTrack) return; //only allow left mouse button to drag

	//Flush trailing half-segment so the stroke ends exactly at the last input point
	//instead of stopping at midpoint(prev, curr).  StampStroke(prev, curr, curr)
	//collapses to a straight line from midpoint(prev, curr) to curr.
	if (m_bHavePrev && m_bHaveCurr)
	{
		StampStroke(m_inputPrev, m_inputCurr, m_inputCurr);
		m_softSurf.FlipY();
		m_softSurf.UpdateGLTexture(&m_surface);
		m_softSurf.FlipY();
	}

	m_bHavePrev = false;
	m_bHaveCurr = false;

	m_bIsDraggingLook = false;
}

void TouchDragMarkupComponent::OnClearActiveMarkups(VariantList* pVList)
{
	LogMsg("%s clearing markups", GetParent()->GetName().c_str());

	InitMarkupBoard();
	m_softSurf.FlipY();
	m_softSurf.UpdateGLTexture(&m_surface);
	m_softSurf.FlipY();

	//Reset smoothing state so any in-progress stroke starts fresh.
	m_bHavePrev = false;
	m_bHaveCurr = false;
}

