#ifdef RT_WEBOS
#include "SDL.h"
#include "WebOSUtils.h"
#include "PDL.h"
#include <errno.h>


using namespace std;

#ifdef RT_WEBOS_ARM
#include "PlatformSetupWebOS.h"
#include <dirent.h>

#include <sys/stat.h>

#else
#include "PlatformSetupWin.h"
#endif

#include <cstdlib>
#include <cstdarg>
#include <cassert>



#ifdef _WINDOWS_
	#include <io.h>
	#include <sys/stat.h>
#endif
void StringReplace(const std::string& what, const std::string& with, std::string& in);
vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter );

string GetBaseAppPath()
{
#ifdef RT_WEBOS_ARM
	char szDir[1024];
	getcwd( szDir, 1024);
	return string(szDir)+"/";

	//return ""; //well, according to the docs PDL_GetCallingPath is deprecated

#else
return "";
#endif
}



string GetSavePath()
{
#ifdef RT_WEBOS_ARM
	return "";  //well, according to the docs, PDL_GetDataFilePath is buggy and returns a dir that can't be written to and "" is better to use.

#else
	return "";

#endif
}

string GetAppCachePath()
{
	return "";
}

void LaunchEmail(string subject, string content)
{
	PDL_LaunchEmail(subject.c_str(), "");
}

void LaunchURL(string url)
{
	LogMsg("Launching %s", url.c_str());
	
	PDL_LaunchBrowser(url.c_str());

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
	switch (GetEmulatedPlatformID())
	{
	case PLATFORM_ID_WINDOWS:
	case PLATFORM_ID_LINUX:
	case PLATFORM_ID_OSX:
		return true;
	}

	
	return false;
}

ePlatformID GetPlatformID()
{
	#ifdef WIN32
	return PLATFORM_ID_WINDOWS;
	#endif

	return PLATFORM_ID_WEBOS;
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
	if (GetPrimaryGLX() == 1024 || GetPrimaryGLY() == 1024) return true;
	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
#ifdef WIN32
	return C_DEVICE_MEMORY_CLASS_4;

#else
	if (GetPrimaryGLX() == 1024 || GetPrimaryGLY() == 1024)
	{
		//touchpad, they have a lot of mem
		return C_DEVICE_MEMORY_CLASS_3;

	} else
	{
		//webos phone probably, who knows
		return C_DEVICE_MEMORY_CLASS_2;
	}
#endif
}

bool LaterThanNow(const int year, const int month, const int day)
{
	return false;
}

unsigned int GetSystemTimeTick()
{
	return SDL_GetTicks();
}
double GetSystemTimeAccurate()
{
	//return double(GetTickCount());

	return SDL_GetTicks();
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
bool IsAppInstalled(string packageName)
{
#ifdef _DEBUG
	LogMsg("IsAppInstalled not yet handled for this OS");
#endif
	return false;
}

string GetRegionString()
{
	const int buffLen= 64;
	char buff[buffLen];
	PDL_GetLanguage(buff, buffLen);

	for (int i=0; buff[i]; i++)
	{
		buff[i] = tolower(buff[i]);
	}
	return buff;
	
	//return "en_jp";
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
	LogMsg("Creating dir %s", full_path.c_str());
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

#ifdef WIN32
BOOL IsDots(const TCHAR* str) {
	if(_tcscmp(str,".") && _tcscmp(str,"..")) return FALSE;
	return TRUE;
}
#endif

vector<string> GetDirectoriesAtPath(string path)
{
	vector<string> v;

#ifdef WIN32
	if (path.empty()) path += "."; //needed for relative paths

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

#else

	//linux version

	dirent * buf, * ent;
	DIR *dp;

		dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetDirectoriesAtPath: opendir failed to get %s", path.c_str());
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
#endif
	return v;
	
}

vector<string> GetFilesAtPath(string path)
{
#ifdef WIN32

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

#else
	vector<string> v;
	dirent * buf, * ent;
	DIR *dp;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("GetFilesAtPath: opendir failed to get %s", path.c_str());
		return v;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{
		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		if (ent->d_type == DT_REG) //regular file
		{
			v.push_back(ent->d_name);
		}
	}

	free (buf);
	closedir(dp);
	return v;
#endif
}

//based on a snippet fromFeroz Zahid (http://www.codeguru.com/cpp/w-p/files/folderdirectorymaintenance/article.php/c8999/
bool RemoveDirectoryRecursively(string path)
{
#ifdef WIN32
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
		if(FindNextFile(hFind,&FindFileData)) 
		{
			if(IsDots(FindFileData.cFileName)) continue;
			_tcscat(FileName,FindFileData.cFileName);
			if((FindFileData.dwFileAttributes &
				FILE_ATTRIBUTE_DIRECTORY))
			{

					// we have found a directory, recurse
					if(!RemoveDirectoryRecursively(FileName))
					{
						FindClose(hFind);
						return FALSE;    // directory couldn't be deleted
					}
					// remove the empty directory
					RemoveDirectory(FileName);
					_tcscpy(FileName,DirPath);
			} else 
			{
				if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
					// change read-only file mode
					_chmod(FileName, _S_IWRITE);
				
				if(!DeleteFile(FileName))
				{    // delete the file
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
#else
	dirent * buf, * ent;
	DIR *dp;

	dp = opendir(path.c_str());
	if (!dp)
	{
		LogError("RemoveDirectoryRecursively: opendir failed");
		return false;
	}

	buf = (dirent*) malloc(sizeof(dirent)+512);
	while (readdir_r(dp, buf, &ent) == 0 && ent)
	{

		if (ent->d_name[0] == '.' && ent->d_name[1] == 0) continue;
		if (ent->d_name[0] == '.' && ent->d_name[1] == '.' && ent->d_name[2] == 0) continue;

		//LogMsg("Got %s. type %d", ent->d_name, int(ent->d_type));
		if (ent->d_type == DT_REG) //regular file
		{
			string fName = path+string("/")+ent->d_name;
			//LogMsg("Deleting %s", fName.c_str());
			unlink( fName.c_str());
		}

		if (ent->d_type == DT_DIR) //regular file
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
#endif

}


bool CheckIfOtherAudioIsPlaying()
{
	return false;
}

void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{
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
}

bool HasVibration()
{
	return false;
}

string GetDeviceID()
{
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
}

void CreateAppCacheDirIfNeeded()
{
	//only applicable to iOS
}


float GetDeviceOSVersion()
{
	return ((float)PDL_GetPDKVersion())/100.0f;
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
#endif