#include "PlatformPrecomp.h"
#include "DragControlComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "Entity/ArcadeInputComponent.h"
#include "../GUI/GameMenu.h"


DragControlComponent::DragControlComponent()
{
	SetName("DragControl");


SAMPLE_COUNT = 8; //unused

LENGTH_REQUIRED_FOR_MOVE = 16;

if (IsIphoneSize || IsIphone4Size) LENGTH_REQUIRED_FOR_MOVE = 32; //the screen is tiny, need a few more pixels

	if (IsIphone4Size)
	{
		LENGTH_REQUIRED_FOR_MOVE *= 2;
	}


}

DragControlComponent::~DragControlComponent()
{
}

void DragControlComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	if (IsIPAD())
	{
		*m_pPos2d =(CL_Vec2f(0,29));
	} else
	{
		*m_pPos2d = (iPhoneMap(0,29));
	}
	
	
	
	//fit the active area to match the active dink screen layout
	if (g_dglo.GetActiveView() != DinkGlobals::VIEW_ZOOMED && IsDrawingDinkStatusBar())
	{
		if (IsIPAD())
		{
			*m_pSize2d = CL_Vec2f(865, 584);
		} else
		{
			*m_pSize2d = iPhoneMap(380, 237);
		}

	} else
	{
		if (IsIPAD())
		{
			*m_pSize2d = CL_Vec2f(866, 715);

		} else
		{
			*m_pSize2d = iPhoneMap(380, 286);
		}
	}
	
	if (GetApp()->GetIconsOnLeft())
	{
		m_pPos2d->x += GetScreenSizeXf()-m_pSize2d->x;
	}
	
	m_lastTouch = CL_Vec2f(0,0);
	
	/*
	m_pScale = &GetParent()->GetShared()->GetVarWithDefault("scale", Variant(1.0f))->GetFloat();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	//register ourselves to render if the parent does
	//GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&DragControlComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&DragControlComponent::OnUpdate, this, _1));
	GetBaseApp()->m_sig_arcade_input.connect(1, boost::bind(&DragControlComponent::OnArcadeInput, this, _1));	

	GetParent()->GetVar("ignoreTouchesOutsideRect")->Set(uint32(1));
	EntityComponent *pTouch = GetParent()->AddComponent(new TouchHandlerComponent);
	
	//movement arrows

	//for speed, we've disabled movement message handling in App.cpp, but we actually want them just here, so we'll hardwire it
	AddInputMovementFocusIfNeeded(GetParent());

	GetParent()->GetFunction("OnOverStart")->sig_function.connect(1, boost::bind(&DragControlComponent::OnOverStart, this, _1));	
	GetParent()->GetFunction("OnOverEnd")->sig_function.connect(1, boost::bind(&DragControlComponent::OnOverEnd, this, _1));	
	GetParent()->GetFunction("OnOverMove")->sig_function.connect(1, boost::bind(&DragControlComponent::OnOverMove, this, _1));

	GetParent()->GetParent()->GetFunction("OnKillingControls")->sig_function.connect(1, boost::bind(&DragControlComponent::OnKillingControls, this, _1));	

	/*

	EntityComponent *pFilter = m_pArrowEnt->AddComponent(new FilterInputComponent);
	pFilter->GetVar("mode")->Set(uint32(FilterInputComponent::MODE_CLIP_INPUT_TO_ABSOLUTE_CLIP_RECT_AND_DISABLE_INPUT_CHILDREN));
	pFilter->GetVar("clipRect")->Set(CL_Rectf(0,43,397, GetScreenSizeY()));

	m_timeOfLastTouch = GetTick(TIMER_SYSTEM);
	SetAlignmentEntity(m_pArrowEnt, ALIGNMENT_CENTER);
	m_pArrowEnt->GetVar("pos2d")->Set(CL_Vec2f(80, 240));

	m_vArrowImageSizeOver2 = m_pArrowEnt->GetVar("size2d")->GetVector2()/2;
	*/
	VariantList vTemp;
	OnOverEnd(&vTemp);
}

void DragControlComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void DragControlComponent::SendKey(eDinkInput key, bool bIsDown)
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

void DragControlComponent::OnKillingControls(VariantList *pVList)
{
	RemoveFocusIfNeeded(GetParent());
}

void DragControlComponent::AddSample(DragUnit v)
{
	if (DinkIsDoingScreenTransition()) return;
	m_samples.push_back(v);
	
	/*
	if (m_samples.size() > SAMPLE_COUNT)
	{
	  m_samples.pop_front();
	}
	*/
	
}

void DragControlComponent::OnOverStart(VariantList *pVList)
{
	m_samples.clear();
	m_lastTouch = pVList->Get(0).GetVector2();
}

void DragControlComponent::OnOverEnd(VariantList *pVList)
{
//	CL_Vec2f vPos = pVList->Get(0).GetVector2();
//	LogMsg("Got over end: %s", PrintVector2(vPos).c_str());
	m_samples.clear();
	m_lastTouch = CL_Vec2f(0,0);
	ClearKeyInput();
}

void DragControlComponent::OnOverMove(VariantList *pVList)
{
	AddSample( DragUnit(pVList->Get(0).GetVector2() - m_lastTouch, GetTick(TIMER_GAME)));
	//SetPosition(pVList->Get(0).GetVector2());
	m_lastTouch = pVList->Get(0).GetVector2();
}

void DragControlComponent::ClearKeyInput()
{
	SendKey(DINK_INPUT_UP, false);
	SendKey(DINK_INPUT_DOWN, false);
	SendKey(DINK_INPUT_LEFT, false);
	SendKey(DINK_INPUT_RIGHT, false);
}

void DragControlComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void DragControlComponent::ProcessLastJoystickReading()
{

	if (m_samples.size() == 0) return;
	//first, lets figure out what direction we want
	CL_Vec2f vDir = CL_Vec2f(0,0);
	
	std::deque< DragUnit> sampleTemp;

	for (int i=m_samples.size()-1; i >= 0;)
	{
		//if (m_samples.size() < SAMPLE_COUNT) break;

		/*
		if (1)
		//if (m_samples.at(i).m_timeMade+100 < GetTick(TIMER_GAME))
			{
			vDir = CL_Vec2f(0,0);
	//would removing this stop us from moving?
			for (int h=1; h < m_samples.size();h++)
			{
				vDir += m_samples[h].m_vPos;
			}
			
			if (vDir.length() < LENGTH_REQUIRED_FOR_MOVE)
			{
				//yes it would.  Just let it be, for now.
				//break;
			} else
			{
				//it's not needed
			}

			m_samples.erase(m_samples.begin()+i);
			continue;
		} else
		{
			break;
		}

		*/
		vDir += m_samples[i].m_vPos;
		sampleTemp.push_front( m_samples[i]);
		
		if (vDir.length() >=  LENGTH_REQUIRED_FOR_MOVE)
		{
			//all done
			break;
		}
		
		i--;
	}
	
	m_samples = sampleTemp;

	//vDir = CL_Vec2f(0,0);

	/*
	for (unsigned int i=0; i < m_samples.size();i++)
	{
		vDir += m_samples[i].m_vPos;
	}
	*/
	ClearKeyInput();

	//LogMsg("vDir length is %.2f", vDir.length());
	if (vDir.length() < LENGTH_REQUIRED_FOR_MOVE) return;

	vDir.normalize();

	//recalculate the points
	sampleTemp.clear();

	//sampleTemp.push_front(m_samples[m_samples.size()-1]);

	sampleTemp.push_front(DragUnit( (vDir*LENGTH_REQUIRED_FOR_MOVE)*1.1, 0));
	m_samples = sampleTemp;

#ifdef _DEBUG
	//LogMsg("Dir is %s (%d samples)", PrintVector2(vDir).c_str(), m_samples.size());
#endif
	//convert to 360 degrees
	int dir = (int(RAD2DEG(atan2(vDir.y, vDir.x))+(90)));

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



void DragControlComponent::OnArcadeInput(VariantList *pVList)
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


void DragControlComponent::OnUpdate(VariantList *pVList)
{
	ProcessLastJoystickReading();
}