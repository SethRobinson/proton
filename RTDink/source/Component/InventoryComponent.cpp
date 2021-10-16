#include "PlatformPrecomp.h"
#include "InventoryComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

InventoryComponent::InventoryComponent()
{
	m_activeFinger = -1;

	SetName("Inventory");
}

InventoryComponent::~InventoryComponent()
{
}

void InventoryComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pArrowEnt = NULL;
	m_bGotFirstClick = false; 
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();


	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&InventoryComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&InventoryComponent::OnUpdate, this, _1));

	AddInputMovementFocusIfNeeded(GetParent());

	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&InventoryComponent::OnInput, this, _1));

}

void InventoryComponent::OnRemove()
{
	EntityComponent::OnRemove();

}

void InventoryComponent::OnUpdatePos(CL_Vec2f vPos)
{
	//LogMsg("Got %s", PrintVector2(vPos).c_str());
	DinkSetInventoryPosition(NativeToDinkCoords(vPos));
}

void InventoryComponent::OnRender(VariantList *pVList)
{
	//CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void InventoryComponent::OnUpdate(VariantList *pVList)
{
}


void InventoryComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	//pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		{

		uint32 fingerID = pVList->Get(2).GetUINT32();
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);
		if (pTouch->WasHandled()) return;
		pTouch->SetWasHandled(true);
		m_activeFinger = fingerID;
		}

		OnUpdatePos(pt);
		break;

	case MESSAGE_TYPE_GUI_CLICK_END:
		{

		uint32 fingerID = pVList->Get(2).GetUINT32();
		if (fingerID == m_activeFinger)
		{

		OnUpdatePos(pt);
		/*
		if (!m_bGotFirstClick)
		{
			//ignore this, they are just releasing from the previous menu's button
			m_bGotFirstClick = true;
		} else
		{
		*/
			if (DinkSetInventoryPosition(NativeToDinkCoords(pt)))
			{
				g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = true;
				g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;
			}

		}
		}
		//HandleClickEnd(pt);
		break;
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		{
		uint32 fingerID = pVList->Get(2).GetUINT32();
		if (fingerID == m_activeFinger) OnUpdatePos(pt);
		}
		break;
	}	

}
