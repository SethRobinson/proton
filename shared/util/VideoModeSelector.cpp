#include "VideoModeSelector.h"

#include "MiscUtils.h"

using namespace std;

VideoMode::VideoMode() :
		name(),
		x(0),
		y(0),
		platformID(PLATFORM_ID_UNKNOWN),
		orientationMode(ORIENTATION_DONT_CARE)
{
}

VideoMode::VideoMode(string name, int x, int y, ePlatformID platformID, eOrientationMode orientationMode) :
		name(name),
		x(x),
		y(y),
		platformID(platformID),
		orientationMode(orientationMode)
{
}

string VideoMode::getName() const
{
	return name;
}

int VideoMode::getX() const
{
	return x;
}

int VideoMode::getY() const
{
	return y;
}

ePlatformID VideoMode::getPlatformID() const
{
	return platformID;
}

eOrientationMode VideoMode::getOrientationMode() const
{
	return orientationMode;
}

map<string, VideoMode> VideoModeSelector::g_videoModes;
vector<string> VideoModeSelector::g_videoModeNames;

void VideoModeSelector::addVideoMode(string name, int x, int y, ePlatformID platformID, eOrientationMode orientationMode)
{
	g_videoModes[name] = VideoMode(name, x, y, platformID, orientationMode);
	g_videoModeNames.push_back(name);
}

void VideoModeSelector::initializeModes() {
	if (!g_videoModes.empty()) {
		return;
	}

	addVideoMode("Windows", 1024, 768, PLATFORM_ID_WINDOWS);
	addVideoMode("Windows Wide", 1280, 800, PLATFORM_ID_WINDOWS);

#if 0
// Just for reference in here, not tested
#ifdef _WIN32
	// get native window size
	HWND        hDesktopWnd = GetDesktopWindow();
	HDC         hDesktopDC = GetDC(hDesktopWnd);
	int nScreenX = GetDeviceCaps(hDesktopDC, HORZRES);
	int nScreenY = GetDeviceCaps(hDesktopDC, VERTRES);
	ReleaseDC(hDesktopWnd, hDesktopDC);
	addVideoMode("Windows Native", nScreenX, nScreenY, PLATFORM_ID_WINDOWS);
#endif
#endif

	//OSX
	addVideoMode("OSX", 1024,768, PLATFORM_ID_OSX); 
	addVideoMode("OSX Wide", 1280,800, PLATFORM_ID_OSX); 

	// Linux
	addVideoMode("Linux", 1024, 768, PLATFORM_ID_LINUX);
	addVideoMode("Linux Wide", 1280, 800, PLATFORM_ID_LINUX);
	
	//iOS - for testing, you should probably use the "Landscape" versions unless you want to hurt your neck
	addVideoMode("iPhone", 320, 480, PLATFORM_ID_IOS);
	addVideoMode("iPhone Landscape", 480, 320, PLATFORM_ID_IOS, ORIENTATION_PORTRAIT);
	addVideoMode("iPad", 768, 1024, PLATFORM_ID_IOS);
	addVideoMode("iPad Landscape", 1024, 768, PLATFORM_ID_IOS, ORIENTATION_PORTRAIT);
	addVideoMode("iPhone4", 640, 960, PLATFORM_ID_IOS, ORIENTATION_PORTRAIT);
	addVideoMode("iPhone4 Landscape", 960, 640, PLATFORM_ID_IOS);
	addVideoMode("iPad HD", 768*2, 1024*2, PLATFORM_ID_IOS);
	addVideoMode("iPhone5", 640, 1136, PLATFORM_ID_IOS, ORIENTATION_PORTRAIT);
	addVideoMode("iPhone5 Landscape", 1136, 640, PLATFORM_ID_IOS);
	
	//Palm er, I mean HP. These should use the Debug WebOS build config in MSVC for the best results, it will
	//use their funky SDL version
	addVideoMode("Pre", 320, 480, PLATFORM_ID_WEBOS);
	addVideoMode("Pre Landscape", 480, 320, PLATFORM_ID_WEBOS);
	addVideoMode("Pixi", 320, 400, PLATFORM_ID_WEBOS);
	addVideoMode("Pre 3", 480, 800, PLATFORM_ID_WEBOS);
	addVideoMode("Pre 3 Landscape", 800,480, PLATFORM_ID_WEBOS);
	addVideoMode("Touchpad", 768, 1024, PLATFORM_ID_WEBOS);
	addVideoMode("Touchpad Landscape", 1024, 768, PLATFORM_ID_WEBOS);

	//Android
	addVideoMode("G1", 320, 480, PLATFORM_ID_ANDROID);
	addVideoMode("G1 Landscape", 480, 320, PLATFORM_ID_ANDROID);
	addVideoMode("Nexus One", 480, 800, PLATFORM_ID_ANDROID);
	addVideoMode("Nexus One Landscape", 800, 480, PLATFORM_ID_ANDROID); 
	addVideoMode("Droid Landscape", 854, 480, PLATFORM_ID_ANDROID); 
	addVideoMode("Xoom Landscape", 1280,800, PLATFORM_ID_ANDROID);
	addVideoMode("Xoom", 800,1280, PLATFORM_ID_ANDROID);
	addVideoMode("Galaxy Tab 7.7 Landscape", 1024,600, PLATFORM_ID_ANDROID);
	addVideoMode("Galaxy Tab 10.1 Landscape", 1280,800, PLATFORM_ID_ANDROID);

	//RIM Playbook OS/BBX/BB10/Whatever they name it to next week
	addVideoMode("Playbook", 600,1024, PLATFORM_ID_BBX);
	addVideoMode("Playbook Landscape", 1024,600, PLATFORM_ID_BBX);
}

VideoModeSelector::VideoModeSelector()
{
	initializeModes();
}

const VideoMode* VideoModeSelector::getNamedMode(const string& name) const
{
	map<string, VideoMode>::const_iterator it(g_videoModes.find(name));
	
	if (it != g_videoModes.end()) {
		return &it->second;
	}
	
	return NULL;
}

const vector<string>& VideoModeSelector::getModeNames() const
{
	return g_videoModeNames;
}

string VideoModeSelector::modesAsString() const
{
	const string nameHeader("Name");
	const string sizeHeader("Width x height");
	const string platformHeader("Platform");
	const string orientationHeader("Orientation");

	const vector<string>& modeNames = getModeNames();

	int maxModeNameLength = nameHeader.size();
	for (vector<string>::const_iterator it(modeNames.begin()); it != modeNames.end(); it++) {
		if ((*it).size() > maxModeNameLength) {
			maxModeNameLength = (*it).size();
		}
	}

	int maxSizeStringLength = sizeHeader.size();
	int maxPlatformStringLength = platformHeader.size();

	const int fieldPadding = 4;

	string ret("Available video modes:\n\n");

	char tmpStr[1024];

	snprintf(tmpStr, sizeof(tmpStr), "%*s%*s%*s%s",
		   -(maxModeNameLength + fieldPadding), nameHeader.c_str(),
		   -(maxSizeStringLength + fieldPadding), sizeHeader.c_str(),
		   -(maxPlatformStringLength + fieldPadding), platformHeader.c_str(),
		   orientationHeader.c_str());
	ret += tmpStr;
	ret += "\n";

	for (vector<string>::const_iterator it(modeNames.begin()); it != modeNames.end(); it++) {
		const VideoMode *mode = getNamedMode(*it);
		const string sizeStr = toString(mode->getX()) + " x " + toString(mode->getY());

		snprintf(tmpStr, sizeof(tmpStr), "%*s%*s%*s%s",
			   -(maxModeNameLength + fieldPadding), it->c_str(),
			   -(maxSizeStringLength + fieldPadding), sizeStr.c_str(),
			   -(maxPlatformStringLength + fieldPadding), PlatformIDAsStringDisplay(mode->getPlatformID()).c_str(),
			   OrientationAsStringDisplay(mode->getOrientationMode()).c_str());
		ret += "\n";
		ret += tmpStr;
	}

	return ret;
}
