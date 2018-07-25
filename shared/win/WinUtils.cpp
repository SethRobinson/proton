#if defined(WIN32) && !defined(RT_WEBOS)

#include "WinUtils.h"
#include "PlatformSetupWin.h"
#include <cstdlib>
#include <cstdarg>
#include <cassert>
#include <memory.h>
#include <cstdio>
#include <windows.h>
#include <ctime>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include "errno.h"
#include "util/MiscUtils.h"
#include <shlobj.h>
 
using namespace std;

void StringReplace(const std::string& what, const std::string& with, std::string& in);
vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter );

string g_appCachePath;

class FastTimerDX
{
public:

	FastTimerDX(int a);

	uint32 GetTicks() 
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&end);

		/*
//uncoment to fake timing errors
#if defined(_DEBUG) && defined (WIN32)
		if (GetAsyncKeyState('F'))
		{

			float r = (float)Random(100)*100000000.0f;

			LogMsg(string("Modified game timer by "+toString(r)).c_str());
			return r;
		}

#endif
		*/

		diff = ((end - start) * 1000) / freq;
		return (unsigned int)(diff & 0xffffffff);
	}

private:
	__int64 freq;
	__int64 start;
	__int64 end;
	__int64 diff;
};


FastTimerDX::FastTimerDX(int a)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
}


FastTimerDX g_fastTimer(0);

string GetBaseAppPath()
{

	// Get path to executable:
	TCHAR szDllName[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFilename[256];
	TCHAR szExt[256];
	GetModuleFileName(0, szDllName, _MAX_PATH);
	_splitpath(szDllName, szDrive, szDir, szFilename, szExt);

	return string(szDrive) + string(szDir); 
}

const char * GetAppName();

string GetSavePath()
{
#ifdef RT_WIN_USE_APPDATA_SAVE_PATH
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path ) == S_OK)
	{
		return string(path)+"\\"+GetAppName()+"\\";
	}
#endif
	
	return "";
}

string GetAppCachePath()
{
	return g_appCachePath;
}
void SetAppCachePath(std::string path)
{
	g_appCachePath = path;
}

void FireAchievement(std::string achievement)
{
}

void LaunchURL(string url)
{
	//LogMsg("Launching %s", url.c_str());
	//weird double cast to get away from MSVC warning
	int result = (int)(LONG_PTR)ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);

	if ( (result < 32) && (result != 2))
	{
		//big fat error.

		std::string s;
		s = std::string("Windows doesn't know how to open ")+url+ 
			std::string("\n\nYou need to use file explorer and associate this file type with something first.");

		MessageBox(GetForegroundWindow(), s.c_str(), url.c_str(), MB_ICONSTOP);
	}

}

float GetDeviceOSVersion()
{
	//TODO
	if (GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
	{
		//pretend we're a touchpad
		return 3.05f;
	}
	return 0.0f;
}

std::string GetMacAddress()
{
	//TODO
#ifdef _DEBUG
//	return "02:00:00:00:00:00"; //fake iOS's invalid mac address of iOS 7
#endif
	return ""; //unimplemented
}

void SetClipboardText(std::string text)
{
	if(OpenClipboard(NULL))
	{
		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, text.length()+1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, text.c_str());
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT,clipbuffer);
		CloseClipboard();
	}

}


string GetClipboardText()
{
	string text;

	if (OpenClipboard(NULL))
	{
		HANDLE hData = GetClipboardData(CF_TEXT);  //note, if we every support unicode we would use CF_UNICODE
		if (hData)
		{
			text = (char*)GlobalLock(hData);
			GlobalUnlock(hData);
		}

		CloseClipboard();
	}
	return text;
	
}

bool IsIPhone3GS()
{
	return false;
}

ePlatformID GetPlatformID()
{
	return PLATFORM_ID_WINDOWS;
}

bool IsDesktop()
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS || GetEmulatedPlatformID() == PLATFORM_ID_OSX
		|| GetEmulatedPlatformID() == PLATFORM_ID_LINUX) return true;
	return false;
}

bool IsIphoneOriPad()
{
	return false;
}

bool IsIphone()
{
	//return false; //act like an iTouch, useful for knowing when not to show stuff about vibration and camera
	if (IsIPAD() || IsIphone4()) return false;

	return true;  //act like an iPhone
}

bool IsIphone4()
{
	//return false; //act like an iTouch, useful for knowing when not to show stuff about vibration and camera
	if (GetPrimaryGLX() == 960 || GetPrimaryGLY() == 960) return true;
	return false; 
}

bool IsIPAD()
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
	{
		if (GetPrimaryGLX() == 1024 || GetPrimaryGLY() == 1024) return true;
	}

	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
	return C_DEVICE_MEMORY_CLASS_4;
}

unsigned int GetSystemTimeTick()
{
	return GetTickCount();
}
double GetSystemTimeAccurate()
{
	//return double(GetTickCount());
	return g_fastTimer.GetTicks();
}

unsigned int GetFreeMemory()
{
	return 0; //don't care on the PC
}

string g_string;

void SetLastStringInput( string s )
{
	g_string = s;
}

std::string GetLastStringInput()
{
	return g_string;
}

eNetworkType IsNetReachable( string url )
{
	return C_NETWORK_WIFI;
}

int GetSystemData()
{
	return C_PIRATED_YES;
}

void RemoveFile( string fileName, bool bAddSavePath)
{
	int ret;

	if (bAddSavePath)
	{
		ret = _unlink( (GetSavePath()+fileName).c_str());
	} else
	{
		ret =_unlink( (fileName).c_str());
	}
	
	if (ret == -1)
	{
		switch (errno)
		{

		case EACCES:
			LogMsg("%s is read only", fileName.c_str());
			break;

		case ENOENT:
#ifdef _DEBUG
			LogMsg("%s not found to delete", fileName.c_str());
#endif
			break;

		}


	}
}

string GetDeviceID()
{
	//half hearted attempt at getting a unique identifier.  Don't count on it, though.
	DWORD dwDiskSerial;
	if (!GetVolumeInformation(_T("C:\\"), NULL, 0, &dwDiskSerial, NULL, NULL, NULL, NULL))
	{
		if (!GetVolumeInformation(_T("D:\\"), NULL, 0, &dwDiskSerial, NULL, NULL, NULL, NULL))
		{
			if (!GetVolumeInformation(_T("E:\\"), NULL, 0, &dwDiskSerial, NULL, NULL, NULL, NULL))
			{
				if (!GetVolumeInformation(_T("F:\\"), NULL, 0, &dwDiskSerial, NULL, NULL, NULL, NULL))
				{
					if (!GetVolumeInformation(_T("G:\\"), NULL, 0, &dwDiskSerial, NULL, NULL, NULL, NULL))
					{
						return "";
					}
				}
			}
		}
	}

	char stTemp[128];
	sprintf(stTemp, "%u",dwDiskSerial);

	return stTemp;

}

bool IsAppInstalled(string packageName)
{
#ifdef _DEBUG
	LogMsg("IsAppInstalled not yet handled for this OS");
#endif
	return false;
}

string GetRegionString()
{
	char  countryCode[12];
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SISO3166CTRYNAME,countryCode,12);

	char  languageCode[12];
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SISO639LANGNAME,languageCode,12);

	string s = ToLowerCaseString(string(languageCode)+"_"+countryCode);
	if (s.size() == 5) return s;
	
	
	//um... something is wrong.  Return default
	return "en_us";
}


//month is 1-12 btw
int GetDaysSinceDate(int month,int day, int year)
{
	time_t ltime;
	time( &ltime );

	tm expire = { 0, 0, 0, day, month-1, year-1900, 0 };	 //Month is 0-11 btw
	tm today = *localtime( &ltime );

	long time_now = (long)today.tm_mday + (long)today.tm_mon * 30 + today.tm_year*365;
	long time_exp = (long)expire.tm_mday +(long)expire.tm_mon * 30 + expire.tm_year * 365;
	long time_passed = time_now - time_exp;

	//now let's convert it back to days
	if (time_passed == 0) return 0; //avoid divide by 0
	return time_passed;
}

string GetDateAndTimeAsString()
{
	time_t ltime;
	time( &ltime );

	tm today = *localtime( &ltime );

	char stTemp[128];
	sprintf(stTemp, "%d/%d %d:%d:%d", today.tm_mday, today.tm_mon+1, today.tm_hour, today.tm_min, today.tm_sec);
	return string(stTemp);
}

void GetDateAndTime(int *monthOut, int *dayOut, int *yearOut, int *hourOut, int *minOut, int *secOut)
{
	time_t ltime;
	time( &ltime );
	tm today = *localtime( &ltime );

	*monthOut = today.tm_mon+1;
	*dayOut = today.tm_mday;
	*yearOut = today.tm_year+1900;
	*hourOut = today.tm_hour;
	*minOut = today.tm_min;
	*secOut = today.tm_sec;
}

bool CheckDay(const int year, const int month, const int day)
{
	int nowyear, nowmonth, nowday, nowhour, nowmin, nowsec;
	GetDateAndTime(&nowmonth, &nowday, &nowyear, &nowhour, &nowmin, &nowsec);
	LogMsg("Comparing against date year %d, month %d, day %d", nowyear, nowmonth, nowday);
	if ((nowday == day) && (nowmonth == month) && (nowyear == year))
	{
		return true;
	}
	return false;
}

bool LaterThanNow(const int year, const int month, const int day)
{
	int nowyear, nowmonth, nowday, nowhour, nowmin, nowsec;
	GetDateAndTime(&nowmonth, &nowday, &nowyear, &nowhour, &nowmin, &nowsec);
	LogMsg("Comparing against date year %d, month %d, day %d", nowyear, nowmonth, nowday);
	if (nowyear < year )
	{
		return false;
	}
	if (nowyear > year )
	{
		return true;
	}
	// year must be equal
	if (nowmonth < month )
	{
		return false;
	}
	if (nowmonth > month )
	{
		return true;
	}
	// month must be equal
	if (nowday < day )
	{
		return false;
	}
	if (nowday > day )
	{
		return true;
	}
	return false;
}

bool RTCreateDirectory(const std::string &dir_name)
{
	if (dir_name.empty())
		return false;

	// this will be a full path
	std::string full_path; 	// calculate the full path

#ifdef WIN32
	DWORD buff_len = ::GetFullPathName(dir_name.c_str(), 0, 0, 0);

	if (buff_len == 0)
		// can't calculate, return bad status
		return false;
	else
	{
		char * buffer = new char[buff_len + 1];
		char * buffer_ptr_to_filename = 0;
		// Obtaining full path
		buff_len = ::GetFullPathName(dir_name.c_str(), buff_len, buffer, &buffer_ptr_to_filename);
		if (buff_len == 0)
		{
			delete[] buffer;			
			// can't obtain full path, return bad status
			return false;
		}

		// ok, save it
		full_path = buffer;
		delete[] buffer;			
	}
#else
	// TODO: add here Linux version of GetFullPathName
	full_path = dir_name;
#endif

#ifdef WIN32
	return ::CreateDirectory(full_path.c_str(), NULL) != 0;
#else
	return ::mkdir(full_path.c_str(), 0755) == 0;
#endif
}

void CreateDirectoryRecursively(string basePath, string path)
{
	StringReplace("\\", "/", path);
	StringReplace("\\", "/", basePath);

	vector<string> tok = StringTokenize(path, "/");

	if (!basePath.empty())
	{
		if (basePath[basePath.size()-1] != '/') basePath += "/";
	}
	path = "";
	for (unsigned int i=0; i < tok.size(); i++)
	{
		path += tok[i].c_str();
		//LogMsg("Creating %s%s", basePath.c_str(), path.c_str());
		RTCreateDirectory(basePath+path);
		path += "/";
	}
}


BOOL IsDots(const TCHAR* str) {
	if(_tcscmp(str,".") && _tcscmp(str,"..")) return FALSE;
	return TRUE;
}

vector<string> GetDirectoriesAtPath(string path)
{
	if (path.empty()) path += "."; //needed for relative paths

	vector<string> v;
	HANDLE hFind;    // file handle
	WIN32_FIND_DATA FindFileData;

	hFind = FindFirstFile((path+"\\*").c_str(),&FindFileData);
	while (FindNextFile(hFind,&FindFileData))
	{
		if(IsDots(FindFileData.cFileName)) continue;

		if((FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY))
		{
			v.push_back(string(FindFileData.cFileName));
		}
	}
	
	FindClose(hFind);
	return v;
}



vector<string> GetFilesAtPath(string path)
{
	if (path.empty()) path += "."; //needed for relative paths

	vector<string> v;
	HANDLE hFind;    // file handle
	WIN32_FIND_DATA FindFileData;

	hFind = FindFirstFile((path+"\\*").c_str(),&FindFileData);
	while (FindNextFile(hFind,&FindFileData))
	{
		if(IsDots(FindFileData.cFileName)) continue;

		if( !(FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY))
		{
			v.push_back(string(FindFileData.cFileName));
		}
	}

	FindClose(hFind);
	return v;
}

//based on a snippet fromFeroz Zahid (http://www.codeguru.com/cpp/w-p/files/folderdirectorymaintenance/article.php/c8999/
bool RemoveDirectoryRecursively(string path)
{
	const TCHAR* sPath = path.c_str();
	HANDLE hFind;    // file handle
	WIN32_FIND_DATA FindFileData;

	TCHAR DirPath[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	_tcscpy(DirPath,sPath);
	_tcscat(DirPath,"\\*");    // searching all files
	_tcscpy(FileName,sPath);
	_tcscat(FileName,"\\");

	// find the first file
	
	//SETH: This is actually wrong as it's ignoring the first file.  But since it's always "." which means the current dir it doesn't
	//matter, right?

	hFind = FindFirstFile(DirPath,&FindFileData);
	if(hFind == INVALID_HANDLE_VALUE) return FALSE;
	_tcscpy(DirPath,FileName);

	bool bSearch = true;
	while(bSearch) {    // until we find an entry
		if(FindNextFile(hFind,&FindFileData)) {
			if(IsDots(FindFileData.cFileName)) continue;
			_tcscat(FileName,FindFileData.cFileName);
			if((FindFileData.dwFileAttributes &
				FILE_ATTRIBUTE_DIRECTORY)) {

					// we have found a directory, recurse
					if(!RemoveDirectoryRecursively(FileName)) {
						FindClose(hFind);
						return FALSE;    // directory couldn't be deleted
					}
					// remove the empty directory
					RemoveDirectory(FileName);
					_tcscpy(FileName,DirPath);
			}
			else {
				if(FindFileData.dwFileAttributes &
					FILE_ATTRIBUTE_READONLY)
					// change read-only file mode
					_chmod(FileName, _S_IWRITE);
				if(!DeleteFile(FileName)) {    // delete the file
					FindClose(hFind);
					return FALSE;
				}
				_tcscpy(FileName,DirPath);
			}
		}
		else {
			// no more files there
			if(GetLastError() == ERROR_NO_MORE_FILES)
				bSearch = false;
			else {
				// some error occurred; close the handle and return FALSE
				FindClose(hFind);
				return FALSE;
			}

		}

	}
	FindClose(hFind);                  // close the file handle

	return RemoveDirectory(sPath) != 0;     // remove the empty directory

}


bool CheckIfOtherAudioIsPlaying()
{
	return false;
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{

}

bool HasVibration()
{
	switch (GetEmulatedPlatformID())
	{
	case PLATFORM_ID_IOS:
	case PLATFORM_ID_ANDROID:
		return true;
	}

	return false;
}


string GetAdvertisingIdentifier()
{

#ifndef _DEBUG
return ""; //not valid on windows
#endif

return "fa39ed5c-0c5d-4de2-96d7-ebf40f2d65e1"; //for testing, it's an ANDROID gid
}

string GetNetworkType()
{
	return "none"; //not supported for this OS
}

void CreateAppCacheDirIfNeeded()
{
	//only applicable to iOS
}

bool GetLastWriteDateOfFile(int *monthOut, int *dayOut, int *yearOut, int *hourOut, int *minOut, int *secOut, std::string fileName, bool bAddSavePath)
{
	if (bAddSavePath)
	{
		fileName = GetSavePath()+fileName;
	}

	WIN32_FILE_ATTRIBUTE_DATA  wfad;
	if (!GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &wfad))
	{
		return false;
	}
	
	SYSTEMTIME stime;
	if (!FileTimeToSystemTime(&wfad.ftLastWriteTime, &stime))
	{
		return false;
	}

	*monthOut = stime.wMonth;
	*dayOut = stime.wDay;
	*yearOut = stime.wYear;
	*hourOut = stime.wHour;
	*minOut = stime.wMinute;
	*secOut = stime.wSecond;

	return true;
}


int GetTouchesReceived()
{
	return 0; //uh, not accurate
}
bool IsStillLoadingPersistentData()
{
	return false;
}
bool IsStillSavingPersistentData()
{
	return false;
}
void SyncPersistentData()
{
}

#endif