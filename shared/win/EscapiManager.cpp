#include "PlatformPrecomp.h"
#include "EscapiManager.h"
#include "Entity/EntityUtils.h"

EscapiManager::EscapiManager()
{
	capture.mHeight = 0;
	capture.mWidth = 0;

	m_timeOfLastCapture = GetTick();
}

EscapiManager::~EscapiManager()
{
	if (m_bInitted)
	{
		deinitCapture(m_captureDeviceID);
		m_bInitted = false;
	}
}

void EscapiManager::SetPauseCapture(bool bPaused)
{
	m_bPaused = bPaused;
}

int EscapiManager::ElapsedMSFromLastCaptureActivity()
{
	return GetTick() - m_timeOfLastCapture;
}

void EscapiManager::Update(int minimumLumaPerColorChannel)
{

	/*
	if (ElapsedMSFromLastCaptureActivity() > 5000)
	{
		LogMsg("Warning, last capture was %d MS ago, requesting reinit ", ElapsedMSFromLastCaptureActivity());
		RequestReInit();
		m_timeOfLastCapture = GetTick();
	}

	*/
	if (isCaptureDone(0))
	{
		m_timeOfLastCapture = GetTick();

		if (m_bPaused == false)
		{
			m_captureScreenshot.Blit(0, 0, &m_captureSurface);
			m_captureScreenshot.FlipRedAndBlue();
			m_captureScreenshot.FlipY();
			
			
			//also make a version with no dark blacks, to help with my luma-keying
			m_captureAdjustedForKeying.Blit(0, 0, &m_captureScreenshot);
			m_captureAdjustedForKeying.RemoveTrueBlack(minimumLumaPerColorChannel);

			m_surface.InitFromSoftSurface(&m_captureAdjustedForKeying, true, 0);
		}

		//start another one
		doCapture(m_captureDeviceID);
		//LogMsg("Doing capture...");
	}
}
void EscapiManager::RequestReInit()
{
	if (m_bInitted)
	{ 
		deinitCapture(0);
		m_bInitted = false;
	}

	if (capture.mWidth == 0)
	{
		LogMsg("Ignoring reinit capture command, wasn't initted in the first place");
		return;
	}
	Init(capture.mWidth, capture.mHeight, m_captureDeviceID);
}

bool EscapiManager::Init(int captureWidth, int captureHeight, int deviceID)
{

	int devices = setupESCAPI();
	m_bInitted = true;
	if (devices == 0)
	{
		ShowTextMessage("ESCAPI initialization failure or no devices found.", 1000, 0);
		return false;
	}

	capture.mWidth = captureWidth;
	capture.mHeight = captureHeight;
	
	m_captureSurface.Init(capture.mWidth, capture.mHeight, SoftSurface::SURFACE_RGBA);
	capture.mTargetBuf = (int*)m_captureSurface.GetPixelData();

	m_captureScreenshot.Init(capture.mWidth, capture.mHeight, SoftSurface::SURFACE_RGB);
	m_captureAdjustedForKeying.Init(capture.mWidth, capture.mHeight, SoftSurface::SURFACE_RGB);
	
	
	m_timeOfLastCapture = GetTick();


	m_captureDeviceID = deviceID;
	char capName[256];
	capName[0] = 0;
	getCaptureDeviceName(m_captureDeviceID, capName, 256);

	LogMsg("Detected %d camera input devices.  We'll use device %d (%s) (set input_camera_device_id in config.txt to change)", devices, m_captureDeviceID,
		capName);
	if (initCapture(m_captureDeviceID, &capture) == 0)
	{
		ShowTextMessage("Capture failed - device "+toString(m_captureDeviceID)+" incorrect or in use?");
		return false;
	}

	doCapture(m_captureDeviceID);

	return true; //success
}
