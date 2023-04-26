#include "psp2Utils.h"
#include "PlatformSetup.h"
#include "BaseApp.h"

#include <map>
#include <string>
#include <vector>
#include <cstring>

#include <psp2/io/dirent.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/rtc.h>
#include <psp2/system_param.h>
#include <psp2/apputil.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>

#include "util/MiscUtils.h"

ePlatformID GetPlatformID()
{
    return PLATFORM_ID_PSVITA;  // PSP2
}

const char * GetAppName();
std::string GetBaseAppPath()
{
    return "ux0:data/" + std::string(GetAppName()) + "/";
}

std::string GetSavePath()
{
    return GetBaseAppPath();
}

std::string g_CachePath;

std::string GetAppCachePath()
{
    std::string path;

    if (g_CachePath != "")
        path = g_CachePath;
    else
        path = GetBaseAppPath() + "cache";
        
    return path;
}

void SetAppCachePath(std::string path)
{
    g_CachePath = path;
}

void CreateAppCacheDirIfNeeded()
{
    std::string path;

    if (g_CachePath != "")
        path = g_CachePath;
    else
        path = GetBaseAppPath() + "cache";

    sceIoMkdir(path.c_str(), 0777);
}

void LogMsg(const char *traceStr, ...)
{
    va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ((void*)buffer, 0, logSize);

	va_start(argsVA, traceStr);
	vsnprintf(buffer, logSize, traceStr, argsVA);
	va_end(argsVA);

    std::string data = std::string(buffer) + "\n";
#ifdef _DEBUG
    sceClibPrintf(data.c_str());
#endif

	int fd = sceIoOpen((GetBaseAppPath() + "logs.txt").c_str(), SCE_O_WRONLY | SCE_O_APPEND | SCE_O_CREAT, 0777);
	sceIoWrite(fd, data.c_str(), data.length());
	sceIoClose(fd);

    if (IsBaseAppInitted())
        GetBaseApp()->GetConsole()->AddLine(buffer);
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
	if (change > 0 && change < (1000 * 120) )
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
    return 0;
}

void LaunchURL(std::string url) {}

void FireAchievement(std::string achievement) {}

std::string g_string;

void SetLastStringInput(std::string s)
{
    g_string = s;
}

std::string GetLastStringInput()
{
    return g_string;
}

bool GetLastWriteDateOf (int *monthOut, int *dayOut, int *yearOut, int *hourOut, int *minOut, int *secOut, std::string fileName, bool bAddSavePath) { return false; }

void RemoveFile(std::string fileName, bool bAddSavePath)
{
    if (bAddSavePath)
        fileName = GetSavePath() + fileName;
    
    sceIoRemove(fileName.c_str());
}

void CreateDirectoryRecursively(std::string basePath, std::string path)
{
    std::vector<std::string> token = StringTokenize(path, "/");

	if (!basePath.empty())
	{
		if (basePath[basePath.size() - 1] != '/')
        {
            basePath += "/";
        }
	}

	path = "";
	for (unsigned int i = 0; i < token.size(); i++)
	{
		path += token[i].c_str();
        std::string directory = basePath + path;
        sceIoMkdir(directory.c_str(), 0777);
		path += "/";
	}
}

bool RemoveDirectoryRecursively(std::string path)
{
    if (sceIoRmdir(path.c_str()) != 0)
        return false;
    return true;
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
                    v.push_back(std::string(dir.d_name));
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
                if (!SCE_S_ISDIR(dir.d_stat.st_mode))
                {
                    v.push_back(std::string(dir.d_name));
                }
            }

        } while ( result > 0 );
        sceIoDclose(fd);
    }

    return v;
}

std::string GetRegionString()
{
    int langId;
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &langId);

    std::map<SceSystemParamLang, std::string> languages = {
        { SCE_SYSTEM_PARAM_LANG_JAPANESE, "ja_JP" },
        { SCE_SYSTEM_PARAM_LANG_ENGLISH_US, "en_US" },
        { SCE_SYSTEM_PARAM_LANG_FRENCH, "fr_FR" },
        { SCE_SYSTEM_PARAM_LANG_SPANISH, "es_ES" },
        { SCE_SYSTEM_PARAM_LANG_GERMAN, "de_DE" },
        { SCE_SYSTEM_PARAM_LANG_ITALIAN, "it_IT" },
        { SCE_SYSTEM_PARAM_LANG_DUTCH, "nl_NL" },
        { SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT, "pt_PT" },
        { SCE_SYSTEM_PARAM_LANG_RUSSIAN, "ru_RU" },
        { SCE_SYSTEM_PARAM_LANG_KOREAN, "ko_KR" },
        { SCE_SYSTEM_PARAM_LANG_CHINESE_T, "zh_CN" },
        { SCE_SYSTEM_PARAM_LANG_CHINESE_S, "zh_CN" },
        { SCE_SYSTEM_PARAM_LANG_FINNISH, "fi_FI" },
        { SCE_SYSTEM_PARAM_LANG_SWEDISH, "se_SE" },
        { SCE_SYSTEM_PARAM_LANG_DANISH, "da_DK" },
        { SCE_SYSTEM_PARAM_LANG_NORWEGIAN, "no_NO" },
        { SCE_SYSTEM_PARAM_LANG_POLISH, "pl_PL" },
        { SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR, "pt_BR" },
        { SCE_SYSTEM_PARAM_LANG_ENGLISH_GB, "en_GB" },
        { SCE_SYSTEM_PARAM_LANG_TURKISH, "tr_TR" }
    };

    return languages[(SceSystemParamLang)langId];
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
    return C_PIRATED_NO;
}

eNetworkType IsNetReachable(std::string url)
{
    return C_NETWORK_NONE;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
    return C_DEVICE_MEMORY_CLASS_2;   
}

bool IsIPodTouchThirdGen()
{
    return false;
}

float GetDeviceOSVersion()
{
    return 0.0f; // unimplemented
}

std::string GetMacAddress()
{
    return ""; // unimplemented
}

bool IsDesktop()
{
    if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS)
        return true;
    return false;
}

bool HasVibration()
{
    return true;    // Yep, if I implement it.
}

std::string GetDeviceID()
{
    return "PSP2-ID-0000";
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
	return std::string(stTemp);
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

void NotifyOSOfOrientationPreference(eOrientationMode orientation) {}

std::string GetNetworkType()
{
	return "none";
}

bool IsStillLoadingPersistentData()
{
	return false;
}
bool IsStillSavingPersistentData()
{
	return false;
}

void SyncPersistentData() {}

std::string GetClipboardText()
{
    return "";
}