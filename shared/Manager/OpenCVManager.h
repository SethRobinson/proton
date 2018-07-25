//  ***************************************************************
//  OpenCVManager - Creation date: 07/18/2018
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2018 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*

//this needs to be linked with zbar, OpenCV, and libraspicam (optional, but much faster)
//raspicam is here:  https://github.com/cedricve/raspicam
  

Usage:

m_openCV.SetCaptureFPS(5);
m_openCV.SetCaptureSize(320, 240);
m_openCV.InitCamera();

//

if (m_openCV.ReadFromCamera())
{
	m_openCV.CopyLastFrameToSoftSurface();
	m_openCV.DecodeQRCode(); //optional
	
	if (m_openCV.GetCountRead() > 0)
	{
	  //the data that was inside the QR code:
	   m_openCV.GetQRReadByIndex(0)->data 
	}
}
*/
#ifndef OpenCVManager_h__
#define OpenCVManager_h__


#ifndef WINAPI
	#define RT_USE_LIBRASPICAM
#endif

#if defined(RT_USE_LIBRASPICAM)

#include <raspicam/raspicam.h>
#endif


#include <opencv2/opencv.hpp>

#include "Renderer/SoftSurface.h"

typedef struct
{
	string type;
	string data;
	vector <CL_Vec2i> location;
} QRCodeInfo;


class OpenCVManager
{
public:
	OpenCVManager();
	virtual ~OpenCVManager();

	void DecodeQRCode();
	bool InitCamera(int deviceID = -1);
	
	
	//set before InitCamera
	//many (most?) cameras ignore this setting or use it weirdly.  Proton does its own timing to fake a low FPS.
	void SetCaptureFPS(int fps);
	
	//set before InitCamera
	void SetCaptureSize(int width, int height);

	bool ReadFromCamera();
	SoftSurface * GetSoftSurface() { return &m_surface; }
	void CopyLastFrameToSoftSurface();

	int GetCountRead() { return (int)m_decodedObjects.size(); }
	QRCodeInfo * GetQRReadByIndex(int index) { return &m_decodedObjects[index]; }
	void SetSharpness(int sharpness);
	void SetContrast(int contrast);
	void SetBrightness(int brightness);
	void SetSaturation(int saturation);

protected:
	

private:
	SoftSurface m_surface;

#if defined(RT_USE_LIBRASPICAM)
	raspicam::RaspiCam m_capture; //Cmaera object
#else
	cv::VideoCapture m_capture;
	cv::Mat m_frame;

#endif
	unsigned int m_frameTimer;
	int m_lastFPSSetInMS;
	vector<QRCodeInfo> m_decodedObjects;
	CL_Vec2i m_lastCaptureSizeSet;
	int m_lastFPSSet;

};

#endif // OpenCVManager_h__
