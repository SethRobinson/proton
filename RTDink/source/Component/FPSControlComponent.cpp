#include "PlatformPrecomp.h"
#include "FPSControlComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "../GUI/GameMenu.h"

#define C_GUI_FADE_OUT_TIMER 400
#define C_GUI_FADE_IN_TIMER 150


FPSControlComponent::FPSControlComponent()
{
	SetName("FPSControl");
}

FPSControlComponent::~FPSControlComponent()
{
}

void FPSControlComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pArrowEnt = NULL;
	m_pCenterBall = NULL;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_lastTouchDir = CL_Vec2f(0, 0); //the middle
	
	m_bTouchingArrows = false;
	

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnUpdate, this, _1));
	
	float guiTrans = GetApp()->GetVar("gui_transparency")->GetFloat();
	GetApp()->GetVar("gui_transparency")->Set(guiTrans);

	GetBaseApp()->m_sig_arcade_input.connect(1, boost::bind(&FPSControlComponent::OnArcadeInput, this, _1));	

	m_arrowMinTransparency = guiTrans;
	m_arrowMaxTransparency = rt_max(0.5f, guiTrans);

	//movement arrows?

	if (GetApp()->GetUsingTouchScreen())
	{

		m_pArrowEnt = CreateOverlayEntity(GetParent(), "arrow_gui", ReplaceWithDeviceNameInFileName("interface/iphone/arrows.rttex"), 0, 0);
	
		EntityComponent *pStripComp = m_pArrowEnt->AddComponent(new TouchStripComponent);
		m_pArrowEnt->GetVarWithDefault(string("touchPadding"), Variant(CL_Rectf(100, 100, 100, 100)))->GetRect();
		//m_pArrowEnt->AddComponent(new TouchHandlerComponent);
		
		//for speed, we've disabled movement message handling in App.cpp, but we actually want them just here, so we'll hardwire it
		AddInputMovementFocusIfNeeded(m_pArrowEnt);

		m_pArrowEnt->GetFunction("OnTouchStripUpdate")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnStripUpdate, this, _1));	
		m_pArrowEnt->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnOverStart, this, _1));	
		m_pArrowEnt->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnOverEnd, this, _1));	
		
		GetParent()->GetParent()->GetFunction("OnKillingControls")->sig_function.connect(1, boost::bind(&FPSControlComponent::OnKillingControls, this, _1));	
		SetAlphaEntity(m_pArrowEnt, 0);
		FadeEntity(m_pArrowEnt, true, m_arrowMinTransparency, 300, 0);
	
		//limit it to touches only on the left side of the screen

		//m_pCenterBall = CreateOverlayEntity(m_pArrowEnt, "center_ball", "interface/center_ball.rttex",0,0);
		
		if (m_pCenterBall)
		{
			m_pCenterBall->GetVar("color")->Set(MAKE_RGBA(255,255,255,0));
			//SetAlphaEntity(m_pCenterBall, 0);
			SetAlignmentEntity(m_pCenterBall, ALIGNMENT_CENTER);
		}

		m_timeOfLastTouch = GetTick(TIMER_SYSTEM);
		SetAlignmentEntity(m_pArrowEnt, ALIGNMENT_CENTER);
		
		CL_Vec2f vArrowPos = FlipXIfNeeded(iPhoneMap(CL_Vec2f(80, 240)));
		if (IsIPAD())
		{
			if (IsInFlingMode())
			{
				vArrowPos = CL_Vec2f( FlipXIfNeeded(149-30), C_FLING_JOYSTICK_Y);
			} else
			{
				vArrowPos = FlipXIfNeeded(CL_Vec2f(137-20, 651-80));
			}
		}
		
		m_pArrowEnt->GetVar("pos2d")->Set(vArrowPos);
		m_vArrowImageSizeOver2 = m_pArrowEnt->GetVar("size2d")->GetVector2()/2;
	
        VariantList vList;
		OnOverEnd(&vList);
	}
	
}

void FPSControlComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void SendKey(eDinkInput key, bool bIsDown)
{
	if (bIsDown)
	{
		g_dglo.m_dirInput[key] = true;
		g_dglo.m_dirInputFinished[key] = false; //make sure it wasn't scheduled to stop pressing
	} else
	{
		g_dglo.m_dirInputFinished[key] = true;
	}
}

void FPSControlComponent::ProcessLastJoystickReading()
{
	CL_Vec2f vDir = m_lastTouchDir;
	vDir.normalize();

	if (m_pCenterBall)
	m_pCenterBall->GetVar("pos2d")->Set(m_vArrowImageSizeOver2+vDir* (rt_min(m_lastTouchDir.length(), 1) * 58));
		
	float deadSpace = 0.25f/2;
	
	if (IsInFlingMode())
	{
		deadSpace *= 1.4f;
	} else
	{
		if (IsIPADSize) deadSpace *= 2;

	}

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

	const int maxDirections = 8;
	int finaldir = mod(dir+ (360/ (maxDirections*2)), 360)/ (360/maxDirections);

	//nah, let's do 8 actually

	//LogMsg("Pressing %s, which is dir %d (final: %d)", PrintVector2(m_lastTouchDir).c_str(), dir, finaldir);

	switch (finaldir)
	{
	case 0:  SendKey(DINK_INPUT_UP, true); break;
	case 1:  SendKey(DINK_INPUT_RIGHT, true); SendKey(DINK_INPUT_UP, true); break;

	case 2:  SendKey(DINK_INPUT_RIGHT, true); break;
	case 3:  SendKey(DINK_INPUT_RIGHT, true); SendKey(DINK_INPUT_DOWN, true);  break;
	case 4:  SendKey(DINK_INPUT_DOWN, true);  break;

	case 5:  SendKey(DINK_INPUT_DOWN, true);  SendKey(DINK_INPUT_LEFT, true);  break;
	case 6:  SendKey(DINK_INPUT_LEFT, true);  break;
	case 7:  SendKey(DINK_INPUT_LEFT, true);  SendKey(DINK_INPUT_UP, true);  break;
	}
}
void FPSControlComponent::OnKillingControls(VariantList *pVList)
{
	RemoveFocusIfNeeded(m_pArrowEnt);
	
}

void FPSControlComponent::OnArcadeInput(VariantList *pVList)
{
	int vKey = pVList->Get(0).GetUINT32();
	bool bIsDown = pVList->Get(1).GetUINT32() != 0;

	//LogMsg("Key %d, down is %d", vKey, int(bIsDown));
	switch (vKey)
	{
	case VIRTUAL_KEY_DIR_LEFT:
		SendKey(DINK_INPUT_LEFT, bIsDown);
		break;

	case VIRTUAL_KEY_DIR_RIGHT:
		SendKey(DINK_INPUT_RIGHT, bIsDown);
		break;

	case VIRTUAL_KEY_DIR_UP:
		SendKey(DINK_INPUT_UP, bIsDown);
		break;

	case VIRTUAL_KEY_DIR_DOWN:
		SendKey(DINK_INPUT_DOWN, bIsDown);
		break;
	}
}


void FPSControlComponent::OnOverStart(VariantList *pVList)
{
}

void FPSControlComponent::OnOverEnd(VariantList *pVList)
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

void FPSControlComponent::ClearKeyInput()
{
	SendKey(DINK_INPUT_UP, false);
	SendKey(DINK_INPUT_DOWN, false);
	SendKey(DINK_INPUT_LEFT, false);
	SendKey(DINK_INPUT_RIGHT, false);
}

void FPSControlComponent::ProcessArrowInput(CL_Vec2f vDir)
{
	if (m_bTouchingArrows || 1)
	{
		m_lastTouchDir = vDir*2 - CL_Vec2f(1, 1);

		//update current position
		ClearKeyInput();
		ProcessLastJoystickReading();
	} else
	{
		//	m_lastTouchDir = CL_Vec2f(0,0);
		if (m_pCenterBall)
			m_pCenterBall->GetVar("pos2d")->Set(m_lastTouchDir);
	}
}

void FPSControlComponent::OnStripUpdate(VariantList *pVList)
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

void FPSControlComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void FPSControlComponent::OnUpdate(VariantList *pVList)
{
}