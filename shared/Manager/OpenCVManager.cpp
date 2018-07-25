#include "PlatformPrecomp.h"
#include "OpenCVManager.h"
//#include "util/libdecoderqr/decodeqr.h"
#include "util/zbar/zbar.h"

#ifdef WINAPI
//don't ask me why, but VC2017 needs this cast to compile
#define RT_ICONV_CAST (const char**)
#include "util/libiconv/iconv.h"
#else
#define RT_ICONV_CAST
#include <iconv.h>
#endif

#if defined(RT_USE_LIBRASPICAM)
using namespace raspicam;
#endif

using namespace cv;
using namespace zbar;

OpenCVManager::OpenCVManager()
{
	m_lastCaptureSizeSet = CL_Vec2i(0, 0);
	m_frameTimer = 0;
	m_lastFPSSet = 0;
}

OpenCVManager::~OpenCVManager()
{
	
}

string FixQRBinaryDataEncoding(string input)
{
	iconv_t cd = iconv_open("ISO-8859-1", "UTF-8");
	if (cd == (iconv_t)-1)
	{
		LogMsg("iconv_open failed!");
		return "";
	}
	int buffSize = (int)input.length() * 2;

	string output;
	char *pOutputBuf = new char[buffSize]; //plenty of space
	size_t outbytes = buffSize;
	size_t inbytes = input.length();

	char *pOutPtr = pOutputBuf;
	char *pSrcPtr = &input.at(0);

	do {
		if (iconv(cd, RT_ICONV_CAST &pSrcPtr, &inbytes, &pOutPtr, &outbytes) == (size_t)-1)
		{
			LogMsg("iconv failed!");
			SAFE_DELETE_ARRAY(pOutputBuf);
			return "";
		}
	} while (inbytes > 0 && outbytes > 0);
	
	iconv_close(cd);
	int finalOutputByteSize = (int)buffSize-(int)outbytes;
	string temp;
	temp.resize(finalOutputByteSize);
	memcpy((void*)temp.c_str(), pOutputBuf, finalOutputByteSize);
	SAFE_DELETE_ARRAY(pOutputBuf);

	return temp;
}


// Find and decode barcodes and QR codes
void decodeSoftSurface(vector<QRCodeInfo>&decodedObjects, SoftSurface *pSoftSurface)
{


	// Create zbar scanner
	ImageScanner scanner;

	// Configure scanner
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

	//load data directly from the SoftSurface
	Image image(pSoftSurface->GetWidth(), pSoftSurface->GetHeight(), "RGB3", (uchar *)pSoftSurface->GetPixelData(), pSoftSurface->GetWidth() * pSoftSurface->GetHeight()*3);


	Image convertedImage = image.convert(*(unsigned int*)"Y800");
	
	//LogMsg("Ok, converted image is now %d bytes instead of %d bytes. ", (int)convertedImage.get_data_length(), pSoftSurface->GetWidth() * pSoftSurface->GetHeight() * 3);
							   // Scan the image for barcodes and QRCodes
	int n = scanner.scan(convertedImage);

	// Print results
	for (Image::SymbolIterator symbol = convertedImage.symbol_begin(); symbol != convertedImage.symbol_end(); ++symbol)
	{
		QRCodeInfo obj;

		string qrData = FixQRBinaryDataEncoding(symbol->get_data());

		obj.type = symbol->get_type_name();
		obj.data = qrData;

		// Print type and data
		LogMsg("Found %s.  %d bytes.", obj.type.c_str(), obj.data.length());
		LogMsg("Data: %s", obj.data.c_str());

		//LogMsg(DataToByteHexDisplay(qrData, 20).c_str());

		// Obtain location
		for (int i = 0; i < symbol->get_location_size(); i++)
		{
			obj.location.push_back(CL_Vec2i(symbol->get_location_x(i), symbol->get_location_y(i)));
		}

		decodedObjects.push_back(obj);
	}
}

// Find and decode barcodes and QR codes
void decode(Mat &im, vector<QRCodeInfo>&decodedObjects)
{

	// Create zbar scanner
	ImageScanner scanner;

	// Configure scanner
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

	// Convert image to grayscale
	Mat imGray;
	cvtColor(im, imGray, CV_BGR2GRAY);

	// Wrap image data in a zbar image
	Image image(im.cols, im.rows, "Y800", (uchar *)imGray.data, im.cols * im.rows);

	// Scan the image for barcodes and QRCodes
	int n = scanner.scan(image);

	// Print results
	for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
	{
		QRCodeInfo obj;

		string qrData = FixQRBinaryDataEncoding(symbol->get_data());

		obj.type = symbol->get_type_name();
		obj.data = qrData;
		
		// Print type and data
		LogMsg("Found %s.  %d bytes.", obj.type.c_str(), obj.data.length());
		LogMsg("Data: %s", obj.data.c_str());
		
		//LogMsg(DataToByteHexDisplay(qrData, 20).c_str());

		// Obtain location
		for (int i = 0; i < symbol->get_location_size(); i++)
		{
			obj.location.push_back(CL_Vec2i(symbol->get_location_x(i), symbol->get_location_y(i)));
		}

		decodedObjects.push_back(obj);
	}
}

void OpenCVManager::DecodeQRCode()
{

	m_decodedObjects.clear();
	// Find and decode barcodes and QR codes
	
#if defined(RT_USE_LIBRASPICAM)
	decodeSoftSurface(m_decodedObjects, &m_surface);
#else

	//decodeSoftSurface(m_decodedObjects, &m_surface);
	decode(m_frame, m_decodedObjects);
#endif

	if (m_decodedObjects.size() > 0)
	{
		LogMsg("Found %d QR codes.", m_decodedObjects.size());
	}
	
}

bool OpenCVManager::InitCamera(int deviceID)
{
#ifdef WINAPI
	if (deviceID == -1)
	{
		LogMsg("OpenCVManager::InitCamera is changing device from -1 to 0, OpenCV on windows doesn't like it I guess.");
		deviceID = 0; 
	}
#endif
	


#ifdef RT_USE_LIBRASPICAM
	
	m_capture.setFormat(raspicam::RASPICAM_FORMAT_BGR);
	//m_capture.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
	if (!m_capture.open())
	{
		LogMsg("Can't open camera");
		return false;
	}

	//Camera.setFormat(raspicam::RASPICAM_FORMAT_IGNORE);
	
#else

	if (!m_capture.open(deviceID))
	{
		LogMsg("Can't open camera");
		return false;
	}

	if (m_lastCaptureSizeSet.x != 0 && m_lastCaptureSizeSet.y != 0)
		SetCaptureSize(m_lastCaptureSizeSet.x, m_lastCaptureSizeSet.y);

	if (m_lastFPSSet != 0)
	{
		SetCaptureFPS(m_lastFPSSet);
	}
	//OpenCV needs this set now, but raspicam needs it set BEFORE the open.  This way the raspicam way will work with both APIs.
#endif

	LogMsg("Camera initialized. (Opened device %d)", deviceID);

	//To see what your cam can do on a raspberry pi do: v4l2-ctl -d 0 --list-formats-ext 

	
	//m_capture.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));

	


	return true;
}


void OpenCVManager::SetCaptureFPS(int fps)
{
#ifdef RT_USE_LIBRASPICAM
	if (m_capture.isOpened())
	{
		if (GetBaseApp()->IsInitted())
		LogMsg("Can't set capture size, you have to do it before you do Init with libraspicam!");
		return;
	}

	m_capture.setFrameRate(fps);
#else
	if (!m_capture.set(CV_CAP_PROP_FPS, fps))
	{
		if (GetBaseApp()->IsInitted())
		LogMsg("Error setting cam FPS");
	}

//	int reportedFPS = m_capture.get(CV_CAP_PROP_FPS);
//	LogMsg("Cam reported FPS is %d", reportedFPS);
#endif

	m_lastFPSSetInMS = (int)(1000.0f / (float)fps);
	m_lastFPSSet = fps;
}

void OpenCVManager::SetCaptureSize(int width, int height)
{
#ifdef RT_USE_LIBRASPICAM

	if (m_capture.isOpened())
	{
		LogMsg("Can't set capture size, you have to do it before you do Init with libraspicam!");
		return;
	}
	m_capture.setWidth(width);
	m_capture.setHeight(height);
	

#else

	m_capture.set(CV_CAP_PROP_FRAME_WIDTH, width);
	m_capture.set(CV_CAP_PROP_FRAME_HEIGHT, height);

#endif

	m_lastCaptureSizeSet = CL_Vec2i(width, height);
}

bool OpenCVManager::ReadFromCamera()
{
	//this "grab" is slow and blocking.  Would need to put this in a thread to not ruin performance in the main thread I guess.
	//a hack is to assume the FPS set is correct and skip checking for frames.  It's crap but..

	if (m_frameTimer > GetTick())
	{
		//LogMsg("Waiting..");
		return false;
	}
	
	//m_capture.retrieve(m_frame);
	m_frameTimer = GetTick() + m_lastFPSSetInMS;

#if defined(RT_USE_LIBRASPICAM)


	m_capture.grab();

	if (m_surface.GetWidth() != m_capture.getWidth() || m_surface.GetHeight() != m_capture.getHeight())
	{
		//one time init
		m_surface.Init(m_capture.getWidth(), m_capture.getHeight(), SoftSurface::SURFACE_RGB);
		LogMsg("Reinitted soft surface. I hope it's now %d bytes.", m_capture.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));
	}

	LogMsg("Grabbing pic of %d by %d", m_capture.getWidth(), m_capture.getHeight() );
	//unsigned char *pData = new unsigned char[Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];
	m_capture.retrieve(m_surface.GetPixelData());//get camera image


	//SAFE_DELETE(pData);
// 	//allocate memory
// 	unsigned char *data = new unsigned char[Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];
// 	//extract the image in rgb format
// 	Camera.retrieve(data, raspicam::RASPICAM_FORMAT_RGB);//get camera image
// 		     												 //save
// 	std::ofstream outFile("raspicam_image.ppm", std::ios::binary);
// 	outFile << "P6\n" << Camera.getWidth() << " " << Camera.getHeight() << " 255\n";
// 	outFile.write((char*)data, Camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB

#else
	m_capture >> m_frame;

#endif

	//imshow("this is you, smile! :)", m_frame);
	//LogMsg("Trying to grab...");
	/*
	if (m_capture.grab())
	{
		//LogMsg("Capturing...");
		m_capture.retrieve(m_frame);
		m_frameTimer = GetTick() + m_lastFPSSetInMS;
		//got it. This work in a GUI environment like windows:
		
	}
	*/

	return true;
}

void OpenCVManager::CopyLastFrameToSoftSurface()
{

	
#ifdef RT_USE_LIBRASPICAM
	//already done actually
	return;

#else
	
	if (m_frame.rows <= 0)
	{
		LogMsg("Frame has no data");
		return;
	}

	if (m_surface.GetWidth() != m_frame.cols || m_surface.GetHeight() != m_frame.rows)
	{
		//one time init
		m_surface.Init(m_frame.cols, m_frame.rows, SoftSurface::SURFACE_RGB);
	}
	//LogMsg("Created soft surface of %d by %d", m_surface.GetWidth(), m_surface.GetHeight());
	assert(m_frame.channels() == 3 && "Huh?  Is there alpha in here or something?");

	
	//for testing a solid color
	//m_surface.FillColor(glColorBytes(0, 255, 0, 255));

	//this would works but you still have the colors in the wrong order. So we'll do it the slower way below
	//m_frame = m_frame.t();
	//memcpy(m_surface.GetPixelData(), m_frame.data, 3 * m_frame.cols*m_frame.rows);
	
	byte * framePixels = m_frame.data; //Pointer to the first pixel data ,it's return array in all values
	
	byte *softSurfacePixels = (byte*)m_surface.GetPixelData();

	const int bytesPerPixel = 3;

	byte *pSrc;
	byte *pDst;

	bool bFlipImageY = true;

	for (int x = 0; x < m_frame.cols; x++)
	{
		for (int y = 0; y < m_frame.rows; y++)
		{
			pSrc = &framePixels[(x + m_frame.cols*y)*bytesPerPixel];

			if (bFlipImageY)
			{
				//note: we're flipping the Y here so it matches GL format
				pDst = &softSurfacePixels[(x + m_frame.cols* ((m_frame.rows - y) - 1))*bytesPerPixel];
			}
			else
			{
				pDst = &softSurfacePixels[(x + m_frame.cols* y)*bytesPerPixel];
			}

			//as for color, src is BGR, so we're flipping to RGB
			pDst[2] = pSrc[0]; 
			pDst[1] = pSrc[1]; 
			pDst[0] = pSrc[2]; 
			
		}
	}

#endif

}

void OpenCVManager::SetSharpness(int sharpness)
{
	#ifdef RT_USE_LIBRASPICAM
	m_capture.setSharpness(sharpness);
		
	#endif
}

void OpenCVManager::SetContrast(int contrast)
{
#ifdef RT_USE_LIBRASPICAM
	m_capture.setSharpness(contrast);

#endif
}

void OpenCVManager::SetBrightness(int brightness)
{
#ifdef RT_USE_LIBRASPICAM
	m_capture.setBrightness(brightness);

#endif
}

void OpenCVManager::SetSaturation(int saturation)
{
#ifdef RT_USE_LIBRASPICAM
	m_capture.setSaturation(saturation);

#endif
}

