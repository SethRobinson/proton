/*
 *  iOSUtils.mm
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */
#include "PlatformSetup.h"
#include "OSXUtils.h"

#import <cstdarg>
#include <string>
#include <sys/time.h>
#include <sys/sysctl.h>
#include "BaseApp.h"
#include <SystemConfiguration/SystemConfiguration.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_dl.h>
#include "Network/NetUtils.h"
#import <Cocoa/Cocoa.h>

const char * GetAppName();

using namespace std;

#ifndef RT_CUSTOM_LOGMSG
void LogMsg(const char *lpFormat, ...)
{
	
	std::va_list argPtr ;
	va_start( argPtr, lpFormat ) ;
	NSLogv([NSString stringWithCString:lpFormat], argPtr) ;
	
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );
	vsnprintf( buffer, logSize,  lpFormat, argPtr );

	if (IsBaseAppInitted())
	GetBaseApp()->GetConsole()->AddLine(buffer);
	va_end(argPtr) ;
		
} 

#endif

string GetMacAddress()
{
	  int                 mgmtInfoBase[6];
  char                *msgBuffer = NULL;
  size_t              length;
  unsigned char       macAddress[6];
  struct if_msghdr    *interfaceMsgStruct;
  struct sockaddr_dl  *socketStruct;
  NSString            *errorFlag = NULL;
 
  // Setup the management Information Base (mib)
  mgmtInfoBase[0] = CTL_NET;        // Request network subsystem
  mgmtInfoBase[1] = AF_ROUTE;       // Routing table info
  mgmtInfoBase[2] = 0;              
  mgmtInfoBase[3] = AF_LINK;        // Request link layer information
  mgmtInfoBase[4] = NET_RT_IFLIST;  // Request all configured interfaces
 
  // With all configured interfaces requested, get handle index
  if ((mgmtInfoBase[5] = if_nametoindex("en0")) == 0) 
    errorFlag = @"if_nametoindex failure";
  else
  {
    // Get the size of the data available (store in len)
    if (sysctl(mgmtInfoBase, 6, NULL, &length, NULL, 0) < 0) 
      errorFlag = @"sysctl mgmtInfoBase failure";
    else
    {
      // Alloc memory based on above call
      if ((msgBuffer = (char*)malloc(length)) == NULL)
        errorFlag = @"buffer allocation failure";
      else
      {
        // Get system information, store in buffer
        if (sysctl(mgmtInfoBase, 6, msgBuffer, &length, NULL, 0) < 0)
          errorFlag = @"sysctl msgBuffer failure";
      }
    }
  }
  // Befor going any further...
  if (errorFlag != NULL)
  {
    NSLog(@"Error: %@", errorFlag);
//   string([macAddressString cStringUsingEncoding:NSUTF8StringEncoding]);
    return "";
  }
  // Map msgbuffer to interface message structure
  interfaceMsgStruct = (struct if_msghdr *) msgBuffer;
  // Map to link-level socket structure
  socketStruct = (struct sockaddr_dl *) (interfaceMsgStruct + 1);  
  // Copy link layer address data in socket structure to an array
  memcpy(&macAddress, socketStruct->sdl_data + socketStruct->sdl_nlen, 6);  
  // Read from char array into a string object, into traditional Mac address format
  NSString *macAddressString = [NSString stringWithFormat:@"%02X:%02X:%02X:%02X:%02X:%02X", 
                                macAddress[0], macAddress[1], macAddress[2], 
                                macAddress[3], macAddress[4], macAddress[5]];
  //NSLog(@"Mac Address: %@", macAddressString);  
  // Release the buffer memory
  free(msgBuffer);
  return string([macAddressString cStringUsingEncoding:NSUTF8StringEncoding]);
}

bool IsAppInstalled(string packageName)
{
	#ifdef _DEBUG
	  LogMsg("IsAppInstalled not yet handled for this OS");
	#endif
	return false;
}

void LaunchURL(string url)
{
	NSURL *appStoreUrl = [NSURL URLWithString:[NSString stringWithCString: url.c_str() encoding: [NSString defaultCStringEncoding]]];
	[[NSWorkspace sharedWorkspace] openURL:appStoreUrl];
}

float GetDeviceOSVersion()
{
	//TODO
	return 0.0f;
}

string GetDeviceID()
{
    return "";
}

bool LaterThanNow(const int year, const int month, const int day)
{
	return false;
}	

eNetworkType IsNetReachable(string url)
{
		
	assert(!"Uh.. I don't really use this, and the emulator build was giving me problems with it");
	
	/*
	// Before trying to connect, verify we have a network connection
	
	NSString *str =  [NSString stringWithCString: url.c_str() encoding: [NSString defaultCStringEncoding]];
	
	SCNetworkReachabilityFlags          flags;
	SCNetworkReachabilityRef reachability =  SCNetworkReachabilityCreateWithName(NULL, [str UTF8String]);
	BOOL gotFlags = SCNetworkReachabilityGetFlags(reachability, &flags);
	
	CFRelease(reachability);
	
	if (!gotFlags || flags & kSCNetworkReachabilityFlagsConnectionRequired || !(flags & kSCNetworkReachabilityFlagsReachable)) {
		return C_NETWORK_NONE;
	}else{
		if (flags & kSCNetworkReachabilityFlagsIsWWAN){
			return C_NETWORK_CELL;
		}else{
			return C_NETWORK_WIFI;
		}
		
	}
	 */
	return C_NETWORK_CELL;
}

string g_stringInput;
void SetLastStringInput(string s)
{
	g_stringInput = s;
}

string GetNetworkType()
{
	return "none"; //faking response for OSX
}

string GetLastStringInput()
{
	return g_stringInput; //g_string?? heh. heh. hehe.	almost makes using globals a good thing.
}

string GetBaseAppPath()
{

	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
	{
		LogMsg("Error getting bundle path");
	}
	CFRelease(resourcesURL);
	

	return string(path)+"/";

//	chdir(path);
//	LogMsg( path);
	
}

string GetSavePath()
{
	
	 NSArray* paths = NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory,
        NSUserDomainMask,
        YES);
    if ([paths count] == 0)
    {
        // *** creation and return of error object omitted for space
        LogError("Couldn't get app path!");
        return "";
    }
 
    NSString *resolvedPath = [paths objectAtIndex:0];
 
	return string([resolvedPath cStringUsingEncoding:NSUTF8StringEncoding])+"/"+string(GetAppName())+"/";
}

string GetAppCachePath() 
{
	return GetSavePath();
}

void CreateAppCacheDirIfNeeded()
{
	CreateDirectoryRecursively("", GetAppCachePath());
}

void RemoveFile(string fileName, bool bAddSavePath)
{ 
	if (bAddSavePath)
	{
		fileName = GetSavePath()+fileName;
	}
	
	NSString *str =  [NSString stringWithCString: fileName.c_str() encoding: [NSString defaultCStringEncoding]];
	
	NSFileManager *FM = [NSFileManager defaultManager];
	[FM removeItemAtPath:str error:NULL];
}	

//The below function is based on a Snippet from http://iphonedevelopertips.com/device/determine-if-iphone-is-3g-or-3gs-determine-if-ipod-is-first-or-second-generation.html

string GetDeviceTypeString()
{
	size_t size;
	// Set 'oldp' parameter to NULL to get the size of the data
	// returned so we can allocate appropriate amount of space
	sysctlbyname("hw.machine", NULL, &size, NULL, 0); 
	// Allocate the space to store name
	char *name = (char*)malloc(size);
	// Get the platform name
	sysctlbyname("hw.machine", name, &size, NULL, 0);
	// Place name into a string
	string deviceType = name;
	// Done with this
	free(name);
	return deviceType;
}


bool IsIPhone3GS()
{
	return false;
}

bool IsIPodTouchThirdGen()
{
	return false;
}


bool IsIPAD()
{
	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
	return C_DEVICE_MEMORY_CLASS_4; //lots of mem
}


bool IsIphone4()
{
	return false;
}

int g_primaryGLX = 0;
int g_primaryGLY = 0;

//this doesn't change even if you rotate, for speed
int GetPrimaryGLX() {return g_primaryGLX;}
int GetPrimaryGLY() {return g_primaryGLY;}

int GetSystemData()
{	
	return C_PIRATED_NO;
}
	
unsigned int GetSystemTimeTick()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec/1000 + tv.tv_sec*1000;
}

double GetSystemTimeAccurate()
{
	static double first = CFAbsoluteTimeGetCurrent();
	//	NSLog(@"operation took %.5f ms", CFAbsoluteTimeGetCurrent()*1000 );
	return uint32((CFAbsoluteTimeGetCurrent()-first)*1000);
}


bool IsIphone()
{
	return false;
}

const char* iPhoneVersion()
{
	return "";
}

const char* iPhoneDeviceID()
{
	return "";
}
string GetRegionString()
{
	NSLocale *currentUsersLocale = [NSLocale currentLocale];
	NSLog(@"Current Locale: %@", [currentUsersLocale localeIdentifier]);
	return [[currentUsersLocale localeIdentifier] cStringUsingEncoding:NSUTF8StringEncoding];
}

#import <mach/mach.h>
#import <mach/mach_host.h>
 
//Snippet from http://landonf.bikemonkey.org/2008/12/06

unsigned int GetFreeMemory ()
 {
    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;
    
    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);        
 
    vm_statistics_data_t vm_stat;
              
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
        LogMsg("Failed to fetch vm statistics");
 
    /* Stats in bytes */ 
	 natural_t mem_free = vm_stat.free_count * pagesize;
	 //natural_t mem_total = mem_used + mem_free;
	 //natural_t mem_used = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count) * pagesize;
	 // LogMsg("Mem used: %u free: %u total: %u", mem_used, mem_free, mem_total);
   return mem_free;
}

void CreateDirectoryRecursively(string basePath, string path)
{
	string fileName = basePath+path;
	NSString *str =  [NSString stringWithCString: fileName.c_str() encoding: [NSString defaultCStringEncoding]];
[[NSFileManager defaultManager] createDirectoryAtPath:str
                          withIntermediateDirectories:YES
                                           attributes:nil
                                                error:nil];
}

bool RemoveDirectoryRecursively(string path)
{
	RemoveFile(path, false);
	return true;
}

vector<string> GetDirectoriesAtPath(string path)
{
	
	LogMsg("Scanning dir %s", path.c_str());
	vector<string> v;
	
	NSString *str =  [NSString stringWithCString: path.c_str() encoding: [NSString defaultCStringEncoding]];
	NSArray *origContents = [[NSFileManager defaultManager] directoryContentsAtPath:str];
	NSLog(@"Number of files = %d", origContents.count);
	
	string dir;
	
	for (int i = 0; i < origContents.count; i++) 
	{
		dir = [[origContents objectAtIndex:i] cStringUsingEncoding:NSUTF8StringEncoding];
		if (dir.find(".") == string::npos)
		{
			//we're assuming that any name without a . in it is a dir.  Probably dumb, but works since we're controlling what gets written...
			v.push_back(dir);
		}
		NSLog (@"%d: <%@>", i, [origContents objectAtIndex:i]);
	}
	
	return v;
}

vector<string> GetFilesAtPath(string path)
{
	
	vector<string> v;
	
	NSString *str =  [NSString stringWithCString: path.c_str() encoding: [NSString defaultCStringEncoding]];
	NSArray *origContents = [[NSFileManager defaultManager] directoryContentsAtPath:str];
	//NSLog(@"Number of files = %d", origContents.count);
	
	string dir;
	
	for (int i = 0; i < origContents.count; i++) 
	{
		dir = [[origContents objectAtIndex:i] cStringUsingEncoding:NSUTF8StringEncoding];
		//if (dir.find(".") != string::npos)
		{
			//we're assuming that any name without a . in it is a dir.  Probably dumb, but works since we're controlling what gets written...
			v.push_back(dir);
		}
		//NSLog (@"%d: <%@>", i, [origContents objectAtIndex:i]);
	}
	
	return v;
}

bool IsIphoneOriPad()
{
	return false;
}

string GetClipboardText()
{
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	NSString *info = [pb stringForType: NSStringPboardType];
    string text = [info cStringUsingEncoding:NSUTF8StringEncoding];
	return text;
}

//donated by PhilHassey - uncomment when needed and when can verify it compiles ok

/*
void SetClipboardText(std::string v)
{
     NSPasteboard *pb = [NSPasteboard generalPasteboard];
     NSString *data = [[NSString alloc] initWithUTF8String: v.c_str()];
     [pb clearContents];
     [pb setString:data forType: NSStringPboardType];
     [data release];
}
*/

bool IsDesktop()
{
	if (GetEmulatedPlatformID() == PLATFORM_ID_WINDOWS || GetEmulatedPlatformID() == PLATFORM_ID_OSX
		|| GetEmulatedPlatformID() == PLATFORM_ID_LINUX) return true;
	return false;
}


ePlatformID GetPlatformID()
{
	return PLATFORM_ID_OSX;
}
	
void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{

}

bool HasVibration()
{
	return false;
}

void InitDeviceScreenInfoEx(int width, int height, int orientation)
{
	g_primaryGLX = width;
	g_primaryGLY = height;
	
	if (!GetBaseApp()->IsInitted())
		{
			SetupScreenInfo(width, height, GetOrientation());
			LogMsg("Initializing BaseApp...");
			srand( (unsigned)time(NULL) );
			
			CreateAppCacheDirIfNeeded(); //actually these creates our user data directory as well as they are the
			//same path. It will be located in ~/Library/Application Support/"+GetAppName().  If you don't want this
			//directory created, we'll need to add some flag here to not do it I guess.
			
			if (!GetBaseApp()->Init())
			{
				
				NSLog(@"Couldn't init app");
				//[self release];
				//return nil;
			}
			
//			CreateDirectoryRecursively("", GetAppCachePath());

		} else
		{
				SetupScreenInfo(width, height, GetOrientation());
		}
	

}

int ConvertOSXKeycodeToProtonVirtualKey(int c)
{
#ifdef _DEBUG
	//LogMsg("Got %d  (%c)", c, char(c));
#endif
	
	switch (c)
	{
		case 27: return VIRTUAL_KEY_BACK;
		case 127: return 8; //dunno why backspaces comes across as 127, whatever
			
			
		case NSLeftArrowFunctionKey: return VIRTUAL_KEY_DIR_LEFT;
		case NSRightArrowFunctionKey: return VIRTUAL_KEY_DIR_RIGHT;
		case NSUpArrowFunctionKey: return VIRTUAL_KEY_DIR_UP;
		case NSDownArrowFunctionKey: return VIRTUAL_KEY_DIR_DOWN;
		default:
			if (c >= NSF1FunctionKey && c <= NSF16FunctionKey)
			{
				c = VIRTUAL_KEY_F1+	(c-NSF1FunctionKey);
			}
			break;
	}
	return c;
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