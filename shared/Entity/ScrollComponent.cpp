#include "PlatformPrecomp.h"
#include "ScrollComponent.h"
#include "BaseApp.h"

ScrollComponent::ScrollComponent()
{
	SetName("Scroll");
	m_activeFinger = -1;
	m_bIsScrolling = false;
	m_vTotalDisplacementOnCurrentSwipe = CL_Vec2f(0,0);
	m_lastTouchPos = CL_Vec2f(-100,-100);
	m_vecDisplacement = CL_Vec2f(0,0);
}

ScrollComponent::~ScrollComponent()
{
	if (m_activeFinger != -1)
	{
		//mark the touch we were using as unhandled now, so if we're recreated right at the same place controls don't
		//go dead until they release and touch again
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(m_activeFinger);
		if (pTouch)
		{
			pTouch->SetWasHandled(false);
		}
	}
}

void ScrollComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//shared with the rest of the entity
	m_vecDisplacement = m_vecChildPos = CL_Vec2f(0,0);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	//vars in our component namespace
	m_pBoundsRect = &GetVarWithDefault("boundsRect", CL_Rectf(0, 0, 0,0))->GetRect();
	m_pScrollStyle = &GetVarWithDefault("scrollStyle", uint32(STYLE_MOMENTUM))->GetUINT32();
	
	//only used for "momentum style"
	m_pFriction = &GetVarWithDefault("friction", 0.1f)->GetFloat();
	m_pMaxScrollSpeed = &GetVarWithDefault("maxScrollSpeed", float(7))->GetFloat();
	m_pPowerMod = &GetVarWithDefault("powerMod", float(0.15))->GetFloat();
	m_progressVar = GetVar("progress2d");
	m_pEnforceFingerTracking = &GetVarWithDefault("fingerTracking", uint32(0))->GetUINT32();
	m_pSwipeDetectDistance = &GetVarWithDefault("swipeDetectDistance", 25.0f)->GetFloat();
	m_pDontScrollUntilSwipeDetected = &GetVarWithDefault("dontScrollUntilSwipeDetected", uint32(0))->GetUINT32();
	m_pEatAllInput = &GetVarWithDefault("eatAllInput", uint32(0))->GetUINT32();

	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&ScrollComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&ScrollComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&ScrollComponent::OnOverMove, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&ScrollComponent::OnUpdate, this, _1));
	GetFunction("SetProgress")->sig_function.connect(1, boost::bind(&ScrollComponent::SetProgress, this, _1));
}


void ScrollComponent::SetProgress(VariantList *pVList)
{
	CL_Vec2f vProg = pVList->m_variant[0].GetVector2();
	//LogMsg("Setting progress to %s", PrintVector2(vProg).c_str());
	m_vecChildPos.x = m_pBoundsRect->right - vProg.x * m_pBoundsRect->get_width();
	m_vecChildPos.y = m_pBoundsRect->bottom - vProg.y * m_pBoundsRect->get_height();

	//update it
	SetPosition(CL_Vec2f(0,0), true);
}


void ScrollComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

bool ScrollComponent::isInterestingFinger(uint32 fingerID) const
{
	if (*m_pEnforceFingerTracking && m_activeFinger != -1 && fingerID != m_activeFinger)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ScrollComponent::SetIsScrolling(bool bScrolling)
{
	if (bScrolling == m_bIsScrolling) return; //no change

	m_bIsScrolling = bScrolling;

	if (!m_bIsScrolling)
	{
		m_vTotalDisplacementOnCurrentSwipe = CL_Vec2f(0,0);
	}
}

void ScrollComponent::OnOverStart(VariantList *pVList)
{
	uint32 fingerID = pVList->Get(2).GetUINT32();
	if (!isInterestingFinger(fingerID)) return;

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

	if (*m_pEnforceFingerTracking && m_activeFinger == -1 && *m_pDontScrollUntilSwipeDetected == 0 && *m_pEatAllInput != 0)
	{
		if (pTouch->WasHandled()) return;
		pTouch->SetWasHandled(true, GetParent());
		m_activeFinger = fingerID;
	}

	pTouch->SetWasPreHandled(true, GetParent());
	
	
	SetIsScrolling(false); 
	m_lastTouchPos = pVList->m_variant[0].GetVector2();
}

void ScrollComponent::OnOverEnd(VariantList *pVList)
{
	uint32 fingerID = pVList->Get(2).GetUINT32();
	if (!isInterestingFinger(fingerID)) return;

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
	pTouch->SetWasPreHandled(false, GetParent());

	SetIsScrolling(false); 
	m_activeFinger = -1;
}

void ScrollComponent::OnOverMove(VariantList *pVList)
{
	uint32 fingerID = 0;
	fingerID = pVList->Get(2).GetUINT32();
	if (*m_pEnforceFingerTracking && m_activeFinger == -1)
	{
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
		if (pTouch->WasHandled()) return;
		m_activeFinger = fingerID;
	}

	if (!isInterestingFinger(fingerID)) return;
		
	//LogMsg("moved %s", PrintVector2(vDisplacement).c_str());

	if (*m_pScrollStyle == STYLE_EXACT)
	{
		m_vecDisplacement += pVList->m_variant[0].GetVector2()-m_lastTouchPos;
		m_vTotalDisplacementOnCurrentSwipe += pVList->m_variant[0].GetVector2()-m_lastTouchPos;
		if (m_bIsScrolling || *m_pSwipeDetectDistance != 0 || *m_pDontScrollUntilSwipeDetected == 0)
		{
			SetPosition(m_vecDisplacement, false);
			m_vecDisplacement = CL_Vec2f(0,0);
		}
	} else
	{
		m_vecDisplacement += (pVList->m_variant[0].GetVector2()-m_lastTouchPos)* *m_pPowerMod;
		m_vTotalDisplacementOnCurrentSwipe +=  pVList->m_variant[0].GetVector2()-m_lastTouchPos;
	}

	m_lastTouchPos = pVList->m_variant[0].GetVector2();

	//was it a big enough swipe to switch to scroll mode?  (scroll mode means they can't accidently hit any buttons/etc inside us)
	//using iPadMapX will roughly allow move movement on larger screens.. not going to be perfect unless we knew the pixels per cm of
	//the screen though.. hrm, not easy to get that.

	if (*m_pSwipeDetectDistance != 0 && m_vTotalDisplacementOnCurrentSwipe.length() > *m_pSwipeDetectDistance)
	{
		//TODO:  Should we check pos/size and if we don't require scrolling (everything fits on screen), not do the next call?
		SetIsScrolling(true);

		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
		pTouch->SetWasHandled(true, GetParent());
	}
}

void ScrollComponent::SetPosition(CL_Vec2f vDisplacement, bool bForceUpdate)
{
	if (vDisplacement == CL_Vec2f(0,0) && !bForceUpdate) return;
	
	/*
	//this works, but it still totally feels wrong, really I would need to turn "momentum" off as well, and perfectly tune the
	//sensitivity so it feels like you're dragging the scroll bar

	if (IsDesktop())
	{
		vDisplacement.y *= -1; //on desktops where you use a mouse, it makes more sense to drag the scroll bar down, rather than "drag the screen up"
	}
	*/

	m_vecChildPos += vDisplacement;

	//force it within our ranges
	ForceRange(m_vecChildPos.x, m_pBoundsRect->left, m_pBoundsRect->right);
	if (m_pBoundsRect->top > m_pBoundsRect->bottom) m_pBoundsRect->top = m_pBoundsRect->bottom;
	ForceRange(m_vecChildPos.y, m_pBoundsRect->top, m_pBoundsRect->bottom);
	
	CL_Vec2f percent2d(0,0);
	//avoid divide by 0 errors
	if (m_pBoundsRect->get_width() != 0) percent2d.x = m_vecChildPos.x/(-m_pBoundsRect->get_width());
	if (m_pBoundsRect->get_height() != 0) percent2d.y = m_vecChildPos.y/ (-m_pBoundsRect->get_height());

	m_progressVar->Set(percent2d);
	
	//also run this on all children
	EntityList *pChildren = GetParent()->GetChildren();

	Variant *pPos;
	EntityList::iterator itor = pChildren->begin();
	while (itor != pChildren->end())
	{
		pPos = (*itor)->GetVar("pos2d");
		pPos->Set(m_vecChildPos);
		itor++;
	}
}

void ScrollComponent::OnUpdate(VariantList *pVList)
{
	if (*m_pScrollStyle == STYLE_MOMENTUM)
	{
		if (m_bIsScrolling || GetBaseApp()->GetTotalActiveTouches() == 0 || *m_pSwipeDetectDistance == 0 || *m_pDontScrollUntilSwipeDetected == 0)
		{
			SetPosition(m_vecDisplacement*GetBaseApp()->GetDelta(), false);
			m_vecDisplacement *= (1- (*m_pFriction*GetBaseApp()->GetDelta()));
		}
	}
}
