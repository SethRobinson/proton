#include "PlatformPrecomp.h"
#include "ScrollToZoomComponent.h"
#include "EntityUtils.h"
#include "TouchDragMoveComponent.h"

ScrollToZoomComponent::ScrollToZoomComponent()
{
	SetName("ScrollToZoom");
}

ScrollToZoomComponent::~ScrollToZoomComponent()
{
}

void ScrollToZoomComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVar("scale2d")->GetVector2();

	EntityComponent* pDragComponent = GetParent()->GetComponentByName("TouchDrag");

	if (!pDragComponent)
	{
		assert(0 && "ScrollToZoomComponent requires a TouchDragComponent to be added first");
		return;
	}

	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnInput, this, _1));
}

void ScrollToZoomComponent::OnInput(VariantList* pVList)
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	
	if (m_bIsDraggingLook)
	{

		switch (eMessageType(int(pVList->Get(0).GetFloat())))
		{
		case MESSAGE_TYPE_GUI_MOUSEWHEEL:
		{
			float wheelVal = pVList->Get(4).GetFloat(); //Note: Using Get(4) instead of (1) as of 9/13/2023

			//windows returns 120 or -120 per notch for historical reasons, but it almost never
			//will actually be 240 etc, so let's just assume one notch per messagebecause you know it's going to break linux/etc later if we don't

			if (wheelVal > 0) wheelVal = 1;
			if (wheelVal < 1) wheelVal = -1;

			float zoomPower = 0.05f;

			//	LogMsg("Mouse wheel: Offet: %.2f", wheelVal);
			float fDelta = ((float)wheelVal) * (m_pScale2d->x * zoomPower);

			GetParent()->GetVar("scale2d")->Set(*m_pScale2d + CL_Vec2f(fDelta, fDelta));
			char buf[256];
			sprintf(buf, "Scale: X: %.2f Y: %.2f", m_pScale2d->x, m_pScale2d->y);
			UpdateStatusMessage(buf); 
		}
			break;

		case MESSAGE_TYPE_GUI_CHAR:
		{
		#ifdef _DEBUG
					LogMsg("Got char: %c (%d)", (char)pVList->Get(2).GetUINT32(), pVList->Get(3).GetUINT32());
		#endif

			//If you want to know when a key is pressed while a mouse is holding down and moving this 'window'...
					pVList->Get(5).Set(GetParent()); //share what entity we below to
					m_sig_input_while_mousedown(pVList);
			
		}
		break;


		}
	}
}

void ScrollToZoomComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ScrollToZoomComponent::UpdateStatusMessage(string msg)
{

	if (GetVarWithDefault("showCoords", (uint32)1)->GetUINT32() == 0) return;


	int timeMS = 1000;

	Entity* pOldEnt = GetParent()->GetEntityByName("DebugText");
	if (pOldEnt)
	{
		pOldEnt->SetName("");
		pOldEnt->SetTaggedForDeletion();
	}

	Entity* pEnt = CreateTextLabelEntity(GetParent(), "DebugText", 0, 0, msg);
	SetupTextEntity(pEnt, FONT_LARGE, 0.66f);
	FadeOutAndKillEntity(pEnt, true, 100, timeMS);
}

void ScrollToZoomComponent::OnTouchDragUpdate(VariantList* pVList)
{
	CL_Vec2f vMovement = pVList->Get(1).GetVector2();

#ifdef _DEBUG
	//LogMsg("offset %s", PrintVector2(vMovement).c_str());
#endif
	
}

void ScrollToZoomComponent::OnOverStart(VariantList* pVList)
{
	m_bIsDraggingLook = true;
}

void ScrollToZoomComponent::OnOverEnd(VariantList* pVList)
{
	m_bIsDraggingLook = false;
}
