#include "HTML5Utils.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstdarg>
#include <cassert>
#include "PlatformSetup.h"
#include <sys/stat.h> //for mkdir
#include <emscripten/emscripten.h>
#include "html5/SharedJSLIB.h"

const char g_html_save_persistent_prefix[] = "/proton_html_save_"; //in emscripten, the persistent "user data" is going to be this prefix + GetAppName(), so apps on the same site won't share the same dir 
const char g_html_cache_persistent_prefix[] = "/proton_html_cache_"; //in emscripten, the persistent "user data" is going to be this prefix + GetAppName(), so apps on the same site won't share the same dir 

#ifndef _CONSOLE 
	//if console is defined, we might be a linux command line server or something, we don't know what GL/GLES stuff
	//is and don't use BaseApp
	#include "BaseApp.h"
#endif

#include <time.h>

using namespace std;

void StringReplace(const std::string& what, const std::string& with, std::string& in);
vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter );

string GetDateAndTimeAsString();
void AppendStringToFile(string filename, string text);

#ifndef RT_CUSTOM_LOGMSG
void LogMsg( const char* traceStr, ... )
{
	va_list argsVA;
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );

	va_start ( argsVA, traceStr );
	vsnprintf( buffer, logSize, traceStr, argsVA );
	va_end( argsVA );
	
	//__Linux_log_write(Linux_LOG_ERROR,GetAppName(), buffer);
	printf ((char*)buffer);
	printf ("\r\n");
	fflush(stdout);

	AppendStringToFile( GetBaseAppPath()+"log.txt", GetDateAndTimeAsString()+": "+string(buffer)+"\r\n");

	#ifndef _CONSOLE 
		if (IsBaseAppInitted())
		{
			GetBaseApp()->GetConsole()->AddLine(buffer);
		}
	#endif
}

#endif


string GetBaseAppPath()
{
	char szDir[1024];
	getcwd( szDir, 1024);
	if (szDir[0] == '/' && szDir[1] == 0) return szDir; //return / instead of //, don't think it matters though
	return string(szDir)+"/";
}


string GetSavePath()
{
	static string savePath;
	if (savePath.empty())
	{
		//one time setup
		savePath = string(g_html_save_persistent_prefix) + GetAppName() + "/";
		StringReplace(" ", "_", savePath);
	}
	
	return savePath;
}


//returns the SD card user save path (will be deleted when app is uninstalled on 2.2+)
//returns "" if no SD card/external writable storage available

string GetAppCachePath()
{
	static string cachePath;
	if (cachePath.empty())
	{
		//one time setup
		cachePath = string(g_html_cache_persistent_prefix) + GetAppName() + "/";
		StringReplace(" ", "_", cachePath);
	}
	return cachePath; //this means this stuff is persistent too, keep that in mind!
}

void LaunchEmail(string subject, string content)
{
}

void LaunchURL(string url)
{
	
	LogMsg("LaunchURL: %s", url.c_str());
	JLIB_OpenURL(url.c_str());

//	emscripten_run_script(string("window.open(\""+url+"\", '_blank');").c_str());
}

void HTMLDownloadFileFromFileSystem(string sourceFile, string fileToWrite)
{
	string command = "saveFileFromMemoryFSToDisk('" + sourceFile + "','" + fileToWrite + "')";
	emscripten_run_script(command.c_str());
}

void HTMLUploadFileToFileSystem()
{
	LogMsg("Click");
	JLIB_OnClickSomethingByID("uploader"); //hope this exists in your html file
}

extern "C" 
{

	void PROTON_SystemMessage(int x, float y, char * str)
	{
		string message = "";
		//LogMsg("Got system message! int: %d, float: %f message: %s", x, y, str);

		//actually send the message
		OSMessage o;
		o.m_type = (OSMessage::eMessageType)x;
		o.m_string = str; //make a copy

		GetBaseApp()->AddOSMessage(o);

		SAFE_FREE(str);
	}

	void PROTON_GUIMessage(int x, float y, char * str)
	{
		string message = "";
		//LogMsg("Got system message! int: %d, float: %f message: %s", x, y, str);

		//actually send the message
		GetMessageManager()->SendGUIStringEx((eMessageType)x, y, 0,0, str, 0);
		SAFE_FREE(str);
	}
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
	switch (GetEmulatedPlatformID())
	{
	case PLATFORM_ID_WINDOWS:
	case PLATFORM_ID_OSX:
	case PLATFORM_ID_LINUX:
		return true;

	default:
		return false;
	}
}

ePlatformID GetPlatformID()
{
	return PLATFORM_ID_HTML5;
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
	return C_DEVICE_MEMORY_CLASS_4;
}

int GetYOffset()
{
	return 0;
}

unsigned int GetSystemTimeTick()
{

	//Note:  Due to using an unsigned int for millseconds, the timer kept rolling over on my linux game server I wrote for
	//Tanked - so I changed this to start at 0, which gives me 46 days of running before the roll-over.  Why don't I just change
	//my stuff to handle timing roll-overs or perhaps use a bigger type for timing?  Well.. hmm.  Ok, maybe, but for now this. -Seth
	
	static unsigned int incrementingTimer = 0;
	static double buildUp = 0;
	static double lastTime = 0;

    
	/*
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	double timeDouble = time.tv_sec*1000 + time.tv_nsec/1000000;
	*/

	//Well, the above doesn't work on the iPhone, it returns 0 only.  So let's use this instead:
	double timeDouble = clock()/1000;

	double change = timeDouble -lastTime;
	if (change > 0 && change < (1000*120) )
	{
		incrementingTimer += change;
	}
	lastTime = timeDouble;
	
	return incrementingTimer;
}

uint64 GetSystemTimeTickLong()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	uint64 accum;
	accum = time.tv_sec*1000 + time.tv_nsec/1000000;
	return accum;
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

double GetSystemTimeAccurate()
{
	return double(GetSystemTimeTick());
	return 0;
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
			LogError("Warning: Unable to delete file %s, file is being used", fileName.c_str());
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
	return "us_en";
}

//month is 1-12 btw
int GetDaysSinceDate(int month,int day, int year)
{
	//LogMsg("GetDaysSinceDate url not done");
	assert(!"no!");
	return 0;
}

bool RTCreateDirectory(const std::string &dir_name)
{
#ifdef _DEBUG
	//LogMsg("CreateDirectory: %s", dir_name.c_str());
#endif

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
		if (!RTCreateDirectory(basePath + path))
		{
			//LogMsg("Error creating %s", (basePath + path).c_str());
		}
		path += "/";
	}
}


vector<string> GetDirectoriesAtPath(string path)
{
	vector<string> v;
	
#ifdef _DEBUG
	//LogMsg("GetDirectoriesAtPath: %s", path.c_str());
#endif

	dirent * buf, * ent;
	DIR *dp;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetDirectoriesAtPath (%s): opendir failed", path.c_str());
		return v;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		
		if (ent->d_type == DT_DIR)
		{
			v.push_back(ent->d_name);
		}
	}

	free (buf);
	closedir(dp);
	return v;
}

vector<string> GetFilesAtPath(string path)
{
#ifdef _DEBUG
	//LogMsg("GetFilesAtPath: %s", path.c_str());
#endif

	vector<string> v;
	dirent * buf, * ent;
	DIR *dp;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetFilesAtPath (%s): opendir failed", path.c_str());
		return v;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		if (ent->d_type != DT_DIR) //regular file
		{
			v.push_back(ent->d_name);
		}
	}

	free (buf);
	closedir(dp);
	return v;
}

bool RemoveDirectoryRecursively(string path)
{
//	LogMsg(" RemoveDirectoryRecursively: %s", path.c_str());
	
	dirent * buf, * ent;
	DIR *dp;

	dp = opendir(path.c_str());
	if (!dp)
	{
		string error = "unknown";
		switch (errno)
		{
			case EACCES: error = "EACCES";	break;
			case EAGAIN: error = "EBADFID";	break;
			case EBUSY: error = "EBUSY";	break;
			case EEXIST: error = "EEXIST";	break;
			case EFAULT: error = "EFAULT";	break;

			default: ;
		}
		LogError("RemoveDirectoryRecursively: opendir of %s failed with error %d (%s)", path.c_str(), errno, error.c_str());
		return false;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{
		
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

//		LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		if (ent->d_type != DT_DIR) //regular file
		{
			string fName = path+string("/")+ent->d_name;
//			LogMsg("Deleting %s", fName.c_str());
			unlink( fName.c_str());
		}

		if (ent->d_type == DT_DIR) //regular file
		{
			string fName = path+string("/")+ent->d_name;
//			LogMsg("Entering DIR %s",fName.c_str());
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

void CreateAppCacheDirIfNeeded()
{
  //only applicable to iOS
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{
}

bool HasVibration()
{
	return true;
}
string GetNetworkType()
{
	return "none"; // not supported for this OS
}