#include "PlatformPrecomp.h"
#include "TouchTestComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

TouchTestComponent::TouchTestComponent()
{
	SetName("TouchTest");
}

TouchTestComponent::~TouchTestComponent()
{

}


void TouchTestComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	
	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale2d = &GetParent()->GetShared()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&TouchTestComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&TouchTestComponent::OnUpdate, this, _1));


	//we want the touch messages
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&TouchTestComponent::OnInput, this, _1));

	for (int i=0; i < MAX_TOUCHES_AT_ONCE; i++)
	{
		m_touch[i].m_color = GetBrightColor();
	}
}

void TouchTestComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void TouchTestComponent::DrawTouch(uint32 touchID, CL_Vec2f vPos)
{
	DrawFilledSquare(vPos.x, vPos.y, 40, m_touch[touchID].m_color, true);
	GetBaseApp()->GetFont(FONT_SMALL)->Draw(vPos.x-5, vPos.y-20, toString(touchID), MAKE_RGBA(0,0,0,255));
}


void TouchTestComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;

	for (int i=0; i < MAX_TOUCHES_AT_ONCE; i++)
	{
		if (m_touch[i].m_bActive)
		{
			DrawTouch(i, vFinalPos+m_touch[i].m_vPos);
		}
	}
}

void TouchTestComponent::OnUpdate(VariantList *pVList)
{
	
}



void TouchTestComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset

	eMessageType type = eMessageType( (int) pVList->Get(0).GetFloat());

	CL_Vec2f pt = pVList->Get(1).GetVector2();

	
	uint32 finger = 0;
	
	switch (type)
	{
		case MESSAGE_TYPE_GUI_CLICK_START:
		case MESSAGE_TYPE_GUI_CLICK_END:
		case MESSAGE_TYPE_GUI_CLICK_MOVE:

		//it's a touch message, extract the fingerID now
		finger = pVList->Get(2).GetUINT32();
		if (finger >= MAX_TOUCHES_AT_ONCE)
		{
			assert(!"impossible!");
			return;
		}
	}

	switch (type)
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		m_touch[finger].m_bActive = true;		
		m_touch[finger].m_vPos = pt;
		break;

	case MESSAGE_TYPE_GUI_CLICK_END:
		m_touch[finger].m_bActive = false;		
		m_touch[finger].m_vPos = pt;
		break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		m_touch[finger].m_vPos = pt;
		break;
	}	

}