#include "PlatformPrecomp.h"
#include "TouchDragComponent.h"
#include "BaseApp.h"

TouchDragComponent::TouchDragComponent()
{
	m_activeFingerID = -1;
	m_pDisabled = NULL;
	SetName("TouchDrag");
}

TouchDragComponent::~TouchDragComponent()
{
	if (m_activeFingerID != -1)
	{
		//mark the touch we were using as unhandled now, so if we're recreated right at the same place controls don't
		//go dead until they release and touch again
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(m_activeFingerID);
		if (pTouch)
		{
			pTouch->SetWasHandled(false);
		}

	}
}

void TouchDragComponent::EndDrag(int fingerID, CL_Vec2f pt)
{
	if (m_activeFingerID == fingerID)
	{
		VariantList vList(pt, GetParent(), uint32(fingerID));
		GetParent()->GetFunction("OnOverEnd")->sig_function(&vList);
		m_activeFingerID = -1;
	}
}
void TouchDragComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_lastPos = CL_Vec2f(-1,-1);
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	m_pVisualStyle = &GetVarWithDefault("visualStyle", uint32(STYLE_NONE))->GetUINT32();
	m_pOnTouchDragUpdate = GetFunction("OnTouchDragUpdate");
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pSwapXAndY = &GetVar("swapXAndY")->GetUINT32();
	m_pReverseX = &GetVar("reverseX")->GetUINT32();
	m_pReverseY = &GetVar("reverseY")->GetUINT32();
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();

	m_pTouchPadding = &GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf()))->GetRect();

	//this will only be set if TouchDragComponent is initted before the TouchHandler...
	//GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(0.0f, 0.0f, 0.0f, 0.0f)))->GetRect();

	//register to get updated every frame
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchDragComponent::OnInput, this, _1));
}

void TouchDragComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void TouchDragComponent::SetPosition(CL_Vec2f vInputPos)
{
	//if (vPos == m_lastPos) return;
	
	//LogMsg("Setting position of %s", PrintVector2(vInputPos).c_str());
	CL_Vec2f vPos = vInputPos-m_lastPos;

	m_lastPos = vInputPos;

	if (*m_pSwapXAndY != 0)
	{
		swap(vPos.x, vPos.y);
	}
	
	if (*m_pReverseX != 0)
	{
		vPos.x *= -1;
	}
	if (*m_pReverseY != 0)
	{
		vPos.y *= -1;
	}

    VariantList vList(this, vPos);

	m_pOnTouchDragUpdate->sig_function(&vList);
}


void TouchDragComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	uint32 fingerID = 0;
	if (pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	//LogMsg("Detected finger %d at %s", fingerID, PrintVector2(pt).c_str());
	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		//first, determine if the click is on our area
		{

			if (*m_pDisabled != 0) return;

			CL_Rectf r(*m_pPos2d, CL_Sizef(m_pSize2d->x, m_pSize2d->y));

			ApplyPadding(&r, *m_pTouchPadding);
		
			if (r.contains(pt))
			{
						
				if (m_activeFingerID != -1)
				{
					//LogMsg("Ignoring new finger..");
					return;
				}
		
				TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
				if (pTouch->WasHandled()) return;
				pTouch->SetWasHandled(true);
                VariantList vList(pt, GetParent(), uint32(fingerID));
                
				GetParent()->GetFunction("OnOverStart")->sig_function(&vList);

				m_lastPos = pt;
				m_activeFingerID = fingerID;
			}
		}
		break;

	case MESSAGE_TYPE_GUI_CLICK_END:
		
		EndDrag(fingerID, pt);
	
		break;
	
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		

		if (*m_pDisabled != 0)
		{
			EndDrag(fingerID, pt);
			return;
		}

		if (m_activeFingerID == fingerID)
		{
			SetPosition(pt);
		}
	
		break;
        default: ;
	}	

}

void TouchDragComponent::SetLastPos( CL_Vec2d vLastPos )
{
	m_lastPos = vLastPos;
}
void TouchDragComponent::ModLastPos( CL_Vec2d vPos )
{
	m_lastPos += vPos;
}

void TouchDragComponent::ResetLastPos()
{
	if (m_activeFingerID != -1)
	{
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(m_activeFingerID);

		m_lastPos = pTouch->GetPos();
	}
}