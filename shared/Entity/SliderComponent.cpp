#include "PlatformPrecomp.h"
#include "SliderComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

SliderComponent::SliderComponent()
{
	SetName("Slider");
}

SliderComponent::~SliderComponent()
{
	
}

void SliderComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pClickStartPos = CL_Vec2f(0,0);
	m_sliderButtonSelected = false;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();

	m_pColor = &GetParent()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	
	m_pSliderButton = GetParent()->GetVarWithDefault("sliderButton", (Entity*)NULL)->GetEntity();
	
	m_pProgress = &GetVarWithDefault("progress", Variant(0.0f))->GetFloat();

	//register ourselves to render if the parent does
	//GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&SliderComponent::OnRender, this, _1));
	//GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&SliderComponent::OnUpdate, this, _1));
	GetVar("progress")->GetSigOnChanged()->connect(boost::bind(&SliderComponent::OnProgressChanged, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&SliderComponent::OnInput, this, _1));

}

void SliderComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void SliderComponent::OnRender(VariantList *pVList)
{
}

void SliderComponent::OnUpdate(VariantList *pVList)
{
}

void SliderComponent::OnProgressChanged(Variant *pDataObject)
{
	SetSliderPosition();
}

void SliderComponent::SetSliderPosition(float value)
{

	*m_pProgress = value;
	SetSliderPosition();
}

void SliderComponent::SetSliderPosition()
{
	if (!m_pSliderButton)
	{
		m_pSliderButton = GetParent()->GetEntityByName("sliderButton");
	    return;
	}
//	assert(m_pSliderButton && "Must set var 'sliderButton' to a valid entity");
	
	CL_Vec2f vPos = m_pSliderButton->GetVar("pos2d")->GetVector2();
	vPos.x = m_pSize2d->x* *m_pProgress;
	m_pSliderButton->GetVar("pos2d")->Set(vPos);
};

void SliderComponent::UpdatePositionByTouch(CL_Vec2f pt)
{
		CL_Vec2f vPos = m_pSliderButton->GetVar("pos2d")->GetVector2();
		vPos.x += (pt-m_pClickStartPos).x;
		ForceRange(vPos.x, 0, m_pSize2d->x);
		m_pSliderButton->GetVar("pos2d")->Set(vPos);
		m_pClickStartPos = pt;

		GetVar("progress")->Set(vPos.x / m_pSize2d->x);
}

void SliderComponent::SetPositionWithMouseClick(CL_Vec2f pt)
{
	CL_Vec2f vOrigPos = GetPos2DEntity(m_pSliderButton);
	vOrigPos.x = pt.x - m_pPos2d->x;

	//limit to bounds
	ForceRange(vOrigPos.x, 0, m_pSize2d->x);
	//LogMsg("Clicked %s - Setting pos to %s", PrintVector2(pt).c_str(), PrintVector2(vOrigPos).c_str());
	SetPos2DEntity(m_pSliderButton, vOrigPos);
	GetVar("progress")->Set(vOrigPos.x / m_pSize2d->x);
}

void SliderComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
	{

		float paddingYForClick = 12;

		//the rect around the slider, so we can test if the pt is inside of it
		CL_Rectf rectSlider = CL_Rectf(m_pPos2d->x, m_pPos2d->y- paddingYForClick, m_pPos2d->x + m_pSize2d->x, m_pPos2d->y + m_pSize2d->y+ paddingYForClick);

		if (rectSlider.contains(pt) == false)
		{
			//not inside the slider, so we don't care
			//LogMsg("Rejecting point, pt is %s and rect is %s", PrintVector2(pt).c_str(), PrintRect(rectSlider).c_str());
			return;
		}

		m_pClickStartPos = pt;
		int fingerID = pVList->Get(2).GetUINT32();
		TouchTrackInfo* pTouch = GetBaseApp()->GetTouch(fingerID);
		pTouch->SetWasHandled(true, GetParent());
		SetPositionWithMouseClick(pt);
		break;
	}

	case MESSAGE_TYPE_GUI_CLICK_END:
	{
		int fingerID = pVList->Get(2).GetUINT32();
		TouchTrackInfo* pTouch = GetBaseApp()->GetTouch(fingerID);

		if (pTouch->GetEntityThatHandledIt() == GetParent())
		{
			SetPositionWithMouseClick(pt);
		}
	}

	break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE:

		int fingerID = pVList->Get(2).GetUINT32();
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

		if (pTouch->GetEntityThatHandledIt() == GetParent())
		{
			SetPositionWithMouseClick(pt);
		}
	
		break;
	}	
}
