//  ***************************************************************
//  GamepadDirectX - Creation date: 01/27/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//The DirectInput code was mostly stolen from Clanlib, thanks guys!

#ifndef GamepadDirectX_h__
#define GamepadDirectX_h__

#include "Gamepad.h"
#include "GamepadProviderDirectX.h"

class GamepadDirectX: public Gamepad
{
public:
	GamepadDirectX();
	virtual ~GamepadDirectX();
	virtual bool Init();
	virtual void Kill();
	virtual void Update();
	
	void SetDeviceInstance(DIDEVICEINSTANCE instance) {m_diDeviceInstance = instance;}

protected:
	

private:

	static BOOL CALLBACK enum_axes_callback(const DIDEVICEOBJECTINSTANCE* instance, void* context);
	static BOOL CALLBACK enum_button_callback(const DIDEVICEOBJECTINSTANCE* instance, void* context);

	DIDEVICEINSTANCE m_diDeviceInstance;
	LPDIRECTINPUTDEVICE8 m_lpDirectInputDevice;

};

#endif // GamepadDirectX_h__