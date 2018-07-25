#include "PlatformPrecomp.h"
#include "TouchStripComponent.h"
#include "BaseApp.h"

TouchStripComponent::TouchStripComponent()
{
	m_activeFinger = -1;
	SetName("TouchStrip");
}

TouchStripComponent::~TouchStripComponent()
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

void TouchStripComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_lastPos = CL_Vec2f(-1,-1);
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	m_pVisualStyle = &GetVarWithDefault("visualStyle", uint32(STYLE_NONE))->GetUINT32();
	m_pOnTouchStripUpdate = GetParent()->GetFunction("OnTouchStripUpdate");
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pMult = &GetVarWithDefault("mult", CL_Vec2f(1,1))->GetVector2();
	m_pSwapXAndY = &GetVar("swapXAndY")->GetUINT32();
	m_pReverseX = &GetVar("reverseX")->GetUINT32();
	m_pReverseY = &GetVar("reverseY")->GetUINT32();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pTouchPadding = &GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(20.0f, 5.0f, 20.0f, 15.0f)))->GetRect();

	//this will only be set if TouchStripComponent is initted before the TouchHandler...
	//GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(0.0f, 0.0f, 0.0f, 0.0f)))->GetRect();

	//register to get updated every frame

	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&TouchStripComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchStripComponent::OnInput, this, _1));

	GetParent()->GetFunction("PrintDiagnostics")->sig_function.connect(1, boost::bind(&TouchStripComponent::OnPrintDiagnostics, this, _1));

}

void TouchStripComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void TouchStripComponent::OnOverStart(VariantList *pVList)
{

#ifdef _DEBUG
	if (GetParent()->GetComponentByName("TouchHandler"))
	{
		assert(!"TouchStrip was changed so it doesn't require a TouchHandlerComponent in its parent entity.  Remove it for things to work right!");
	}
#endif
	uint32 fingerID = pVList->Get(2).GetUINT32();
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
	if (pTouch->WasHandled()) return;
	pTouch->SetWasHandled(true);
	m_activeFinger = fingerID;
	SetPosition(pVList->Get(0).GetVector2());

	//LogMsg("Tracking finger %d", fingerID);
}

void TouchStripComponent::OnPrintDiagnostics(VariantList *pVList)
{
	LogMsg("TouchStripComponent: m_activeFinger is %d.  Disabled is %d", m_activeFinger, *m_pDisabled);
	
}

void TouchStripComponent::OnInput(VariantList *pVList)
{

	CL_Vec2f pt = pVList->Get(1).GetVector2();

	//0 = message type, 1 = parent coordinate offset

	uint32 fingerID = 0;
	if (pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	
	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{

	case MESSAGE_TYPE_GUI_CLICK_START:
		{

		//first, determine if the click is on our area
		CL_Rectf r(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));
		ApplyPadding(&r, *m_pTouchPadding);

		if (r.contains(pt))
		{
			if (m_activeFinger != -1)
			{
				//LogMsg("Ignoring new finger..");
				return;
			}

            VariantList vList(pt, GetParent(), fingerID);
			GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
		}
		}

		break;
	
	case MESSAGE_TYPE_GUI_CLICK_END:
		
		if (fingerID == m_activeFinger)
		{
		//	LogMsg("Finger %d released", fingerID);
			VariantList vList(pt, GetParent(), fingerID);
            GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);
			m_activeFinger = -1;
		}
		break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		{
			
			if (m_activeFinger != fingerID)
			{
				//not ours.. but hold on, if it's an unclaimed touch let's take it anyway
				if (m_activeFinger == -1)			
				{
			
					//well, nobody has claimed it yet.  Tell you what, if they are over us, let's let it count as a first touch
					CL_Rectf r(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));
					ApplyPadding(&r, *m_pTouchPadding);

					if (r.contains(pt))
					{
						//LogMsg("Touchstrip: Claimed finger %d", fingerID);
						VariantList vList(pt, GetParent(), fingerID);
						GetParent()->GetFunction("OnOverStart")->sig_function(&vList);
					}
					
				}

				return;
			}
		
			SetPosition(pt);
			
		}	
		break;	
	
        default: ;
	}	
}


void TouchStripComponent::SetPosition(CL_Vec2f vPos)
{
	//if (vPos == m_lastPos) return;
	m_lastPos = vPos;
	if (vPos.x < m_pPos2d->x) vPos.x = m_pPos2d->x;
	if (vPos.x > m_pPos2d->x + m_pSize2d->x) vPos.x = m_pPos2d->x + m_pSize2d->x;

	if (vPos.y < m_pPos2d->y) vPos.y = m_pPos2d->y;
	if (vPos.y > m_pPos2d->y + m_pSize2d->y) vPos.y = m_pPos2d->y + m_pSize2d->y;
	
	//convert to 0..1 range
	vPos -= *m_pPos2d;

	vPos.x = vPos.x / m_pSize2d->x;	
	vPos.y = vPos.y / m_pSize2d->y;

	if (*m_pSwapXAndY != 0)
	{
		swap(vPos.x, vPos.y);
	}
	if (*m_pReverseX != 0)
	{
		vPos.x = 1-vPos.x;
	}
	if (*m_pReverseY != 0)
	{
		vPos.y = 1-vPos.y;
	}
	vPos.x *= m_pMult->x;
	vPos.y *= m_pMult->y;
	
    VariantList vList(this, vPos);
	m_pOnTouchStripUpdate->sig_function(&vList);
}

