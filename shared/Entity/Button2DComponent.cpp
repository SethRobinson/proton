#include "PlatformPrecomp.h"

#include "Button2DComponent.h"
#include "BaseApp.h"
#include "Entity/EntityUtils.h"

//Flascc didn't like a global string, but I can new() it without errors, so this is a hack to make sure it gets
//deleted as well

string *g_defaultButtonClickSound = NULL;


class Button2DFlashHackInit
{
public:

	Button2DFlashHackInit()
	{
		//uh, this didn't work to fix flash, will do it a different way, must instiate too early for it
		//g_defaultButtonClickSound =  new string("audio/click.wav");
	}

	~Button2DFlashHackInit()
	{
		SAFE_DELETE(g_defaultButtonClickSound);
	}

} g_flashHackInit;


Button2DComponent::eButtonStyle g_defaultButtonStyle = Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH;

void SetDefaultButtonStyle(Button2DComponent::eButtonStyle style)
{
	g_defaultButtonStyle = style;
}

void SetDefaultAudioClickSound(string fileName)
{
	SAFE_DELETE(g_defaultButtonClickSound);
	g_defaultButtonClickSound = new string (fileName);
}

Button2DComponent::Button2DComponent() :
    m_pressed(false)
{
	SetName("Button2D");
}

Button2DComponent::~Button2DComponent()
{
}

void Button2DComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	//when area is clicked, OnButtonSelected is called on the parent entity

	if (!g_defaultButtonClickSound)
	{
		g_defaultButtonClickSound =  new string("audio/click.wav");
	}

	m_pOnClickAudioFile = &GetVarWithDefault("onClickAudioFile", Variant(*g_defaultButtonClickSound))->GetString();
	m_pDisabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	m_pRepeatDelayMS = &GetVarWithDefault("repeatDelayMS", uint32(250))->GetUINT32();
	m_pVisualStyle = &GetVarWithDefault("visualStyle", uint32(STYLE_FADE_ALPHA_ON_HOVER))->GetUINT32();
	m_pButtonStyle = &GetVarWithDefault("buttonStyle", uint32(g_defaultButtonStyle))->GetUINT32();
	m_pFileName = &GetVar("fileName")->GetString();
	m_pOverFileName = &GetVar("overFileName")->GetString();
	m_pTouchOver = &GetParent()->GetVar("touchOver")->GetUINT32();
	m_pAlpha = &GetParent()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	m_pTouchPadding = &GetParent()->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf()))->GetRect();
	m_pVisible = &GetParent()->GetVarWithDefault("visible", uint32(1))->GetUINT32();

	m_repeatTimer = 0;

	GetParent()->GetFunction("PerformClick")->sig_function.connect(1, boost::bind(&Button2DComponent::PerformClick, this, _1));

	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&Button2DComponent::OnOverStart, this, _1));
	GetParent()->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&Button2DComponent::OnOverMove, this, _1));
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&Button2DComponent::OnOverEnd, this, _1));
	GetParent()->GetFunction("OnTouchEnd")->sig_function.connect(1, boost::bind(&Button2DComponent::OnTouchEnd, this, _1));
	GetParent()->GetFunction("OnTouchStart")->sig_function.connect(1, boost::bind(&Button2DComponent::OnTouchStart, this, _1));
	GetVar("fileName")->GetSigOnChanged()->connect(boost::bind(&Button2DComponent::UpdateButtonVisuals, this, _1));
	GetVar("overFileName")->GetSigOnChanged()->connect(boost::bind(&Button2DComponent::UpdateButtonVisuals, this, _1));
	GetVar("visualStyle")->GetSigOnChanged()->connect(boost::bind(&Button2DComponent::OnVisualStyleChanged, this, _1));
}

void Button2DComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void Button2DComponent::OnVisualStyleChanged(Variant *pVariant)
{
	if (*m_pVisualStyle == STYLE_INVISIBLE_UNTIL_CLICKED)
	{
		m_alphaSave = *m_pAlpha;
		*m_pAlpha = 0;
	}
}

void Button2DComponent::UpdateButtonVisuals(Variant *pVariant)
{
	if (m_pFileName->empty() && m_pOverFileName->empty()) return;
	
	EntityComponent *pComp = GetParent()->GetComponentByName("OverlayRender");
	
	if (*m_pTouchOver == 0)
	{
		pComp->GetVar("fileName")->Set(*m_pFileName);
	} else
	{
		pComp->GetVar("fileName")->Set(*m_pOverFileName);
	}
}

void Button2DComponent::OnOverStart(VariantList *pVList)
{

	switch (*m_pVisualStyle)
	{
	case STYLE_FADE_ALPHA_ON_HOVER:
		m_alphaSave = *m_pAlpha;
		break;

	case STYLE_SCALE_DOWN_ON_HOVER:
		m_scale2dSave = *m_pScale2d;
		m_touchPaddingSave = *m_pTouchPadding;
		break;
	}

	if (*m_pButtonStyle == BUTTON_STYLE_CLICK_ON_TOUCH_PRESS_RELEASE && !m_pressed) {
		return;
	}

	UpdateButtonVisuals(NULL);

	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (pTouch->WasHandled()) return;

	if (*m_pDisabled == 0 && *m_pVisible != 0 && m_repeatTimer < GetBaseApp()->GetTick())
	{
		switch (*m_pVisualStyle)
		{
		case STYLE_FADE_ALPHA_ON_HOVER:
			GetParent()->GetVar("alpha")->Set(m_alphaSave*0.5f);
			break;

		case STYLE_SCALE_DOWN_ON_HOVER:
			CL_Vec2f originalSize(GetParent()->GetVar("size2d")->GetVector2());
			GetParent()->GetVar("scale2d")->Set(m_scale2dSave * 0.9f);
			CL_Vec2f scaledDownSize(GetParent()->GetVar("size2d")->GetVector2());
			// Enlarge the touch padding temporarily so that the touch area matches the original, non-scaled one
			CL_Rectf enlargedPadding = m_touchPaddingSave;
			enlargedPadding.translate((originalSize - scaledDownSize) / 2.0f);
			GetParent()->GetVar("touchPadding")->Set(enlargedPadding);
			break;
		}
	}
}

void Button2DComponent::OnOverMove(VariantList *pVList)
{
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (pTouch->WasHandled() && pTouch->GetEntityThatHandledIt() != GetParent())
	{
		m_pressed = false;

		buttonNoLongerPressed();
	}
}

void Button2DComponent::OnOverEnd(VariantList *pVList)
{
	if (pVList->Get(3).GetUINT32() == 0) {
		// The touch point moved outside the button so it's not pressed anymore
		m_pressed = false;
	}
	
	buttonNoLongerPressed();
}

void Button2DComponent::OnTouchStart(VariantList *pVList)
{
	TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
	if (pTouch->WasHandled()) return;
	m_pressed = true;
	
	if (*m_pButtonStyle == BUTTON_STYLE_CLICK_ON_TOUCH || *m_pButtonStyle == BUTTON_STYLE_CLICK_ON_TOUCH_IGNORE_DRAGGING)
	{
		pTouch->SetWasHandled(true, GetParent());
	
		switch (*m_pVisualStyle)
		{
		case STYLE_FADE_ALPHA_ON_HOVER:
			m_alphaSave = *m_pAlpha;
			break;

		case STYLE_SCALE_DOWN_ON_HOVER:
			m_scale2dSave = *m_pScale2d;
			m_touchPaddingSave = *m_pTouchPadding;
			break;
		}

		PerformClick(pVList);
	}
}

void Button2DComponent::PerformClick(VariantList *pVList)
{

	assert(pVList && "Need to send in coords so it knows where they clicked on the image");

	//why are we setting the parent here?  Well, I must have had a good reason.. someone must want this as output.
	//Don't change to be safe.  -Seth
	
	pVList->m_variant[1].Set(GetParent());

	switch (*m_pVisualStyle)
	{
		case STYLE_FADE_ALPHA_ON_HOVER:
			GetParent()->GetVar("alpha")->Set(m_alphaSave*0.5f);
			GetParent()->GetVar("alpha")->Set(m_alphaSave);
			break;

		case STYLE_SCALE_DOWN_ON_HOVER:
			GetParent()->GetVar("scale2d")->Set(m_scale2dSave);
			GetParent()->GetVar("touchPadding")->Set(m_touchPaddingSave);
			break;

		case STYLE_INVISIBLE_UNTIL_CLICKED:
			GetParent()->GetVar("alpha")->Set(m_alphaSave);
			//schedule the alpha to change back after a while
			GetMessageManager()->SetEntityVariable(GetParent(), *m_pRepeatDelayMS, "alpha", 0.0f);
			break;
	}

	if (*m_pDisabled == 0 &&  *m_pVisible != 0 && m_repeatTimer < GetBaseApp()->GetTick())
	{
		m_repeatTimer = GetBaseApp()->GetTick() + (*m_pRepeatDelayMS);
		if (!m_pOnClickAudioFile->empty())
		{
			GetAudioManager()->Play(*m_pOnClickAudioFile);
		}
		GetMessageManager()->CallEntityFunction(GetParent(), 1, "OnButtonSelected", pVList);  //sends a vec2 with position and this entity
		if (*m_pButtonStyle == BUTTON_STYLE_CLICK_ON_TOUCH && *m_pRepeatDelayMS != 0 )
		{
			SendFakeInputMessageToEntity(GetParent(), MESSAGE_TYPE_GUI_CLICK_END, pVList->m_variant[0].GetVector2()); //otherwise the menu may never get the touch release message
			return;
		}
		
	} else
	{
#ifdef _DEBUG
		LogMsg("Ignoring click to %s, button is disabled or not visible",GetParent()->GetName().c_str());
#endif
	}
}

void Button2DComponent::OnTouchEnd(VariantList *pVList)
{
	switch (*m_pButtonStyle) 
	{
		case BUTTON_STYLE_CLICK_ON_TOUCH_RELEASE:
		{
			TouchTrackInfo *pTouch = GetBaseApp()->GetTouch(pVList->Get(2).GetUINT32());
			if (!pTouch->WasHandled() || pTouch->GetEntityThatHandledIt() == NULL || pTouch->GetEntityThatHandledIt() == GetParent())
			{
				if (*m_pDisabled == 0)
					PerformClick(pVList);
			}
			break;
		}

		case BUTTON_STYLE_CLICK_ON_TOUCH_PRESS_RELEASE:
			if (m_pressed)
			{
				if (*m_pDisabled == 0)
					PerformClick(pVList);
			}
			break;

		default:
			break;
	}

	m_pressed = false;
}

void Button2DComponent::buttonNoLongerPressed()
{
	UpdateButtonVisuals(NULL);

	switch (*m_pVisualStyle)
	{
	case STYLE_FADE_ALPHA_ON_HOVER:
		GetParent()->GetVar("alpha")->Set(m_alphaSave);
		break;

	case STYLE_SCALE_DOWN_ON_HOVER:
		GetParent()->GetVar("scale2d")->Set(m_scale2dSave);
		GetParent()->GetVar("touchPadding")->Set(m_touchPaddingSave);
		break;
	}
}
