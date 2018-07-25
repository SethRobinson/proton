#include "PlatformPrecomp.h"
#include "Gamepad.h"
#include "Entity/ArcadeInputComponent.h"


#ifdef _DEBUG
	//#define SHOW_GAMEPAD_DEBUG_STUFF
#endif


Gamepad::Gamepad()
{
	m_pPadProvider = NULL;
	m_name = "Unknown";
	m_buttonsUsedCount = 0;
	m_axisUsedCount = 0;
	m_bIsUsed = false;
	m_pArcadeComp = NULL;
	m_bJustSentStickRelease = false;

	SetRightStickAxis(7, 2);

	for (int i=0; i < GAMEPAD_MAX_BUTTONS; i++)
	{
		m_buttons[i].m_virtualKey = (eVirtualKeys)(VIRTUAL_DPAD_BUTTON_LEFT+i); //defaults
	}
}

Gamepad::~Gamepad()
{
	LogMsg("Removing gamepad %s", m_name.c_str());
}

void Gamepad::SetAxis( int axis, float val )
{
	m_axis[axis].m_axis = val;

#ifdef SHOW_GAMEPAD_DEBUG_STUFF
	if (val != 0)	LogMsg("Got axis %d: %.2f", axis, val);
#endif

}

void Gamepad::OnButton( bool bDown, int buttonID )
{
	m_buttons[buttonID].OnPress(bDown);
	
#ifdef SHOW_GAMEPAD_DEBUG_STUFF
	LogMsg("Button %d is %s", buttonID, bDown ? "Down" : "Up");
#endif
	
	VariantList v;
	v.Get(0).Set(uint32(m_buttons[buttonID].m_virtualKey));
	v.Get(2).Set(uint32(m_id));
	if (bDown)
	{
		v.Get(1).Set(uint32(VIRTUAL_KEY_PRESS)); 
	} else
	{
		v.Get(1).Set(uint32(VIRTUAL_KEY_RELEASE)); 
	}

	m_sig_gamepad_buttons(&v);
}

void Gamepad::SendArcadeDirectionByKey(eVirtualKeys key, bool bDown)
{
    if (!m_pArcadeComp) return;
    
	switch (key)
	{
        case VIRTUAL_KEY_DIR_UP:
            m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, bDown);
            break;
        case VIRTUAL_KEY_DIR_DOWN:
            m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, bDown);
            break;
        case VIRTUAL_KEY_DIR_LEFT:
            m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, bDown);
            break;
        case VIRTUAL_KEY_DIR_RIGHT:
            m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, bDown);
            break;
            
        default:
            LogMsg("SendArcadeDirectionByKey Unhandled direction ( %d)", key);
	}
    
}

void Gamepad::SendArcadeDirectionByDegrees(int val)
{
	//seriously?  Don't we have a better way to do this?

	switch (int(val))
	{
	case 0:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
		break;

	case 45:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, true);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
		break;

	case 135:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, true);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
		break;

	case 180:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
		break;

	case 225:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, true);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
		break;

	case 270:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
		break;

	case 315:
	case -45:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, true);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
		break;

	case 90:
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, true);

		//turn off the rest, if needed
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
		m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
		break;
	
	default:
		LogMsg("Unhandled direction ( %d), you should preprocess to be one we accept before sending here", val);
	}

}

void Gamepad::SendArcadeDirectionRelease()
{
	//just shut them all off
	m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_DOWN, false);
	m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_LEFT, false);
	m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_RIGHT, false);
	m_pArcadeComp->SetDirectionKey(MOVE_BUTTON_DIR_UP, false);
}

void Gamepad::OnHat( int index, float val )
{
	//ignoring this for now.. add as extra buttons later?
#ifdef SHOW_GAMEPAD_DEBUG_STUFF
	LogMsg("Hat index %d is %.2f", index, val);
#endif

	if (!m_pArcadeComp) return;

	if (val == -1)
	{
		//just shut them all off
		SendArcadeDirectionRelease();
		return;
	}

	SendArcadeDirectionByDegrees((int)val);
	
}


void Gamepad::SetRightStickAxis( int axisX, int axisY )
{
	m_rightStickAxisX = axisX;
	m_rightStickAxisY = axisY;

}

void Gamepad::OnArcadeCompDestroyed( VariantList *pVList )
{
	assert(m_pArcadeComp);
	m_pArcadeComp = NULL; //stop sending things to it, it's dead, jim

}

void Gamepad::ConnectToArcadeComponent( ArcadeInputComponent *pComp, bool bSendButtonEvents, bool bSendPadEventsAsFourDirections )
{

	if (m_pArcadeComp)
	{
		LogMsg("Gamepad %s disconnecting itself from old arcade component so we can connect to the new one", GetName().c_str());
		m_pArcadeComp->GetFunction("OnDelete")->sig_function.disconnect(boost::bind(&Gamepad::OnArcadeCompDestroyed, this, _1));
		m_pArcadeComp = NULL;
	}

	if (bSendButtonEvents)
	{
		m_sig_gamepad_buttons.connect(1, boost::bind(&ArcadeInputComponent::OnRawKeyboard, pComp, _1));
	}

	if (bSendPadEventsAsFourDirections)
	{
		m_pArcadeComp = pComp;
		//we should get notified if this component is destroyed
		pComp->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&Gamepad::OnArcadeCompDestroyed, this, _1));
	}
}

GamepadButton * Gamepad::GetButton( int buttonID )
{
	assert(buttonID < GAMEPAD_MAX_BUTTONS && "No button this high");
	return &m_buttons[buttonID];
}

CL_Vec2f Gamepad::GetLeftStick()
{
	return CL_Vec2f(m_axis[0].m_axis, m_axis[1].m_axis);
}

CL_Vec2f Gamepad::GetRightStick()
{
	return CL_Vec2f(m_axis[m_rightStickAxisX].m_axis, m_axis[m_rightStickAxisY].m_axis);
}

void Gamepad::Update()
{
	if (GetLeftStick() != m_vLastSentLeftStick)
	{
		VariantList v(GetLeftStick(), (int32)m_id, (int32)0);
		m_sig_left_stick(&v);
		m_vLastSentLeftStick = GetLeftStick();

		if (m_pArcadeComp)
		{
			//let's convert to 8 way directional signals as well
			const float deadSpace = 0.3f; //TODO: make member var and add an accessor to it?
			if (GetLeftStick().length() > deadSpace)
			{
				//convert to 360 degrees
				int dir = (int(RAD2DEG(atan2(GetLeftStick().y, GetLeftStick().x))+(180)));
				//convert into 8 directions
				int finaldir = mod(dir-(45/2), 360)/45;
#ifdef _DEBUG
	//			LogMsg("Pressing %s, which is dir %d (final: %d)", PrintVector2(GetLeftStick()).c_str(), dir, finaldir);
#endif
				SendArcadeDirectionByDegrees( (finaldir*45)-45 );
				m_bJustSentStickRelease = false;
			} else
			{
				//cancel all dpad movement - but only if we've actually moved inbetween, otherwise slight changes in the stick
				//will keep sending cancels for no reason, which can affect other forums of input

				if (!m_bJustSentStickRelease)			
				{
					SendArcadeDirectionRelease();
					m_bJustSentStickRelease = true;

				}
			}

		}
	}

	if (GetRightStick() != m_vLastSentRightStick)
	{
		VariantList v(GetRightStick(), (int32)m_id, (int32)1);
		m_sig_right_stick(&v);
		m_vLastSentRightStick = GetRightStick();
	}
}

void GamepadButton::OnPress( bool bDown )
{
	m_bDown = bDown;
	//LogMsg(")
}