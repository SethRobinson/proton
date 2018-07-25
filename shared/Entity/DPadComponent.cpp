#include "PlatformPrecomp.h"
#include "DPadComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

#define C_GUI_FADE_OUT_TIMER 400
#define C_GUI_FADE_IN_TIMER 150



void DPADButton::OnButtonChange(int key, bool bDown)
{
	if (bDown)
	{
		//push
		
		if (!m_bDown)
		{
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR, (float)key , (float)bDown);  
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)key , (float)bDown);  
//			LogMsg("DOWN: Got %d down is %d", key, int(bDown));
			m_bDown = true;
		}
	} else
	{
		//release
		if (m_bDown)
		{
			m_bDown = false;
			GetMessageManager()->SendGUI(MESSAGE_TYPE_GUI_CHAR_RAW, (float)key , (float)bDown);  
//			LogMsg("RELEASE: Got %d down is %d", key, int(bDown));
		}
	}
}


DPadComponent::DPadComponent()
{
	SetName("FPSControl");
}

DPadComponent::~DPadComponent()
{
}

void DPadComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pArrowEnt = NULL;
	m_pCenterBall = NULL;
	m_ptouchAreaPadding = &GetVarWithDefault("touch_area_padding", 40.0f)->GetFloat();

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pScale2d = &GetParent()->GetVarWithDefault("scale2d", CL_Vec2f(1,1))->GetVector2();
	m_lastTouchDir = CL_Vec2f(0, 0); //the middle
	m_bTouchingArrows = false;
	
	m_pDpadImage = &GetParent()->GetVarWithDefault("dpad_image", string("interface/dpad.rttex"))->GetString();

	//get a callback if anybody changes our scale setting
	GetParent()->GetVar("scale2d")->GetSigOnChanged()->connect(boost::bind(&DPadComponent::OnScaleChanged, this, _1));

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&DPadComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&DPadComponent::OnUpdate, this, _1));

	float guiTrans = 0.3f;

	m_arrowMinTransparency = guiTrans;
	m_arrowMaxTransparency = rt_max(0.5f, guiTrans);

	//movement arrows?

	if (!GetFileManager()->FileExists(*m_pDpadImage))
	{
		LogMsg("Missing file %s.  Set dpad_image to desired filename in DPadComponent before adding!", m_pDpadImage->c_str());
		assert(!"Can't load the dpad file!");
	}
	m_pArrowEnt = CreateOverlayEntity(GetParent(), "arrow_gui", *m_pDpadImage, 0, 0);

	EntityComponent *pStripComp = m_pArrowEnt->AddComponent(new TouchStripComponent);
	

	m_pArrowEnt->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(*m_ptouchAreaPadding, *m_ptouchAreaPadding, *m_ptouchAreaPadding, *m_ptouchAreaPadding)))->GetRect();
	//m_pArrowEnt->AddComponent(new TouchHandlerComponent);

	//for speed, we've disabled movement message handling in App.cpp, but we actually want them just here, so we'll hardwire it
	AddInputMovementFocusIfNeeded(m_pArrowEnt);

	m_pArrowEnt->GetFunction("OnTouchStripUpdate")->sig_function.connect(1, boost::bind(&DPadComponent::OnStripUpdate, this, _1));	
	m_pArrowEnt->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&DPadComponent::OnOverStart, this, _1));	
	m_pArrowEnt->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&DPadComponent::OnOverEnd, this, _1));	

	SetAlphaEntity(m_pArrowEnt, 0);
	FadeEntity(m_pArrowEnt, true, m_arrowMinTransparency, 300, 0);

	if (m_pCenterBall)
	{
		m_pCenterBall->GetVar("color")->Set(MAKE_RGBA(255,255,255,0));
		//SetAlphaEntity(m_pCenterBall, 0);
		SetAlignmentEntity(m_pCenterBall, ALIGNMENT_CENTER);
	}

	//we also need to add a filter so we ignore touches too far away from this control, otherwise hitting buttons and stuff will 
	//make it think it stopped touching the dpad

	SetupClipRectOnDPad();
	
	m_timeOfLastTouch = GetTick(TIMER_SYSTEM);
	SetAlignmentEntity(m_pArrowEnt, ALIGNMENT_CENTER);

	m_vArrowImageSizeOver2 = m_pArrowEnt->GetVar("size2d")->GetVector2()/2;
	
    VariantList vList;
    OnOverEnd(&vList);
}

void DPadComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void DPadComponent::OnScaleChanged(Variant *pVariant)
{
	
	assert(m_pArrowEnt);
	//scale up the arrow
	m_pArrowEnt->GetVar("scale2d")->Set(*pVariant);

	//rebuild the custom cliprect we had made earlier, to fit the new size
	SetupClipRectOnDPad();
	
}

void DPadComponent::SetupClipRectOnDPad()
{

	//no longer needed

	/*
	//if there was an old one, remove it
	m_pArrowEnt->RemoveComponentByName("FilterInput");
	EntityComponent *pFilter = m_pArrowEnt->AddComponent(new FilterInputComponent);
	pFilter->GetVar("mode")->Set(uint32(FilterInputComponent::MODE_CLIP_INPUT_TO_CLIP_RECT));
	CL_Vec2f vImageSize = m_pArrowEnt->GetVar("size2d")->GetVector2();
	CL_Rectf clipRect = CL_Rectf(0,0, vImageSize.x, vImageSize.y);
	
	//note: see the 50 below that I harcoded?  might want to change that to a changable var later
	clipRect.expand(*m_ptouchAreaPadding+50); //clip our input to a little outside of our image area (plus padding), so we don't get confused by
	//other buttons far away, yet can still register "they slipped off the button"
	pFilter->GetVar("clipRect")->Set(clipRect);
*/
}
void DPadComponent::SendKey(eVirtualKeys key, bool bIsDown)
{
		switch (key)
		{
		case VIRTUAL_KEY_DIR_LEFT:
			m_button[BUTTON_LEFT].OnButtonChange(key, bIsDown);
			break;
		case VIRTUAL_KEY_DIR_RIGHT:
			m_button[BUTTON_RIGHT].OnButtonChange(key, bIsDown);
			break;
		case VIRTUAL_KEY_DIR_UP:
			m_button[BUTTON_UP].OnButtonChange(key, bIsDown);
			break;
		case VIRTUAL_KEY_DIR_DOWN:
			m_button[BUTTON_DOWN].OnButtonChange(key, bIsDown);
			break;
		}
}

void DPadComponent::ProcessLastJoystickReading()
{
	CL_Vec2f vDir = m_lastTouchDir;
	vDir.normalize();

	if (m_pCenterBall)
		m_pCenterBall->GetVar("pos2d")->Set(m_vArrowImageSizeOver2+vDir* (rt_min(m_lastTouchDir.length(), 1) * 58));

	const float deadSpace = 0.25f/2;

	if (m_lastTouchDir.length() < deadSpace)
	{
		//dead space
		if (m_pCenterBall)
		{
			m_pCenterBall->GetVar("pos2d")->Set(m_vArrowImageSizeOver2);
		}
		return;
	}

	//convert to 360 degrees
	int dir = (int(RAD2DEG(atan2(m_lastTouchDir.y, m_lastTouchDir.x))+(90)));

	//convert to 8 directions
	const int maxDirections = 8;
	int finaldir = mod(dir+ (360/ (maxDirections*2)), 360)/ (360/maxDirections);

	//LogMsg("Pressing %s, which is dir %d (final: %d)", PrintVector2(m_lastTouchDir).c_str(), dir, finaldir);

	eVirtualKeys vKey1 = VIRTUAL_KEY_NONE;
	eVirtualKeys vKey2 = VIRTUAL_KEY_NONE;

	switch (finaldir)
	{
	case 0:  vKey1 = VIRTUAL_KEY_DIR_UP; break;
	case 1:  vKey1 = VIRTUAL_KEY_DIR_RIGHT; vKey2 = VIRTUAL_KEY_DIR_UP; break;

	case 2:  vKey1 = VIRTUAL_KEY_DIR_RIGHT; break;
	case 3:  vKey1 = VIRTUAL_KEY_DIR_RIGHT; vKey2 = VIRTUAL_KEY_DIR_DOWN;  break;
	case 4:  vKey1 = VIRTUAL_KEY_DIR_DOWN;  break;

	case 5:  vKey1 = VIRTUAL_KEY_DIR_DOWN;  vKey2 = VIRTUAL_KEY_DIR_LEFT;  break;
	case 6:  vKey1 = VIRTUAL_KEY_DIR_LEFT;  break;
	case 7:  vKey1 = VIRTUAL_KEY_DIR_LEFT;  vKey2 = VIRTUAL_KEY_DIR_UP;  break;
	}

	//kill any keys they didn't just hit.. yeah, this could be done more eleganty.  That's what I get for refactoring a file from
	//another project...
	if (vKey1 != VIRTUAL_KEY_DIR_UP && vKey2 != VIRTUAL_KEY_DIR_UP)
	SendKey(VIRTUAL_KEY_DIR_UP, false);
	if (vKey1 != VIRTUAL_KEY_DIR_DOWN && vKey2 != VIRTUAL_KEY_DIR_DOWN)
		SendKey(VIRTUAL_KEY_DIR_DOWN, false);
	if (vKey1 != VIRTUAL_KEY_DIR_LEFT && vKey2 != VIRTUAL_KEY_DIR_LEFT)
		SendKey(VIRTUAL_KEY_DIR_LEFT, false);
	if (vKey1 != VIRTUAL_KEY_DIR_RIGHT && vKey2 != VIRTUAL_KEY_DIR_RIGHT)
		SendKey(VIRTUAL_KEY_DIR_RIGHT, false);

	//actually perform the key sends

	if (vKey1 != VIRTUAL_KEY_NONE)
	{
		SendKey(vKey1, true);
	}
	
	if (vKey1 != VIRTUAL_KEY_NONE)
	{
		SendKey(vKey2, true);
	}

}


void DPadComponent::OnOverStart(VariantList *pVList)
{
}

void DPadComponent::OnOverEnd(VariantList *pVList)
{
	m_bTouchingArrows = false;
	ClearKeyInput();
	if (m_pCenterBall)
		m_pCenterBall->GetVar("color")->Set(MAKE_RGBA(255,255,255,0));

	//m_pArrowEnt->GetVar("alpha")->Set(1.0f);
	FadeEntity(m_pArrowEnt, false, m_arrowMinTransparency, C_GUI_FADE_OUT_TIMER);
	if (m_pCenterBall)
		FadeEntity(m_pCenterBall, false, 0.0, C_GUI_FADE_OUT_TIMER/3);
}

void DPadComponent::ClearKeyInput()
{
	SendKey(VIRTUAL_KEY_DIR_UP, false);
	SendKey(VIRTUAL_KEY_DIR_DOWN, false);
	SendKey(VIRTUAL_KEY_DIR_LEFT, false);
	SendKey(VIRTUAL_KEY_DIR_RIGHT, false);
}

void DPadComponent::ProcessArrowInput(CL_Vec2f vDir)
{
	if (m_bTouchingArrows)
	{
		m_lastTouchDir = vDir*2 - CL_Vec2f(1, 1);

		//update current position
		//ClearKeyInput();
		ProcessLastJoystickReading();
	} else
	{
		//	m_lastTouchDir = CL_Vec2f(0,0);
		if (m_pCenterBall)
			m_pCenterBall->GetVar("pos2d")->Set(m_lastTouchDir);
	}
}

void DPadComponent::OnStripUpdate(VariantList *pVList)
{
	if (!m_bTouchingArrows)
	{
		m_bTouchingArrows = true;
		//m_pArrowEnt->GetVar("pos2d")->Set( ConvertEntityClickToScreenCoords(pVList->Get(0).GetVector2(), pVList->Get(1).GetEntity()));
		//ProcessArrowInput(pVList->Get(1).GetVector2());
		if (m_pCenterBall)
			m_pCenterBall->GetVar("color")->Set(MAKE_RGBA(255,255,255,255));

		FadeEntity(m_pArrowEnt, false, m_arrowMaxTransparency, C_GUI_FADE_IN_TIMER);
		if (m_pCenterBall)
			FadeEntity(m_pCenterBall, false, m_arrowMaxTransparency, C_GUI_FADE_OUT_TIMER/3);

	}
	ProcessArrowInput(pVList->Get(1).GetVector2());
}

void DPadComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void DPadComponent::OnUpdate(VariantList *pVList)
{
}