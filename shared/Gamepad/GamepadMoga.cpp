#include "PlatformPrecomp.h"
#include "GamepadMoga.h"


GamepadMoga::GamepadMoga()
{
	m_pPadProvider = NULL;

	//wire ourselves up to hear moga messages
	GetBaseApp()->m_sig_joypad_events.connect(1, boost::bind(&GamepadMoga::OnJoypadEvent, this, _1));
	m_bConnected = false;
}

GamepadMoga::~GamepadMoga()
{
	Kill();
}

bool GamepadMoga::Init()
{
	m_axisUsedCount = 4;
	SetRightStickAxis(2,3);
	m_name = "Moga";

	m_buttons[0].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
	m_buttons[1].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
	m_buttons[2].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
	m_buttons[3].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

	m_buttons[4].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
	m_buttons[5].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
	m_buttons[6].m_virtualKey = VIRTUAL_DPAD_SELECT;
	m_buttons[7].m_virtualKey = VIRTUAL_DPAD_START;
	m_buttons[6].m_virtualKey = VIRTUAL_DPAD_SELECT;
	m_buttons[7].m_virtualKey = VIRTUAL_DPAD_START;

	return true;
}

void GamepadMoga::Kill()
{

}

void GamepadMoga::Update()
{
	Gamepad::Update();
}

void GamepadMoga::OnJoypadEvent( VariantList *pVList )
{
	eMessageType message = (eMessageType) pVList->Get(0).GetUINT32();

	switch (message)
	{
	case MESSAGE_TYPE_GUI_JOYPAD_CONNECT:
		{
			bool bConnected = pVList->Get(1).GetUINT32() != 0;
			
			if (m_bConnected != bConnected) //stops a problem where we get two "connected" messages
			{
				if (bConnected)
				{
					LogMsg("Moga connected."); 
				} else
				{
					LogMsg("Moga disconnected.");
				}

				m_bConnected = bConnected;
			}
		}
		break;

	case MESSAGE_TYPE_GUI_JOYPAD_BUTTONS:
		{
			uint32 buttonID = pVList->Get(1).GetUINT32();
			bool bPressed = !pVList->Get(2).GetUINT32() != 0; //why is this backwards?  Uhh.. 
			//LogMsg("Moga: Button %d is %d", buttonID, bPressed);

			//convert:

			switch (buttonID)
			{
			case 96: OnButton(bPressed, 0); break;  //A button
			case 97: OnButton(bPressed, 1); break;  //B button
			case 98: OnButton(bPressed, 2); break;  //X button
			case 99: OnButton(bPressed, 3); break;  //Y button
	
			case 102: OnButton(bPressed, 4); break;  //L button
			case 103: OnButton(bPressed, 5); break;  //R button

			case 109: OnButton(bPressed, 6); break;  //select
			case 108: OnButton(bPressed, 7); break;  //start

			//we ignore these as we generate them ourselves from the joystick data in GamePad.cpp
			case 19: break; //up
			case 20: break; //down
			case 21: break; //left
			case 22: break; //right

			default: ;
			#ifdef _DEBUG
							LogMsg("Unknown moga button: %d", buttonID);
			#endif
			}

		}
		break;

	case MESSAGE_TYPE_GUI_JOYPAD:
		{
			float lx,ly, rx,ry;
			lx = pVList->Get(1).GetFloat();
			ly = pVList->Get(2).GetFloat();
			rx = pVList->Get(3).GetFloat();
			ry = pVList->Get(4).GetFloat();

			//LogMsg("Moga axis change: %.2f, %.2f, %.2f, %.2f", lx, ly, rx, ry);

			SetAxis(0, lx);
			SetAxis(1, ly);

			SetAxis(2, rx);
			SetAxis(3, ry);

		}

		break;
	
	default:

		LogMsg("Unknown moga message: %d", message);
	}
}