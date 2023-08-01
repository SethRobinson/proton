//  ***************************************************************
//  GamepadProviderDirectX - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*


11/4/2020 - Update:  I've added GameProviderXInput which handles XBox controllers a bit better via XInput.  You can use both of these together (many devices
will only be usable via directinput instead of xinput) but you should use the SetIgnoreXInputCapableDevices(true) so you don't map the same gamepads twice.

An advantage of XInput is in theory it could be used to do vibration and also works if you didn't plug in/turn on when the app started.

Handles gamepads for Proton's GamepadManager in windows. It will detect most popular pads correctly.

Handles multiple gamepads.

Problems:  Doesn't handle the left and right analog triggers right.  One axis split between them, but both off is 0, and both
on is zero?  Uh.. oh well, fix when you need the triggers working I guess.

To use, you would do:

GetGamepadManager()->AddProvider(new GamepadProviderDirectX);

Which assumes you already have something like:


GamepadManager g_gamepadManager;
GamepadManager * GetGamepadManager() {return &g_gamepadManager;}

in your App.cpp or somewhere.

From there, you can use GetGamepadManager()->GetDefaultGamepad() and connect signals to it for buttons and stick data, or poll it.
*/

#ifndef GamepadProviderDirectX_h__
#define GamepadProviderDirectX_h__

#include "GamepadProvider.h"

#define DIRECTINPUT_VERSION 0x0800

#if defined(_MSVC_LANG) && _MSVC_LANG <= 201402L
	


  #include <dinput.h>
#else
	#define byte rpc_byte
	#include <dinput.h>
	#undef byte
#endif

#if DIRECTINPUT_HEADER_VERSION < 0x0800
#error Found DirectX headers older than 8.0. Please download a newer directx, and make sure its FIRST in the include path and library path (Tools->Options->Directories in MSVC).
#endif

class GamepadProviderDirectX: public GamepadProvider
{
public:
	GamepadProviderDirectX();
	virtual ~GamepadProviderDirectX();

	virtual string GetName() {return "DirectX";}
	virtual bool Init();
	virtual void Kill();
	virtual void Update();

	void SetIgnoreXInputCapableDevices(bool bIgnoreThem) { m_ignoreXInputCapableDevices = bIgnoreThem; }
	bool GetIgnoreXInputCapableDevices() { return m_ignoreXInputCapableDevices; }

	//Used by our custom GamepadDirectX class
	HWND GetHWND();
	LPDIRECTINPUT8 GetDInput(){return directinput;}

protected:
	

private:

	LPDIRECTINPUT8 directinput;
	static BOOL CALLBACK enum_devices_callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	bool m_ignoreXInputCapableDevices;
};

#endif // GamepadProviderDirectX_h__