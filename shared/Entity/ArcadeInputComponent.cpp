#include "PlatformPrecomp.h"
#include "ArcadeInputComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

void MoveButtonState::OnPress(int timeToAddMS, boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange)
{
	VariantList v;

	if (!m_bIsDown)
	{
		m_bIsDown = true;
		v.Get(0).Set(uint32(m_keyType));
		v.Get(1).Set(uint32(VIRTUAL_KEY_PRESS)); 
		
        if (bSendChange)
        {
            if (pCustomSignal)
            {
                (*pCustomSignal)(&v);
            } else
            {
                GetBaseApp()->m_sig_arcade_input(&v);
            }
        }

	}
	
	m_releaseTimer = rt_max(GetTick(TIMER_SYSTEM), m_releaseTimer);
	m_releaseTimer += timeToAddMS;
}

void MoveButtonState::OnPressToggle(bool bDown, boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange)
{
	VariantList v;

	if (bDown)
	{

	if (!m_bIsDown)
	{
		m_bIsDown = true;

		v.Get(0).Set(uint32(m_keyType));
		v.Get(1).Set(uint32(VIRTUAL_KEY_PRESS)); 
		
        if (bSendChange)
        {
            if (pCustomSignal)
            {
                (*pCustomSignal)(&v);
            } else
            {
                GetBaseApp()->m_sig_arcade_input(&v);
            }
        }
	}

	m_releaseTimer = rt_max(GetTick(TIMER_SYSTEM), m_releaseTimer);
	m_releaseTimer += 1000*60;
	} else
	{
		ReleaseIfNeeded(pCustomSignal, bSendChange);
	}
}
void MoveButtonState::Update(boost::signal<void (VariantList*)> *pCustomSignal)
{
	if (m_bIsDown && m_releaseTimer < GetTick(TIMER_SYSTEM))
	{
		//time to release, since Android doesn't send one
		ReleaseIfNeeded(pCustomSignal);
	}
}

void MoveButtonState::ReleaseIfNeeded(boost::signal<void (VariantList*)> *pCustomSignal, bool bSendChange)
{
	if (m_bIsDown)
	{
		m_bIsDown = false;
		VariantList v;

		v.Get(0).Set(uint32(m_keyType));
		v.Get(1).Set(uint32(VIRTUAL_KEY_RELEASE)); 
	
        if (bSendChange)
        {
            if (pCustomSignal)
            {
                (*pCustomSignal)(&v);
            } else
            {
                GetBaseApp()->m_sig_arcade_input(&v);
            }
        }
    }
}

ArcadeInputComponent::ArcadeInputComponent()
{
	SetName("ArcadeInput");
	m_customSignal = NULL;
	m_buttons[MOVE_BUTTON_DIR_LEFT].SetKeyType(VIRTUAL_KEY_DIR_LEFT);
	m_buttons[MOVE_BUTTON_DIR_RIGHT].SetKeyType(VIRTUAL_KEY_DIR_RIGHT);
	m_buttons[MOVE_BUTTON_DIR_UP].SetKeyType(VIRTUAL_KEY_DIR_UP);
	m_buttons[MOVE_BUTTON_DIR_DOWN].SetKeyType(VIRTUAL_KEY_DIR_DOWN);
}

ArcadeInputComponent::~ArcadeInputComponent()
{
}

void ArcadeInputComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	
	//register ourselves to render if the parent does
	//GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&MovementInputComponent::OnRender, this, _1));
	GetBaseApp()->m_sig_update.connect(1, boost::bind(&ArcadeInputComponent::OnUpdate, this, _1));
	GetBaseApp()->m_sig_trackball.connect(1, boost::bind(&ArcadeInputComponent::OnTrackball, this, _1));
	GetBaseApp()->m_sig_raw_keyboard.connect(1, boost::bind(&ArcadeInputComponent::OnRawKeyboard, this, _1));

	GetFunction("SetOutputEntity")->sig_function.connect(1, boost::bind(&ArcadeInputComponent::SetOutput, this, _1));
	GetFunction("RemoveKeyBindingsStartingWith")->sig_function.connect(1, boost::bind(&ArcadeInputComponent::RemoveKeyBindingsStartingWith, this, _1));

	GetFunction("AddKeyBinding")->sig_function.connect(1, boost::bind(&ArcadeInputComponent::AddKeyBinding, this, _1));
	m_pTrackballMode = &GetVarWithDefault("trackball_mode", uint32(TRACKBALL_MODE_WALKING))->GetUINT32();

	//get notified when this changes:
	GetVar("trackball_mode")->GetSigOnChanged()->connect(1, boost::bind(&ArcadeInputComponent::OnTrackballModeChanged, this, _1));
	//example of adding a keybinding:
	//pComp->GetFunction("AddKeyBinding")->sig_function(&VariantList(string("Left"), uint32(VIRTUAL_KEY_DIR_LEFT), uint32(VIRTUAL_KEY_DIR_LEFT)));
}

void ArcadeInputComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ArcadeInputComponent::OnTrackballModeChanged(Variant *pVar)
{
	if (pVar->GetUINT32() == TRACKBALL_MODE_MENU_SELECTION)
	{
		m_trackball = CL_Vec2f(0,0); //clear whatever was there before
	}
}
void ArcadeInputComponent::SetOutput(VariantList *pVList)
{
	Entity *pEnt = pVList->Get(0).GetEntity();
	assert(!m_customSignal && "We don't support setting this twice yet");
	
	m_customSignal = &pEnt->GetFunction("OnArcadeInput")->sig_function;

	//get notified when this entity dies
	pEnt->sig_onRemoved.connect(1, boost::bind(&ArcadeInputComponent::OnCustomOutputRemoved, this, _1));
}

void ArcadeInputComponent::OnCustomOutputRemoved(Entity *pEnt)
{
	m_customSignal = NULL;
}

void ArcadeInputComponent::AddKeyBinding(VariantList *pVList)
{
	ArcadeKeyBind b;
	b.m_name = pVList->Get(0).GetString();
	b.m_inputkeycode = pVList->Get(1).GetUINT32();
	b.m_outputkeycode = pVList->Get(2).GetUINT32();
	b.m_bOutputAsNormalKeyToo = pVList->Get(3).GetUINT32() != 0;
	
	if (pVList->Get(4).GetType() == Variant::TYPE_UINT32)
	{
		//they sent extra info, the binding may require certain modifiers like ctrl/shift/alt
		b.m_keyModifiersRequired = pVList->Get(4).GetUINT32();
	}
	
	m_bindings.push_back(b);
}


#define C_ANGLE_VEC (sqrt(2.0f)/2.0f)

//returns false if no direction is active (if no keys are down)
bool ConvertKeysToDirectionVector(bool bLeft, bool bRight, bool bUp, bool bDown, CL_Vec2f * pVecOut)
{

	if (!bLeft && !bRight && !bUp && !bDown)
	{
		*pVecOut = CL_Vec2f(0,0);
		return false;
	}
	
	if (bLeft) *pVecOut = CL_Vec2f(-1,0); else
	if (bRight) *pVecOut = CL_Vec2f(1,0); else
	if (bUp) *pVecOut = CL_Vec2f(0,-1); else
	*pVecOut = CL_Vec2f(0, 1); //must have hit down

	//check diagonals

	if (bUp && bLeft) *pVecOut = CL_Vec2f(-C_ANGLE_VEC,-C_ANGLE_VEC); else
	if (bDown && bLeft) *pVecOut = CL_Vec2f(-C_ANGLE_VEC,C_ANGLE_VEC); else
	if (bUp && bRight) *pVecOut = CL_Vec2f(C_ANGLE_VEC,-C_ANGLE_VEC); else
	if (bDown && bRight) *pVecOut = CL_Vec2f(C_ANGLE_VEC,C_ANGLE_VEC);

	return true;
}

void ArcadeInputComponent::ActivateBinding(ArcadeKeyBind *pBind, bool bDown)
{
	//special handling for directional keys, so they work in tandem with the trackball or whatever else also does directions

	bool bWasDirectionalKey = true;

	switch (pBind->m_outputkeycode)
	{
		case VIRTUAL_KEY_DIR_LEFT:  m_buttons[MOVE_BUTTON_DIR_LEFT].OnPressToggle(bDown, m_customSignal);  break;
		case VIRTUAL_KEY_DIR_RIGHT:  m_buttons[MOVE_BUTTON_DIR_RIGHT].OnPressToggle(bDown, m_customSignal);  break;
		case VIRTUAL_KEY_DIR_UP:  m_buttons[MOVE_BUTTON_DIR_UP].OnPressToggle(bDown, m_customSignal);  break;
		case VIRTUAL_KEY_DIR_DOWN:  m_buttons[MOVE_BUTTON_DIR_DOWN].OnPressToggle(bDown, m_customSignal); break;
			
		default:

			bWasDirectionalKey = false;
			break;
	}

	if (bWasDirectionalKey) 
	{
		//avoid sending it twice, we already would above
		return;
	}

	//for other keys, just send it through the arcade signal:
	VariantList v;
	v.Get(0).Set(pBind->m_outputkeycode);
	v.Get(1).Set(uint32(bDown)); 

	if (m_customSignal)
	{
		(*m_customSignal)(&v);
	} else
	{
		GetBaseApp()->m_sig_arcade_input(&v);
	}

	if (bDown)
	{
		//send as normal key, only on the keydown
		VariantList v2;
		v2.Get(0).Set((float)MESSAGE_TYPE_GUI_CHAR);
		v2.Get(1).Set(0,0);
		v2.Get(2).Set(uint32(pBind->m_outputkeycode));
		GetBaseApp()->m_sig_input(&v2);
	}
	if (pBind->m_bOutputAsNormalKeyToo)
	{
		//also send as regular global key
		GetBaseApp()->m_sig_raw_keyboard(&v);
	}
}

void ArcadeInputComponent::OnRawKeyboard(VariantList *pVList)
{
	int keyCode = pVList->Get(0).GetUINT32();
	bool bDown = pVList->Get(1).GetUINT32() == 1;

	int modifiers = pVList->Get(3).GetUINT32();

#ifdef _DEBUG
	
	/*
	string mods;

	if (modifiers & VIRTUAL_KEY_MODIFIER_CONTROL)
	{
		mods += "Control ";
	}
	if (modifiers & VIRTUAL_KEY_MODIFIER_SHIFT)
	{
		mods += "Shift ";
	}
	if (modifiers & VIRTUAL_KEY_MODIFIER_ALT)
	{
		mods += "Alt ";
	}

	LogMsg("ArcadeInputComponent> Got key %d, %d (%s)", keyCode, int(bDown), mods.c_str());
	*/

#endif

	ArcadeBindList::iterator itor = m_bindings.begin();

	for (;itor != m_bindings.end(); itor++)
	{
		if (keyCode == itor->m_inputkeycode && (itor->m_keyModifiersRequired == 0|| modifiers == itor->m_keyModifiersRequired))
		{
			ActivateBinding( &(*itor), bDown );
		}
	}
}

void ArcadeInputComponent::OnTrackball(VariantList *pVList)
{

	float x = pVList->Get(1).GetVector3().x;
	float y = pVList->Get(1).GetVector3().y;
	
	if (*m_pTrackballMode == TRACKBALL_MODE_MENU_SELECTION)
	{
		m_trackball.x += x;
		m_trackball.y += y;

		if (m_trackball.x < -1.0f) 
		{
			m_buttons[MOVE_BUTTON_DIR_LEFT].OnPress(0, m_customSignal);
			m_buttons[MOVE_BUTTON_DIR_LEFT].ReleaseIfNeeded(m_customSignal);
			m_trackball.x += 1;
			m_trackball.y = 0;

		} else
		{
			if (m_trackball.x > 1.0f) 
			{
				m_buttons[MOVE_BUTTON_DIR_RIGHT].OnPress(0, m_customSignal);
				m_buttons[MOVE_BUTTON_DIR_RIGHT].ReleaseIfNeeded(m_customSignal);
				m_trackball.x -= 1;
				m_trackball.y = 0;
			}
		}

		if (m_trackball.y < -1.0f)
		{
			m_buttons[MOVE_BUTTON_DIR_UP].OnPress(0, m_customSignal);
			m_buttons[MOVE_BUTTON_DIR_UP].ReleaseIfNeeded(m_customSignal);
			m_trackball.y += 1;
			m_trackball.x = 0;
		}else
		{
			if (m_trackball.y > 1.0f)
			{
				m_buttons[MOVE_BUTTON_DIR_DOWN].OnPress(0, m_customSignal);
				m_buttons[MOVE_BUTTON_DIR_DOWN].ReleaseIfNeeded(m_customSignal);
				m_trackball.y -= 1;
				m_trackball.x = 0;
			}
		}

		return;
	}


	int releaseTime = 50;

	if (x < 0) 
	{
		m_buttons[MOVE_BUTTON_DIR_LEFT].OnPress(releaseTime, m_customSignal);
		m_buttons[MOVE_BUTTON_DIR_RIGHT].ReleaseIfNeeded(m_customSignal);
	} else
	{
		if (x > 0) 
		{
			m_buttons[MOVE_BUTTON_DIR_RIGHT].OnPress(releaseTime, m_customSignal);
			m_buttons[MOVE_BUTTON_DIR_LEFT].ReleaseIfNeeded(m_customSignal);
		}
	}
	
	if (y < 0)
	{
		m_buttons[MOVE_BUTTON_DIR_UP].OnPress(releaseTime, m_customSignal);
		m_buttons[MOVE_BUTTON_DIR_DOWN].ReleaseIfNeeded(m_customSignal);
	}else
	{
		if (y > 0)
		{
			m_buttons[MOVE_BUTTON_DIR_DOWN].OnPress(releaseTime, m_customSignal);
			m_buttons[MOVE_BUTTON_DIR_UP].ReleaseIfNeeded(m_customSignal);
		}
	}
}

void ArcadeInputComponent::OnUpdate(VariantList *pVList)
{
	for (int i=0; i < MOVE_BUTTON_DIR_COUNT; i++)
	{
		m_buttons[i].Update(m_customSignal);
	}
}

bool ArcadeInputComponent::GetDirectionKeysAsVector( CL_Vec2f *pVecOut )
{
	return ConvertKeysToDirectionVector(m_buttons[MOVE_BUTTON_DIR_LEFT].m_bIsDown,
		m_buttons[MOVE_BUTTON_DIR_RIGHT].m_bIsDown, m_buttons[MOVE_BUTTON_DIR_UP].m_bIsDown,
		m_buttons[MOVE_BUTTON_DIR_DOWN].m_bIsDown, pVecOut);
}

bool ArcadeInputComponent::GetDirectionKeys( bool &bLeftOut, bool &bRightOut, bool &bUpOut, bool &bDownOut )
{
	bLeftOut = m_buttons[MOVE_BUTTON_DIR_LEFT].m_bIsDown;
	bRightOut = m_buttons[MOVE_BUTTON_DIR_RIGHT].m_bIsDown;
	bUpOut = m_buttons[MOVE_BUTTON_DIR_UP].m_bIsDown;
	bDownOut = m_buttons[MOVE_BUTTON_DIR_DOWN].m_bIsDown;

	return (bLeftOut || bRightOut || bUpOut || bDownOut);
}

void ArcadeInputComponent::SetDirectionKey(eMoveButtonDir moveDir, bool bPressed, bool bBroadcastKeyIfChanged )
{
	m_buttons[moveDir].OnPressToggle(bPressed, m_customSignal, bBroadcastKeyIfChanged);
}

void ArcadeInputComponent::ResetDirectionKeys(bool bBroadcastKeyIfChanged )
{
	for (int i=0; i < MOVE_BUTTON_DIR_COUNT; i++)
	{
		m_buttons[i].OnPressToggle(false, m_customSignal, bBroadcastKeyIfChanged);
	}
}


void ArcadeInputComponent::RemoveKeyBindingsStartingWith( VariantList *pVList )
{
	ArcadeBindList::iterator itor = m_bindings.begin();


	for (;itor != m_bindings.end();)
	{
		if (StringFromStartMatches(itor->m_name, pVList->Get(0).GetString()))
		{
#ifdef _DEBUG
			LogMsg("Removing binding %s (%d left)", itor->m_name.c_str(), m_bindings.size());
#endif
			//ArcadeBindList::iterator itorTemp = itor;
			//itor++;
			m_bindings.erase(itor++);
			if (m_bindings.empty()) return;
		} else
		{
			 itor++;
		}
	}
}

void AddKeyBinding(EntityComponent *pComp, string name, uint32 inputcode, uint32 outputcode, bool bAlsoSendAsNormalRawKey, uint32 modifiersRequired)
{
    VariantList vList(name, inputcode, outputcode, uint32(bAlsoSendAsNormalRawKey!=0), modifiersRequired);
	pComp->GetFunction("AddKeyBinding")->sig_function(&vList);
}

string ProtonVirtualKeyToString(eVirtualKeys vKey)
{

	switch(vKey)
	{
	case VIRTUAL_DPAD_BUTTON_LEFT:
		return "left action button";
		break;
	case VIRTUAL_DPAD_BUTTON_RIGHT:
		return "right action button";
		break;
	case VIRTUAL_DPAD_BUTTON_UP:
		return "up action button";
		break;
	case VIRTUAL_DPAD_BUTTON_DOWN:
		return "down action button";
		break;

	case VIRTUAL_DPAD_LBUTTON:
		return "left shoulder button";
		break;

	case VIRTUAL_DPAD_RBUTTON:
		return "right shoulder button";
		break;

	case VIRTUAL_DPAD_SELECT:
		return "select button";
		break;

	case VIRTUAL_DPAD_START:
		return "start button";
		break;

	case VIRTUAL_DPAD_HAT_LEFT:
		return "left hat button";
		break;

	case VIRTUAL_DPAD_HAT_RIGHT:
		return "right hat button";
		break;

	case VIRTUAL_DPAD_HAT_UP:
		return "up hat button";
		break;

	case VIRTUAL_DPAD_HAT_DOWN:
		return "down hat button";
		break;

	default:
		return "Button ID "+toString(vKey);
		break;
	}

	assert(!"wat?");
	return "";
}