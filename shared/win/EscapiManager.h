//  ***************************************************************
//  EscapiManager - Creation date: 04/10/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EscapiManager_h__
#define EscapiManager_h__

#include "escapi.h"
#include "Renderer/Surface.h"
#include "Renderer/SoftSurface.h"

class EscapiManager
{
public:
	EscapiManager();
	virtual ~EscapiManager();

	void SetPauseCapture(bool bPaused);
	int ElapsedMSFromLastCaptureActivity();
	void Update(int minimumChroma);
	void RequestReInit();
	bool Init(int captureWidth, int captureHeight, int deviceID);
	SoftSurface * GetSoftSurface() { return &m_captureScreenshot; }
	Surface * GetSurface() { return &m_surface; }

protected:
	struct SimpleCapParams capture;
	bool m_bInitted = false;
	bool m_bPaused = false;
	Surface m_surface;
	SoftSurface m_captureSurface; //this is being updated in the background, dangerous touch

	SoftSurface m_captureScreenshot; //a copy we make so we can safely send it back
	SoftSurface m_captureAdjustedForKeying; //another copy but with luma adjusted
	unsigned int m_timeOfLastCapture = 0;
	int m_captureDeviceID = 0;
};

#endif // EscapiManager_h__#pragma once
