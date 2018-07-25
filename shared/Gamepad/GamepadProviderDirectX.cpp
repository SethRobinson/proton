#include "PlatformPrecomp.h"
#include "GamepadProviderDirectX.h"
#include "GamepadDirectX.h"
#include "GamepadManager.h"

HWND hwndMain;

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParam)
{
	HINSTANCE hinst=(HINSTANCE)GetModuleHandle(NULL);


#ifdef _WIN64

if((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE)==hinst &&
   IsWindowVisible(hwnd))
#else
	if((HINSTANCE)GetWindowLongPtr(hwnd, GWL_HINSTANCE)==hinst &&
		IsWindowVisible(hwnd))
#endif
	{
		hwndMain=hwnd;
		return FALSE;
	}
	else
		return TRUE;
}


GamepadProviderDirectX::GamepadProviderDirectX()
{
	
}

GamepadProviderDirectX::~GamepadProviderDirectX()
{
}

bool GamepadProviderDirectX::Init()
{
	LogMsg("Initting windows gamepad provider");
	
	hwndMain=NULL;
	EnumWindows(EnumWindowProc, 0);

	HRESULT result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *) &directinput, 0);
	if (FAILED(result))
	{
		LogError("debug", "Unable to initialize direct input");
		return false;
	}


	//enumerate each device
	result = directinput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		&GamepadProviderDirectX::enum_devices_callback,
		this,
		DIEDFL_ATTACHEDONLY);
	if (FAILED(result))
	{
		LogError("Unable to enumerate direct input devices");
	}

	return true; //success
}

void GamepadProviderDirectX::Kill()
{
	SAFE_RELEASE(directinput);
}


BOOL GamepadProviderDirectX::enum_devices_callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	GamepadProviderDirectX *self = (GamepadProviderDirectX *) pvRef;

	
	GamepadDirectX *pPad = new GamepadDirectX;
	pPad->SetProvider(self);
	pPad->SetDeviceInstance(*lpddi);
	GetGamepadManager()->AddGamepad(pPad);

	//	CL_InputDevice device(new CL_InputDevice_DirectInput(self, lpddi));
	//	self->input_context.add_joystick(device);
	
	return TRUE;
}


void GamepadProviderDirectX::Update()
{
	
}

HWND GamepadProviderDirectX::GetHWND()
{
	return hwndMain;
}
