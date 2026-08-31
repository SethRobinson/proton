#include "PlatformPrecomp.h"
#include "SliderComponent.h"
#include "util/GLESUtils.h"
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
	m_pVertical = &GetVarWithDefault("vertical", Variant(uint32(0)))->GetUINT32();

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
	//LogMsg("Progress set to %.2f, ", pDataObject->GetFloat());
	SetSliderPosition(pDataObject->GetFloat());
}

void SliderComponent::SetSliderPosition(float value)
{

	*m_pProgress = value;
	SetSliderPosition();
}


Entity * SliderComponent::GetSliderButton()
{
	if (!m_pSliderButton)
	{
		//the button entity usually doesn't exist yet when our OnAdd runs (CreateSlider adds us first), so locate it now
		m_pSliderButton = GetParent()->GetVar("sliderButton")->GetEntity();

		if (!m_pSliderButton)
		{
			m_pSliderButton = GetParent()->GetEntityByName("sliderButton");
		}
	}

	return m_pSliderButton;
}

float SliderComponent::GetTrackLength()
{
	return *m_pVertical ? m_pSize2d->y : m_pSize2d->x;
}

void SliderComponent::SetSliderPosition()
{
	if (!GetSliderButton()) return;

	CL_Vec2f vPos = m_pSliderButton->GetVar("pos2d")->GetVector2();
	if (*m_pVertical)
		vPos.y = m_pSize2d->y * *m_pProgress;
	else
		vPos.x = m_pSize2d->x * *m_pProgress;
	m_pSliderButton->GetVar("pos2d")->Set(vPos);
};

//along = distance from the start of the track, clamped here
void SliderComponent::SetKnobAlongTrack(float along)
{
	float length = GetTrackLength();
	ForceRange(along, 0.0f, length);

	CL_Vec2f vPos = m_pSliderButton->GetVar("pos2d")->GetVector2();
	if (*m_pVertical)
		vPos.y = along;
	else
		vPos.x = along;
	m_pSliderButton->GetVar("pos2d")->Set(vPos);

	GetVar("progress")->Set(length > 0 ? along / length : 0.0f);
}

void SliderComponent::UpdatePositionByTouch(CL_Vec2f pt)
{
	if (!GetSliderButton()) return;

	CL_Vec2f vPos = m_pSliderButton->GetVar("pos2d")->GetVector2();
	CL_Vec2f vDelta = pt - m_pClickStartPos;
	m_pClickStartPos = pt;
	SetKnobAlongTrack(*m_pVertical ? vPos.y + vDelta.y : vPos.x + vDelta.x);
}

void SliderComponent::SetPositionWithMouseClick(CL_Vec2f pt)
{
	if (!GetSliderButton()) return;

	//LogMsg("Clicked %s", PrintVector2(pt).c_str());
	SetKnobAlongTrack(*m_pVertical ? pt.y - m_pPos2d->y : pt.x - m_pPos2d->x);
}

void SliderComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
	{

		float paddingForClick = 12; //across the track, so a thin track is still easy to hit

		//the rect around the slider, so we can test if the pt is inside of it
		CL_Rectf rectSlider;
		if (*m_pVertical)
			rectSlider = CL_Rectf(m_pPos2d->x - paddingForClick, m_pPos2d->y, m_pPos2d->x + m_pSize2d->x + paddingForClick, m_pPos2d->y + m_pSize2d->y);
		else
			rectSlider = CL_Rectf(m_pPos2d->x, m_pPos2d->y - paddingForClick, m_pPos2d->x + m_pSize2d->x, m_pPos2d->y + m_pSize2d->y + paddingForClick);

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
	{
		int fingerID = pVList->Get(2).GetUINT32();
		TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(fingerID);

		if (pTouch->GetEntityThatHandledIt() == GetParent())
		{
			SetPositionWithMouseClick(pt);
		}

		break;
	}

	default:
		break; //don't care about the other message types
	}
}
