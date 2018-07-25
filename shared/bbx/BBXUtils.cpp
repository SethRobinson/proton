

#include <errno.h>

using namespace std;

#include "PlatformSetupBBX.h"

#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/locale.h>
#include <bps/event.h>
#include <dirent.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstdarg>
#include <cassert>
#include <sys/types.h>

#include <time.h>

void StringReplace(const std::string& what, const std::string& with, std::string& in);
vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter );

string GetBaseAppPath()
{

	return "app/native/";
	/*
	char szDir[1024];
	getcwd( szDir, 1024);
	return string(szDir)+"/";
	*/

}

void CreateAppCacheDirIfNeeded()
{
	//only applicable to iOS
}


void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );
	printf(buffer);
	printf("\n");
	fflush(stdout);
}



string GetSavePath()
{
	return "./data/";
}

string GetAppCachePath()
{
	return "./data/";
}

void LaunchEmail(string subject, string content)
{
	LogMsg("no way to send email from BBX NDK?!");
}

void LaunchURL(string url)
{
	LogMsg("Launching %s", url.c_str());
	char *pError;
	navigator_invoke(url.c_str(), &pError);
}

string GetClipboardText()
{
	return "";
}

float GetDeviceOSVersion()
{
	//TODO
	return 0.0f;
}

string GetMacAddress()
{
	//TODO
	return ""; //unimplemented
}

bool IsIPhone3GS()
{
	return false;
}

bool IsDesktop()
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_BBX) return false;
	return true;
}

ePlatformID GetPlatformID()
{
	return PLATFORM_ID_BBX;
}


bool IsIphoneOriPad()
{
	return false;
}

bool IsIphone()
{
	//return false; //act like an iTouch, useful for knowing when not to show stuff about vibration and camera
	return false;
}

bool IsIphone4()
{
	return false; 
}

bool IsIPAD()
{
	//if (GetPrimaryGLX() == 1024 || GetPrimaryGLY() == 1024) return true;
	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
	return C_DEVICE_MEMORY_CLASS_4;
}


// convert the timespec into milliseconds //thanks, cocos2d-x
long time2millis(struct timespec *times)
{
    return times->tv_sec*1000 + times->tv_nsec/1000000;
}


unsigned int GetSystemTimeTick()
{
	/*

	 //yeah, this works great in the simulator, too bad it's all goofy ass on the real device
	struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_usec/1000 + tv.tv_sec*1000;
	*/

	static timespec lastTime;
	static bool bRanFirstTime = false;
	static unsigned int t = 0;

	if (!bRanFirstTime)
	{
		clock_gettime(CLOCK_REALTIME, &lastTime);
		bRanFirstTime = true;
	}

	timespec curTime;
	clock_gettime(CLOCK_REALTIME, &curTime);

	t +=  (time2millis(&curTime)-time2millis(&lastTime)) ;
	lastTime = curTime;
	return t;
}

double GetSystemTimeAccurate()
{
	return (float)GetSystemTimeTick();
}

unsigned int GetFreeMemory()
{
	return 0; //don't care
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

const char * GetBundleName();
const char * GetBundlePrefix();

int GetSystemData()
{
	/*
	//don't know if it's safe to use this, I read on the forum it can lockup devices who are on wifi only?
	int ret = PDL_isAppLicensedForDevice(  (  string(GetBundlePrefix())+string(GetBundleName()) ).c_str());
	if (ret == 0) return C_PIRATED_YES;

	*/
	return C_PIRATED_NO;
}

void RemoveFile( string fileName, bool bAddSavePath)
{

	#ifdef WIN32

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
#else
	if (bAddSavePath)
	{
		fileName = GetSavePath()+fileName;
	}

	if (unlink(fileName.c_str()) == -1)
	{
		switch (errno)
		{
		case EACCES: 
			LogMsg("Warning: Unable to delete file %s, no access", fileName.c_str());
			break;
		case EBUSY: 
			LogMsg("Warning: Unable to delete file %s, file is being used", fileName.c_str());
			break;
		case EPERM: 
			LogMsg("Warning: Unable to delete file %s, may be a dir", fileName.c_str());
			break;
		case EROFS: 
			LogMsg("Warning: Unable to delete file %s, File system is read only", fileName.c_str());
			break;
		default:
			//LogMsg("Warning: Unable to delete file %s, unknown error", fileName.c_str());
			//file doesn't exist
break;
		}
	}
#endif

}

string GetRegionString()
{

	char* country = NULL;
	char* language = NULL;

	bps_initialize();
	locale_get(&language, &country);

	LogMsg("Got %s, and %s", language, country);
	/*
	const int buffLen= 64;
	char buff[buffLen];
	PDL_GetLanguage(buff, buffLen);

	for (int i=0; buff[i]; i++)
	{
		buff[i] = tolower(buff[i]);
	}
	return buff;
*/
	return "en_jp";
}


//month is 1-12 btw
int GetDaysSinceDate(int month,int day, int year)
{

	LogMsg("GetDaysSinceDate url not done");
	assert(!"no!");
	return 0;
/*
	time_t ltime;
	time( &ltime );

	tm expire = { 0, 0, 0, day, month-1, year-1900, 0 };	 //Month is 0-11 btw
	tm today = *localtime( &ltime );

	long time_now = (long)today.tm_mday + (long)today.tm_mon * 30 + today.tm_year*365;
	long time_exp = (long)expire.tm_mday +(long)expire.tm_mon * 30 + expire.tm_year * 365;
	long time_passed = time_now - time_exp;

	//now let's convert it back to days
	if (time_passed == 0) return 0; //avoid devide by 0
	return time_passed;
	*/
}


bool RTCreateDirectory(const std::string &dir_name)
{
	if (dir_name.empty())
		return false;

	// this will be a full path
	std::string full_path; 	// calculate the full path

	// TODO: add here Linux version of GetFullPathName
	full_path = dir_name;
	return ::mkdir(full_path.c_str(), 0755) == 0;
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



vector<string> GetDirectoriesAtPath(string path)
{
	vector<string> v;


#ifdef _DEBUG
	//LogMsg("GetDirectoriesAtPath: %s", path.c_str());
#endif

	dirent * ent;
	DIR *dp;

	dirent_extra *pExtra = NULL;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetDirectoriesAtPath: opendir failed");
		return v;
	}

	errno = EOK;
	while (ent = readdir(dp))
	{
		pExtra = _DEXTRA_FIRST(ent);
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		LogMsg("Got %s. type %d", ent->d_name, int(pExtra->d_type));

		if (pExtra->d_type == 0) //DT_DIR not defined.. er... but it's 0, verified it on playbook
		{
			v.push_back(ent->d_name);
		}
	}


	closedir(dp);
	return v;

}

#ifdef WIN32
BOOL IsDots(const TCHAR* str) {
	if(_tcscmp(str,".") && _tcscmp(str,"..")) return FALSE;
	return TRUE;
}
#endif

vector<string> GetFilesAtPath(string path)
{

	vector<string> v;
	dirent * buf, * ent;
	DIR *dp;
	dirent_extra *pExtra = NULL;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetDirectoriesAtPath: opendir failed");
		return v;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{
		pExtra = _DEXTRA_FIRST(ent);
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		if (pExtra->d_type != 0) //regular file
		{
			v.push_back(ent->d_name);
		}
	}

	free (buf);
	closedir(dp);
	return v;

}

//based on a snippet fromFeroz Zahid (http://www.codeguru.com/cpp/w-p/files/folderdirectorymaintenance/article.php/c8999/
bool RemoveDirectoryRecursively(string path)
{
		dirent * buf, * ent;
		DIR *dp;
		dirent_extra *pExtra = NULL;

		dp = opendir(path.c_str());
		if (!dp)
		{
			LogError("RemoveDirectoryRecursively: opendir failed");
			return false;
		}

		buf = (dirent*) malloc(sizeof(dirent)+512);
		while (readdir_r(dp, buf, &ent) == 0 && ent)
		{
			pExtra = _DEXTRA_FIRST(ent);

			if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
			if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

			//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
			if (pExtra->d_type == 4) //regular file
			{
				string fName = path+string("/")+ent->d_name;
				//LogMsg("Deleting %s", fName.c_str());
				unlink( fName.c_str());
			}

			if (pExtra->d_type == 0) //regular file
			{
				string fName = path+string("/")+ent->d_name;
				//LogMsg("Entering DIR %s",fName.c_str());
				if (!RemoveDirectoryRecursively(fName.c_str()))
				{
					LogError("Error removing dir %s", fName.c_str());
					break;
				}
			}
		}

		free (buf);
		closedir(dp);

		//delete the final dir as well
		rmdir( path.c_str());
		return true; //success
}


bool CheckIfOtherAudioIsPlaying()
{
	return false;
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{

	return;
	/*
	PDL_Orientation o = PDL_ORIENTATION_0;

	//map to WebOS specific versions from the iPhone versions
	switch(orientation)
	{
	case ORIENTATION_PORTRAIT:
		o = PDL_ORIENTATION_0;
		break;

	case ORIENTATION_PORTRAIT_UPSIDE_DOWN:
		o = PDL_ORIENTATION_180;
		break;


	case ORIENTATION_LANDSCAPE_LEFT:
		o = PDL_ORIENTATION_270;
		break;

	case ORIENTATION_LANDSCAPE_RIGHT:

		break;
	}

	PDL_Err err = PDL_SetOrientation(o);
	if (err == PDL_INVALIDINPUT) LogMsg("Invalid orientation passed");
	if (err == PDL_ECONNECTION) LogMsg("Unable to communicate with app card");
	*/
	assert(0);
}

bool HasVibration()
{
	return false;
}

string GetDeviceID()
{
	assert(0);
	return "NONE";

	/*
	const int buffSize = 128;
	char buff[buffSize];
	
	PDL_Err status = PDL_GetUniqueID(buff, buffSize);

	if (status != PDL_NOERROR)
	{
		LogMsg("Error getting DeviceID for some reason");
		return "UNKNOWN";
	}
	#ifdef _DEBUG
	LogMsg("Read %s as the deviceid", buff);
	#endif
	return buff;
	*/

}
string GetNetworkType()
{
	return "none"; // not supported for this OS
}