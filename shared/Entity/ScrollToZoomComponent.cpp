#include "PlatformPrecomp.h"
#include "ScrollToZoomComponent.h"
#include "EntityUtils.h"

ScrollToZoomComponent::ScrollToZoomComponent()
{
	
	SetName("TouchDragMove");
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
	//pDragComponent->GetFunction("OnTouchDragUpdate")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnTouchDragUpdate, this, _1));
	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&ScrollToZoomComponent::OnInput, this, _1));

}

void  ScrollToZoomComponent::OnInput(VariantList* pVList)
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	switch (eMessageType(int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_MOUSEWHEEL:
		if (m_bIsDraggingLook)
		{
			float wheelVal = pVList->Get(4).GetFloat(); //Note: Using Get(4) instead of (1) as of 9/13/2023

			//windows returns 120 or -120 per notch for historical reasons, but it almost never
			//will actually be 240 etc, so let's just assume one notch per messagebecause you know it's going to break linux/etc later if we don't

			if (wheelVal > 0) wheelVal = 1;
			if (wheelVal < 1) wheelVal = -1;
	
			float zoomPower = 0.05f;

		//	LogMsg("Mouse wheel: Offet: %.2f", wheelVal);
			float fDelta = ((float)wheelVal)* (m_pScale2d->x * zoomPower);
		
			//change scale by using GetParent()->GetVar() instead of m_pScale2d directly so that the OnChanged signal is fired
			GetParent()->GetVar("scale2d")->Set(*m_pScale2d + CL_Vec2f(fDelta, fDelta));

			UpdateStatusMessage("Scale: X: " + toString(m_pScale2d->x) + " Y: " + toString(m_pScale2d->y));
		}
		break;
	}
}

void ScrollToZoomComponent::OnRemove()
{
	EntityComponent::OnRemove();
}


void ScrollToZoomComponent::UpdateStatusMessage(string msg)
{

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
