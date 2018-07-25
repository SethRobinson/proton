#include "PlatformPrecomp.h"

#include "TouchHandlerArcadeComponent.h"
#include "Manager/MessageManager.h"
#include "util/MiscUtils.h"
#include "BaseApp.h"

const float C_PERCENT_OF_PINCH_TO_DETECT_IT = 0.05f;

TouchHandlerArcadeComponent::TouchHandlerArcadeComponent()
{
	m_pDisabled = NULL;
	m_activeFinger = -1;
	m_secondFinger = -1;
	m_pDontClaimOwnership = NULL;
	SetName("TouchHandlerArcade");
	m_bIsPinching = false;
}

TouchHandlerArcadeComponent::~TouchHandlerArcadeComponent()
{
	ReleaseTouchIfNeeded();
}

void TouchHandlerArcadeComponent::ReleaseTouchIfNeeded()
{
	if (m_activeFinger != -1)
	{
		//mark the touch we were using as unhandled now, so if we're recreated right at the same place controls don't
		//go dead until they release and touch again
		if (m_pDontClaimOwnership && *m_pDontClaimOwnership == 0)
		{
			TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(m_activeFinger);

			if (pTouch)
			{
				pTouch->SetWasHandled(false);
			}
		}
	}
}
void TouchHandlerArcadeComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pTouchOver = GetParent()->GetVar("touchOver");
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pTouchPadding = &GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(20.0f, 5.0f, 20.0f, 15.0f)))->GetRect();
	m_pAllowSlideOns = &GetVarWithDefault("allowSlideOns", uint32(1))->GetUINT32();
	m_pDontClaimOwnership = &GetVarWithDefault("dontClaimOwnerShip", uint32(0))->GetUINT32();

	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchHandlerArcadeComponent::OnInput, this, _1));
	//GetFunction("EndClick")->sig_function.connect(1, boost::bind(&TouchHandlerArcadeComponent::EndClick, this, _1));

	GetParent()->GetVar("pos2d")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerArcadeComponent::UpdateTouchArea, this, _1));
	GetParent()->GetVar("size2d")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerArcadeComponent::UpdateTouchArea, this, _1));
	GetParent()->GetVar("touchPadding")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerArcadeComponent::UpdateTouchArea, this, _1));
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();

	UpdateTouchArea(NULL);

	EntityComponent::OnAdd(pEnt);
}

void TouchHandlerArcadeComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TouchHandlerArcadeComponent::HandleClickStart(CL_Vec2f &pt, uint32 fingerID)
{
	if (m_pTouchOver->GetUINT32())
	{
		LogMsg("Ignoring click start because we're already 'down'.  Shouldn't be possible really.")	;
		return;
	}

	//first, determine if the click is on our area
	if (m_touchArea.contains(pt))
	{
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
		
			if (*m_pDontClaimOwnership == 1)
			{
				if (m_activeFinger == fingerID)
				{
					ReleaseClick(pt, fingerID);
				}
			}
		
		if (*m_pDontClaimOwnership == 0)
		{
			pTouch->SetWasHandled(true, GetParent());
		}
		
		m_activeFinger = fingerID;
		m_pTouchOver->Set(uint32(1));
		VariantList vList(pt, GetParent(), fingerID, uint32(true));

		GetParent()->GetFunction("OnTouchStart")->sig_function(&vList);
		GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
	}
}

void TouchHandlerArcadeComponent::HandleClickMove( CL_Vec2f &pt, uint32 fingerID )
{
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

	if (fingerID == m_activeFinger)
	{
		//currently over, did we move off?
		if (m_touchArea.contains(pt))
		{
			//still over, do nothing?
			FunctionObject *pFunc = GetParent()->GetShared()->GetFunctionIfExists("OnOverMove");

			if (pFunc)
			{
				// I guess someone cares about knowing if it moves as well
				VariantList vList(pt, GetParent(), fingerID, uint32(true));
				pFunc->sig_function(&vList);
			}

		} else
		{
			m_pTouchOver->Set(uint32(0));
			VariantList vList(pt, GetParent(), fingerID, uint32(false));
			GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);
			m_activeFinger = -1;
			m_secondFinger = -1;
			EndPinchIfNeeded();

			if (*m_pDontClaimOwnership == 0)
			{
				pTouch->SetWasHandled(false);
			}
		}
	} else
	{
		//currently not over, but did we move onto it?
		if (*m_pAllowSlideOns != 0 && m_touchArea.contains(pt))
		{
			m_pTouchOver->Set(uint32(1));
			VariantList vList(pt, GetParent(), fingerID, uint32(true));
			
			if (*m_pDontClaimOwnership == 0)
			{
				pTouch->SetWasHandled(true, GetParent());
			}
			m_activeFinger = fingerID;
			GetParent()->GetFunction("OnTouchStart")->sig_function(&vList);
			GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
	
} else
		{
			//not on it, do nothing
		}

	}
}

void TouchHandlerArcadeComponent::HandleClickEnd( CL_Vec2f &pt, uint32 fingerID )
{
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

	if (m_activeFinger != fingerID)
	{
		return;
	}
	
	const bool touchAreaContainsTouchPoint = m_touchArea.contains(pt);
	m_pTouchOver->Set(uint32(0));
	VariantList vList(pt, GetParent(), fingerID, uint32(touchAreaContainsTouchPoint));

	GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);

	if (touchAreaContainsTouchPoint)
	{
		GetParent()->GetFunction("OnTouchEnd")->sig_function(&vList);
	}

	m_secondFinger = -1;
	m_activeFinger = -1;
	EndPinchIfNeeded();

	if (*m_pDontClaimOwnership == 0)
	{
		pTouch->SetWasHandled(false);
	}

}


void TouchHandlerArcadeComponent::EndPinchIfNeeded()
{
	if (m_bIsPinching)
	{
		m_bIsPinching = false;
		GetParent()->GetFunction("OnPinchEnd")->sig_function(NULL);
	}
}

void TouchHandlerArcadeComponent::ReleaseClick(CL_Vec2f vPt, uint32 fingerID)
{
	//um, we were tracking this and someone else claimed ownship?!  Ok, release it.
	m_pTouchOver->Set(uint32(0));
	VariantList vList(vPt, GetParent(), fingerID, uint32(false));
	GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);
	m_activeFinger = -1;
	m_secondFinger = -1;
	
	EndPinchIfNeeded();

}

void TouchHandlerArcadeComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	uint32 fingerID = 0;
	if (pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	} else
	{
		//what is this?
		return;
	}

	TouchTrackInfo *pTouch;

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{

	case MESSAGE_TYPE_GUI_CLICK_START:
	case MESSAGE_TYPE_GUI_CLICK_END:
	case MESSAGE_TYPE_GUI_CLICK_MOVE:

		pTouch = GetBaseApp()->GetTouch(fingerID);
		
		if (pTouch->WasHandled() && pTouch->GetEntityThatHandledIt() != GetParent())
		{
			if (*m_pDontClaimOwnership == 1)
			{
				if (m_activeFinger == fingerID)
				{
					ReleaseClick(pt, fingerID);
				}
			}
			return;
		}

		if (pTouch->WasPreHandled())
		{
			if (*m_pDontClaimOwnership == 1)
			{
				if (m_activeFinger == fingerID)
				{
					ReleaseClick(pt, fingerID);
				}
			}
			return; //a scroll box marks if this way, if a scroller is in front of us,
		}
		//we don't want to claim ownership of this
		break;

	default:;
		return;
	}

	if (m_activeFinger != -1 && m_activeFinger != fingerID)
	{
		//don't care, we're tracking something else right now
		//just kidding, we do, because it's a second finger that could be used for pinching
		switch (eMessageType( int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_CLICK_START:
			if (*m_pDisabled != 0) return;
			HandleClickStartSecond(pt, fingerID);
			break;

		case MESSAGE_TYPE_GUI_CLICK_MOVE:
			if (*m_pDisabled != 0) return;
			HandleClickMoveSecond(pt, fingerID);
			break;

		case MESSAGE_TYPE_GUI_CLICK_END:

			HandleClickEndSecond(pt, fingerID);
			break;

		default:;
		}	
		return;
	}


	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
            
	case MESSAGE_TYPE_GUI_CLICK_START:
			if (*m_pDisabled != 0) return;
            HandleClickStart(pt, fingerID);
            break;
    
        case MESSAGE_TYPE_GUI_CLICK_MOVE:
			if (*m_pDisabled != 0) return;
            HandleClickMove(pt, fingerID);
            break;

		case MESSAGE_TYPE_GUI_CLICK_END:
			HandleClickEnd(pt, fingerID);
			break;

        default:;
	}	

}

void TouchHandlerArcadeComponent::UpdateTouchArea(Variant *v)
{
	m_touchArea.set_top_left(*m_pPos2d);
	m_touchArea.set_size(CL_Sizef(m_pSize2d->x, m_pSize2d->y));
	ApplyPadding(&m_touchArea, *m_pTouchPadding);
}


//a second finger has touched
void TouchHandlerArcadeComponent::HandleClickStartSecond(CL_Vec2f &pt, uint32 fingerID)
{

	if (m_secondFinger != -1)
	{
		//we're already tracking a second finger though
		LogMsg("Ignoring finger, already tracking two");
		return;
	}

	//first, determine if the click is on our area
	if (m_touchArea.contains(pt))
	{
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
		
		m_secondFinger = fingerID;

		//remember locations of the two fingers
		m_secondFingerStartPos = pTouch->GetPos();
		m_fingerStartPos = GetBaseApp()->GetTouch(m_activeFinger)->GetPos();
	}
}

void TouchHandlerArcadeComponent::HandleClickMoveSecond( CL_Vec2f &pt, uint32 fingerID )
{
	if (m_secondFinger != fingerID) return; //not us

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
	
	float originalDist = (m_fingerStartPos-m_secondFingerStartPos).length();
	float newDist = (GetBaseApp()->GetTouch(m_activeFinger)->GetPos() - pTouch->GetPos()).length();

	float finalMod = (originalDist-newDist)/ GetScreenSize().length();

	//LogMsg("Pinch Offset is %.2f (pinch mod: %.2f)", originalDist-newDist, finalMod );


	if (m_bIsPinching || fabs(finalMod) > C_PERCENT_OF_PINCH_TO_DETECT_IT)
	{
		if (!m_bIsPinching)
		{
			m_bIsPinching = true;
			GetParent()->GetFunction("OnPinchStart")->sig_function(NULL);
		}
		
		VariantList vList(GetParent(), -finalMod);
		GetParent()->GetFunction("OnPinchMod")->sig_function(&vList);

		//reset last pos
		m_secondFingerStartPos = pTouch->GetPos();
		m_fingerStartPos = GetBaseApp()->GetTouch(m_activeFinger)->GetPos();
	}
    
}

void TouchHandlerArcadeComponent::HandleClickEndSecond( CL_Vec2f &pt, uint32 fingerID )
{
	if (m_secondFinger != fingerID) return; //not us
	m_secondFinger = -1;
	EndPinchIfNeeded();
}
