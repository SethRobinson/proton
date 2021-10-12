#include "VitaUtils.h"
#include "PlatformSetup.h"
#include "BaseApp.h"

#include <psp2/kernel/sysmem.h>
#include <psp2/rtc.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/apputil.h>
#include <psp2/vshbridge.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/apputil.h>

using namespace std;

ePlatformID GetPlatformID()
{
    return PLATFORM_ID_PSVITA;
}

const char * GetAppName();
std::string GetBaseAppPath()
{
    return "ux0:/data/proton/"; //for now hardcore values because we really cannot write to the game directory...
}

std::string GetSavePath()
{
    return GetBaseAppPath() + string(GetAppName()) + "/";
}

string g_CachePath;

std::string GetAppCachePath()
{
    if(g_CachePath != "")
        return g_CachePath;
    return ""; //no cache.
}

void LogMsg(const char *traceStr, ...)
{
    va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );

	//AppendStringToFile(GetBaseAppPath() + "log.txt", GetDateAndTimeAsString() + ": " + string(buffer) + "\n");
    string wtf = string(buffer) + "\n";

	int f = sceIoOpen("ux0:/data/logs.txt", SCE_O_WRONLY | SCE_O_APPEND | SCE_O_CREAT, 0777);
	sceIoWrite(f, wtf.data(), strlen(wtf.data()));
	sceIoClose(f);

    if (IsBaseAppInitted())
        GetBaseApp()->GetConsole()->AddLine(buffer);
}

void SetAppCachePath(std::string path)
{
    g_CachePath = path;
}

void CreateAppCacheDirIfNeeded()
{
    if(g_CachePath == "")
    {
        g_CachePath = "ux0:/data/proton/cache";
    }
}

bool LaterThanNow(const int year, const int month, const int day)
{
    struct SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);

    if(time.year < year && time.month < month && time.day < day)
        return true;
    return false;
}

bool CheckDay(const int year, const int month, const int day)
{
    struct SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);

    if(time.year == year && time.month == month && time.day == day)
        return true;
    return false;
}

unsigned int GetSystemTimeTick()
{
    static unsigned int incrementingTimer = 0;
	static double buildUp = 0;
	static double lastTime = 0;

    struct SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	double timeDouble = time.second * 1000 + (time.microsecond * 1000) / 1000000;
	
	double change = timeDouble -lastTime;
	if (change > 0 && change < (1000*120) )
	{
		incrementingTimer += change;
	}
	lastTime = timeDouble;

	return incrementingTimer;
}

double GetSystemTimeAccurate()
{
	return GetSystemTimeTick();
}

unsigned int GetFreeMemory()
{
    return 0; //return current memory used in the "user" space.
}

void LaunchURL(std::string url)
{
    //sceAppUtilLaunchWebBrowser
}

void FireAchievement(std::string achievement)
{
    //TODO!!! ( idk what it is used for tbh )
}

string g_string; //i dont know what this is used for but i will add it anyways.

void SetLastStringInput(std::string s)
{
    g_string = s;
}

std::string GetLastStringInput()
{
    return g_string;
}

bool GetLastWriteDateOfFile(int *monthOut, int *dayOut, int *yearOut, int *hourOut, int *minOut, int *secOut, std::string fileName, bool bAddSavePath)
{
    return false; //TODO!!
}

void RemoveFile(std::string fileName, bool bAddSavePath)
{
    if (bAddSavePath)
        fileName = GetSavePath() + fileName;
    
    if(sceIoRemove(fileName.c_str()) < 0)
    {
        LogMsg("RemoveFile(): Failed to remove file!!!");
    }
}

void CreateDirectoryRecursively(std::string basePath, std::string path)
{
    string directory = basePath + path;
    if(sceIoMkdir(directory.c_str(), 0777) != 0) //TODO: 0777 is dangerous use proper file permissions!
    {
        LogMsg("CreateDirectoryRecursively(): Failed to create directory!!!");
    }
}

bool RemoveDirectoryRecursively(std::string path)
{
    if(sceIoRmdir(path.c_str()) != 0)
    {
        LogMsg("CreateDirectoryRecursively(): Failed to create directory!!!");
        return false;
    }
    return true; //i guess this mean it was sucess.
}

std::vector<std::string> GetDirectoriesAtPath(std::string path)
{
    std::vector<std::string> v;

    SceUID fd = sceIoDopen(path.c_str());
    if(fd >= 0)
    {
        int result = 0;
        do {

            SceIoDirent dir;
            memset(&dir, 0, sizeof(SceIoDirent));

            result = sceIoDread(fd, &dir);

            if ( result > 0 )
            {
                if (SCE_S_ISDIR(dir.d_stat.st_mode))
                {
                    LogMsg("Dir is %d", dir.d_name);
                }
            }

        } while ( result > 0 );
        sceIoDclose(fd);
    }

    return v;
}

std::vector<std::string> GetFilesAtPath(std::string path)
{
    std::vector<std::string> v;

    SceUID fd = sceIoDopen(path.c_str());
    if(fd >= 0)
    {
        int result = 0;
        do {

            SceIoDirent dir;
            memset(&dir, 0, sizeof(SceIoDirent));

            result = sceIoDread(fd, &dir);

            if ( result > 0 )
            {
                LogMsg("Dir is %d", dir.d_name);
            }

        } while ( result > 0 );
        sceIoDclose(fd);
    }

    return v;
}

std::string GetRegionString()
{
    //TODO: use SceSystemParamLang
    return "en_us";
}

bool IsIphone()
{
    return false;
}

bool IsIPAD()
{
    return false;
}

bool IsIphoneOriPad()
{
    return false;
}

bool IsIPhone3GS()
{
    return false;
}

bool IsIphone4()
{
    return false;
}

int GetSystemData()
{
    return C_PIRATED_NO; //we are running it on modified system there is no reason to check for pirating.
}

eNetworkType IsNetReachable(std::string url)
{
    return C_NETWORK_NONE; //TODO
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
    return C_DEVICE_MEMORY_CLASS_3;   
}

bool IsIPodTouchThirdGen()
{
    return false;
}

float GetDeviceOSVersion()
{
    return 0.0f; //unimplemented
}

std::string GetMacAddress()
{
    return ""; //unimplemented
}

bool IsDesktop()
{
    if(GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS) return true;
    return false;
}

bool HasVibration()
{
    return true;
}

std::string GetDeviceID()
{
    return "TODO";
}

void SystemSleep(int sleepMS)
{
    sceKernelDelayThread(sleepMS);
}

std::string GetDateAndTimeAsString()
{
    struct SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);

	char stTemp[128];
	sprintf(stTemp, "%d/%d %d:%d:%d", time.day, time.month, time.hour, time.minute, time.second);
	return string(stTemp);
}

void GetDateAndTime(int *monthOut, int *dayOut, int *yearOut, int *hourOut, int *minOut, int *secOut)
{
    struct SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);

	*monthOut = time.month;
	*dayOut = time.day;
	*yearOut = time.year;
	*hourOut = time.hour;
	*minOut = time.minute;
	*secOut = time.second;
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{

}

string GetNetworkType()
{
	return "none"; // not supported for this OS
}