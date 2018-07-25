#include "PlatformPrecomp.h"

bool g_forceAspectRatioWhenChangingWindowSize = true; //applicable to manually dragging corners/sides.  Can hold Shift while dragging to toggle.

bool GetForceAspectRatioWhenResizing()
{
	return g_forceAspectRatioWhenChangingWindowSize;
}
ePlatformID g_emulatedPlatformID = PLATFORM_ID_UNKNOWN;
eMarketID g_marketID = MARKET_ID_UNSPECIFIED;

//usually the same as PlatFormID, unless we're using one OS to test another OS's stuff
ePlatformID GetEmulatedPlatformID()
{
	if (g_emulatedPlatformID == PLATFORM_ID_UNKNOWN)
	{
		//assume no emulation, give them the real platform ID
		return GetPlatformID();
	}

	return g_emulatedPlatformID;
}

string AddPlatformNameURL()
{
	if (GetEmulatedPlatformID() != PLATFORM_ID_WINDOWS)
	{
		return "/"+GetPlatformName();
	}
	return "";
}

//these cannot be changed
string PlatformIDAsString(ePlatformID platformID)
{

	switch (platformID)
	{
	case PLATFORM_ID_UNKNOWN: return "unknown";
	case PLATFORM_ID_WINDOWS: return "windows";
	case PLATFORM_ID_IOS: return "iphone";
	case PLATFORM_ID_OSX: return "osx";
	case PLATFORM_ID_LINUX: return "linux";
	case PLATFORM_ID_ANDROID: return "android";
	case PLATFORM_ID_WINDOWS_MOBILE: return "winmo";
	case PLATFORM_ID_WEBOS: return "webos";
	case PLATFORM_ID_BBX: return "bbx";
        default:;

	}

	return "";
}

//these cannot be changed
ePlatformID PlatformStringAsID(string platform)
{

	if (platform == "iphone") return PLATFORM_ID_IOS;
	if (platform == "android") return PLATFORM_ID_ANDROID;
	if (platform == "windows") return PLATFORM_ID_WINDOWS;
	if (platform == "osx") return PLATFORM_ID_OSX;
	if (platform == "winmo") return PLATFORM_ID_WINDOWS_MOBILE;
	if (platform == "webos") return PLATFORM_ID_WEBOS;
	if (platform == "bbx") return PLATFORM_ID_BBX;
	if (platform == "linux") return PLATFORM_ID_LINUX;

	return PLATFORM_ID_UNKNOWN;
}

string PlatformIDAsStringDisplay(ePlatformID platformID)
{

	switch (platformID)
	{
	case PLATFORM_ID_UNKNOWN: return "???";
	case PLATFORM_ID_WINDOWS: return "Windows";
	case PLATFORM_ID_IOS: return "iOS";
	case PLATFORM_ID_OSX: return "OSX";
	case PLATFORM_ID_LINUX: return "Linux";
	case PLATFORM_ID_ANDROID: return "Android";
	case PLATFORM_ID_WINDOWS_MOBILE: return "WinMo";
	case PLATFORM_ID_WEBOS: return "WebOS";
	case PLATFORM_ID_BBX: return "BBX";
        default:;
	}
	return "";
}

string OrientationAsStringDisplay(eOrientationMode orientation)
{
	switch (orientation)
	{
	case ORIENTATION_DONT_CARE: return "Not specified";
	case ORIENTATION_PORTRAIT: return "Portrait";
	case ORIENTATION_PORTRAIT_UPSIDE_DOWN: return "Portrait upside down";
	case ORIENTATION_LANDSCAPE_LEFT: return "Landscape left";
	case ORIENTATION_LANDSCAPE_RIGHT: return "Landscape right";
	default: return "<invalid value>";
	}
}

string GetPlatformName()
{
	return PlatformIDAsString(GetEmulatedPlatformID());
}


void SetMarketID(eMarketID marketID)
{
	g_marketID = marketID;
}

eMarketID GetMarketID()
{
	return g_marketID;
}

void SetEmulatedPlatformID(ePlatformID platformID)
{
	g_emulatedPlatformID = platformID;
}

#ifdef _CONSOLE
float GetScreenSizeXf() { assert(0); return 0;}
float GetScreenSizeYf() { assert(0); return 0;}
int GetScreenSizeX() { assert(0); return 0;}
int GetScreenSizeY() { assert(0); return 0;}

#endif
