#include "FLASHUtils.h"
#include <errno.h>

#include <AS3/AS3.h>

using namespace std;

string g_appCachePath;

#include "PlatformSetupFLASH.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdarg>
#include <cassert>


//I do the trace so these messages will show in Vizzy or other flash debug tools

void LogMsg ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );
	
	//Flash will show these in trace automatically, unless the special flash Console.as used in the RTConsole is used
#ifdef _CONSOLE
	printf(buffer);
	printf("\n");
#endif
	
	
		inline_as3(
			"trace(CModule.readString(%0, %1));\n"
			: : "r"(buffer), "r"(strlen(buffer))
			);
	

	fflush(stdout);
}

#ifdef _CONSOLE
void LogError ( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );

	//Flash will show these in trace automatically, unless the special flash Console.as used in the RTConsole is used
#ifdef _CONSOLE
	printf("ERROR: ");
	printf(buffer);
	printf("\n");
#endif

	inline_as3(
		"trace(\"ERROR: \"+CModule.readString(%0, %1));\n"
		: : "r"(buffer), "r"(strlen(buffer))
		);


	fflush(stdout);
}

#endif

void StringReplace(const std::string& what, const std::string& with, std::string& in);
vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter );

string GetBaseAppPath()
{

	//char szDir[1024];
	//getcwd( szDir, 1024);
	//return string(szDir)+"/";
	return "";
}



string GetSavePath()
{
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
void LaunchEmail(string subject, string content)
{
//	PDL_LaunchEmail(subject.c_str(), "");
}

void LaunchURL(string url)
{
	LogMsg("Launching %s", url.c_str());

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"Console.current.LaunchURL(CModule.readString(%0, %1));\n"
		: : "r"(url.c_str()) , "r"(url.length())
	);

}

string GetClipboardText()
{
	return "";
}

bool IsIPhone3GS()
{
	return false;
}

bool IsDesktop()
{
	return true;
}

ePlatformID GetPlatformID()
{
	return PLATFORM_ID_FLASH;
}

bool IsIphoneOriPad()
{
	return false;
}

bool IsIphone()
{
	return false;
}

bool IsIphone4()
{
	return false; 
}

bool IsIPAD()
{
	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
	C_DEVICE_MEMORY_CLASS_4;
}

unsigned int GetSystemTimeTick()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec/1000 + tv.tv_sec*1000;
}
double GetSystemTimeAccurate()
{
	return double(GetSystemTimeTick());
}

unsigned int GetFreeMemory()
{
	return 0; //don't care on the PC
}

unsigned int GetNativeMemoryUsed()
{
	unsigned int usedBytes = 0;

	inline_as3(
		"import com.adobe.flascc.Console;\n"
		"%0= Console.current.calculateUsedBytes();\n"
		: "=r"(usedBytes) :
		);

		return usedBytes;
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
	return C_PIRATED_NO;
}

void RemoveFile( string fileName, bool bAddSavePath)
{

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


}

string GetRegionString()
{
	return "en_us";
}


//month is 1-12 btw
int GetDaysSinceDate(int month,int day, int year)
{
	
	LogMsg("GetDaysSinceDate url not done");
//	assert(!"no!");
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
	LogMsg("Todo: GetDirectoriesAtPath: %s", path.c_str());
#endif

//assert(!"todo");

/*
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
	*/
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
	
	/*
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
	*/
	return v;

}

//based on a snippet fromFeroz Zahid (http://www.codeguru.com/cpp/w-p/files/folderdirectorymaintenance/article.php/c8999/
bool RemoveDirectoryRecursively(string path)
{
	
	/*
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
	*/

	return false;  //fail
}



bool CheckIfOtherAudioIsPlaying()
{
	return false;
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{
	return;
}

bool HasVibration()
{
	return false;
}

string GetDeviceID()
{
	return "NONE";
}

void CreateAppCacheDirIfNeeded()
{
	//only applicable to iOS
}


float GetDeviceOSVersion()
{
	return 0;
}


string GetMacAddress()
{
	//TODO
	return ""; //unimplemented
}

string GetNetworkType()
{
	return "none"; // not supported for this OS
}