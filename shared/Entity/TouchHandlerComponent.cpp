#include "PlatformPrecomp.h"

#include "TouchHandlerComponent.h"
#include "Manager/MessageManager.h"
#include "util/MiscUtils.h"
#include "BaseApp.h"

TouchHandlerComponent::TouchHandlerComponent()
{
	SetName("TouchHandler");
}

TouchHandlerComponent::~TouchHandlerComponent()
{
}

void TouchHandlerComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pTouchOver = GetParent()->GetVar("touchOver");
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pTouchPadding = &GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(20.0f, 5.0f, 20.0f, 15.0f)))->GetRect();
	m_pIgnoreTouchesOutsideRect = &GetParent()->GetVar("ignoreTouchesOutsideRect")->GetUINT32();

	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchHandlerComponent::OnInput, this, _1));
	//GetFunction("EndClick")->sig_function.connect(1, boost::bind(&TouchHandlerComponent::EndClick, this, _1));

	GetParent()->GetVar("pos2d")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerComponent::UpdateTouchArea, this, _1));
	GetParent()->GetVar("size2d")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerComponent::UpdateTouchArea, this, _1));
	GetParent()->GetVar("touchPadding")->GetSigOnChanged()->connect(boost::bind(&TouchHandlerComponent::UpdateTouchArea, this, _1));

	UpdateTouchArea(NULL);

	EntityComponent::OnAdd(pEnt);
}

void TouchHandlerComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TouchHandlerComponent::HandleClickStart(CL_Vec2f &pt, uint32 fingerID)
{
	if (m_pTouchOver->GetUINT32())
	{
		//LogMsg("Ignoring click start because we're already 'down'")	;
		return;
	}

	//first, determine if the click is on our area
	if (m_touchArea.contains(pt))
	{
		m_pTouchOver->Set(uint32(1));
		VariantList vList(pt, GetParent(), fingerID, uint32(true));
        
		GetParent()->GetFunction("OnTouchStart")->sig_function(&vList);
		GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
	}
}

void TouchHandlerComponent::HandleClickMove( CL_Vec2f &pt, uint32 fingerID )
{
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

	if (*m_pIgnoreTouchesOutsideRect != 0)
	{
		if (!m_touchArea.contains(pt))
		{
			//ignore this
			return;
		}
	}

	if (m_pTouchOver->GetUINT32())
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
		}
	} else
	{
		//currently not over, but did we move onto it?
		if (m_touchArea.contains(pt))
		{
			m_pTouchOver->Set(uint32(1));
            VariantList vList(pt, GetParent(), fingerID, uint32(true));
            
			GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
		} else
		{
			//not on it, do nothing
		}

	}
}

void TouchHandlerComponent::HandleClickEnd( CL_Vec2f &pt, uint32 fingerID )
{
	if (!m_pTouchOver->GetUINT32()) return;

	const bool touchAreaContainsTouchPoint = m_touchArea.contains(pt);

	if (*m_pIgnoreTouchesOutsideRect != 0)
	{
		if (!touchAreaContainsTouchPoint)
		{
			//they lifted a finger, but not in our rect so ignore it
			return;
		}
	}

	m_pTouchOver->Set(uint32(0));
    VariantList vList(pt, GetParent(), fingerID, uint32(touchAreaContainsTouchPoint));

	GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);

	if (touchAreaContainsTouchPoint)
	{
		GetParent()->GetFunction("OnTouchEnd")->sig_function(&vList);
	}
}

void TouchHandlerComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	uint32 fingerID = 0;
	if (pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
		case MESSAGE_TYPE_GUI_CLICK_START:
			HandleClickStart(pt, fingerID);
			break;
		case MESSAGE_TYPE_GUI_CLICK_END:
			HandleClickEnd(pt, fingerID);
			break;
		case MESSAGE_TYPE_GUI_CLICK_MOVE:
			HandleClickMove(pt, fingerID);
			break;
        default:;
	}	
	
}

void TouchHandlerComponent::UpdateTouchArea(Variant *v)
{
	m_touchArea.set_top_left(*m_pPos2d);
	m_touchArea.set_size(CL_Sizef(m_pSize2d->x, m_pSize2d->y));
	ApplyPadding(&m_touchArea, *m_pTouchPadding);
}
