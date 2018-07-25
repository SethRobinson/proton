#include "PlatformPrecomp.h"
#include "GamepadDirectX.h"

GamepadDirectX::GamepadDirectX()
{
	m_lpDirectInputDevice = NULL;
}

GamepadDirectX::~GamepadDirectX()
{
	Kill();
}

bool GamepadDirectX::Init()
{

	HRESULT result = ((GamepadProviderDirectX*)m_pPadProvider)->GetDInput()->CreateDevice( m_diDeviceInstance.guidInstance, &m_lpDirectInputDevice,0);

	if (FAILED(result))
	{
		LogError("Unable to open directinput device");
		return false;
	}

	result = m_lpDirectInputDevice->SetCooperativeLevel(((GamepadProviderDirectX*)m_pPadProvider)->GetHWND(), DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		LogError ("Unable to the set cooperative level");
		return false;
	}

	result = m_lpDirectInputDevice->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(result))
	{
		LogError ("Unable to set device dataformat");
		return false;
	}

	// Enable buffering of input events:
	DIPROPDWORD value;
	memset(&value, 0, sizeof(DIPROPDWORD));
	value.diph.dwSize = sizeof(DIPROPDWORD);
	value.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	value.diph.dwObj = 0;
	value.diph.dwHow = DIPH_DEVICE;
	value.dwData = 128;
	result = m_lpDirectInputDevice->SetProperty(DIPROP_BUFFERSIZE, &value.diph);
	if (FAILED(result))
	{
		LogError("Unable to set buffer size attribute on device");
	}

	result = m_lpDirectInputDevice->Acquire();
	if (FAILED(result))
	{
		LogError("Unable to acquire device");
	}


	//get info on its buttons and axis
	// record number of values and states
	DIDEVCAPS  caps; 

	caps.dwSize = sizeof(DIDEVCAPS); 
	if (FAILED(m_lpDirectInputDevice->GetCapabilities(&caps)))
	{
		LogError("debug", "Unable to getcaps of direct input");
	}
	m_axisUsedCount = 0;
	// Enumerate the axes of the joyctick 
	if (FAILED(m_lpDirectInputDevice->EnumObjects(&GamepadDirectX::enum_axes_callback, (void*) this, DIDFT_AXIS)))
	{
		LogError("debug", "Unable to enumerate joystick axis");
	}


	// Enumerate the buttons available
	m_buttonsUsedCount = 0;
	if (FAILED(m_lpDirectInputDevice->EnumObjects(&GamepadDirectX::enum_button_callback, (void*) this, DIDFT_BUTTON)))
	{
		LogError("debug", "Unable to enumerate joystickbuttons");
	}

	m_name = m_diDeviceInstance.tszProductName;

	//these defaults are actually for a 360 pad.  Probably wrong for other pads..?

	m_buttons[0].m_virtualKey = VIRTUAL_DPAD_BUTTON_DOWN;
	m_buttons[1].m_virtualKey = VIRTUAL_DPAD_BUTTON_RIGHT;
	m_buttons[2].m_virtualKey = VIRTUAL_DPAD_BUTTON_LEFT;
	m_buttons[3].m_virtualKey = VIRTUAL_DPAD_BUTTON_UP;

	m_buttons[4].m_virtualKey = VIRTUAL_DPAD_LBUTTON;
	m_buttons[5].m_virtualKey = VIRTUAL_DPAD_RBUTTON;
	m_buttons[6].m_virtualKey = VIRTUAL_DPAD_SELECT;
	m_buttons[7].m_virtualKey = VIRTUAL_DPAD_START;


	//hack for the xbox right stick.  Add real profiles later?
	if (m_name.find("Xbox 360 Wireless") != string::npos)
	{
		//improve mappings
		SetRightStickAxis(5, 6);
		//LogMsg("XBOX Wireless pad detected");
	} else
	if (m_name.find("XBOX") != string::npos)
	{
		//improve mappings
		SetRightStickAxis(5, 6);

	}

	return true;
}


BOOL CALLBACK GamepadDirectX::enum_axes_callback( const DIDEVICEOBJECTINSTANCE* instance, void* context )
{
	GamepadDirectX *pPad = (GamepadDirectX *)context;
	pPad->m_axisUsedCount++;
	return DIENUM_CONTINUE;
}

BOOL CALLBACK GamepadDirectX::enum_button_callback( const DIDEVICEOBJECTINSTANCE* instance, void* context )
{
	GamepadDirectX *pPad = (GamepadDirectX *)context;
	pPad->m_buttonsUsedCount++;
	return DIENUM_CONTINUE;
}

void GamepadDirectX::Kill()
{
	SAFE_RELEASE(m_lpDirectInputDevice);
}

void GamepadDirectX::Update()
{
	m_lpDirectInputDevice->Poll();

	// Get events:
	while (true)
	{
		DIDEVICEOBJECTDATA buffer[16];
		DWORD num_events = 16;

		HRESULT result = m_lpDirectInputDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), buffer, &num_events, 0);
		// Try to reacquire joystick if we lost it.
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) m_lpDirectInputDevice->Acquire();

		if (FAILED(result) && result != DI_BUFFEROVERFLOW) break;
		if (num_events == 0) break;
	
		for (unsigned int i=0; i<num_events; i++)
		{
			if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lX))
			{
				SetAxis(0, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}	else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lY))
			{
				// Y Axis position event
				SetAxis(1, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			} else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lZ))
			{
				// Z Axis position event
				SetAxis(2, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, rglSlider[0]))
			{
				// extra 1 axis position event
				SetAxis(3, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}
			else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, rglSlider[1]))
			{
				// extra 2 axis position event
				SetAxis(4, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}
			else if (buffer[i].dwOfs >= FIELD_OFFSET(DIJOYSTATE2, rgbButtons) && buffer[i].dwOfs < FIELD_OFFSET(DIJOYSTATE2, rgbButtons)+128)
			{
				// Button event
				int button_index = buffer[i].dwOfs - FIELD_OFFSET(DIJOYSTATE2, rgbButtons);
				// If high bit of lower byte is set, key is down
				OnButton( LOBYTE(buffer[i].dwData) != 0, button_index);
			} else if (buffer[i].dwOfs >= FIELD_OFFSET(DIJOYSTATE2, rgdwPOV) && buffer[i].dwOfs < FIELD_OFFSET(DIJOYSTATE2, rgdwPOV)+4*sizeof(DWORD))
			{
				// Hat event:
				int hat_index = (buffer[i].dwOfs - FIELD_OFFSET(DIJOYSTATE2, rgdwPOV)) / sizeof(DWORD);
				bool centered = (LOWORD(buffer[i].dwData) == 0xFFFF);
				int direction = buffer[i].dwData / DI_DEGREES;
				if (centered) direction = -1;

				OnHat(hat_index, (float)direction); 
			}
			else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lRx))
			{
				// X Axis rotation event
				SetAxis(5, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}
			else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lRy))
			{
				// Y Axis rotation event
				SetAxis(6, float(buffer[i].dwData)/(0xffff/2)-1.0f);

			}
			else if (buffer[i].dwOfs == FIELD_OFFSET(DIJOYSTATE2, lRz))
			{
				// Z Axis rotation event
				SetAxis(7, float(buffer[i].dwData)/(0xffff/2)-1.0f);
			}
			{
				//Ignoring hat data, force, torque, rotation, etc for now
#ifdef _DEBUG
				//LogMsg("Unhandled gamepad event");
#endif
			}

		}
	}

	Gamepad::Update();
}