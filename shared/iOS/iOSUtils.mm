/*
 *  iOSUtils.mm
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */
#include "iOSUtils.h"
#import <UIKit/UIKit.h>
#import <cstdarg>
#include <string>
#include <sys/time.h>
#include <sys/sysctl.h>
#include "BaseApp.h"
#include <SystemConfiguration/SystemConfiguration.h>
#include "Network/NetUtils.h"
#import <MobileCoreServices/MobileCoreServices.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_dl.h>
#import "Reachability.h"


#ifdef RT_IAP_SUPPORT
    //otherwise we have to add the lib even if we don't use it
    #import <AdSupport/ASIdentifierManager.h>
#endif
using namespace std;

void LogMsg(const char *lpFormat, ...)
{
	std::va_list argPtr ;
	va_start( argPtr, lpFormat ) ;
	
    //NSLogv([NSString stringWithCString:lpFormat], argPtr) ;
    
    //well, we could use NSLogv, but xcode 8 seems to have a bug where it won't show sometimes unless
    //OS_ACTIVITY_MODE is set to false or something.. so let's use our own method..
    
	
	const int logSize = 4096;
	char buffer[logSize];
	memset ( (void*)buffer, 0, logSize );
	vsnprintf( buffer, logSize,  lpFormat, argPtr );
	
    printf(buffer);
    printf("\n");
    
	if (IsBaseAppInitted())
	GetBaseApp()->GetConsole()->AddLine(buffer);
	
	va_end(argPtr) ;
	
} 


void LaunchURL(string url)
{
	NSURL *appStoreUrl = [NSURL URLWithString:[NSString stringWithCString: url.c_str() encoding: [NSString defaultCStringEncoding]]];
	[[UIApplication sharedApplication] openURL:appStoreUrl];
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

string GetLastStringInput()
{
	return g_stringInput; //g_string?? heh. heh. hehe.	almost makes using globals a good thing.
}

string GetBaseAppPath()
{
	
	CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
    char path[PATH_MAX];
	Boolean const success = CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);
    CFRelease(resourcesURL);	
    if (!success)
    {
        LogMsg("Can't change to Resources directory; something's seriously wrong\n");
		return "";
    }

	return string(path)+"/";
	
}

string GetSavePath()
{

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	return string([ [paths objectAtIndex:0] cStringUsingEncoding:NSUTF8StringEncoding])+"/";
}

string GetAppCachePath() //writable, but not backed up by iTunes
{

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES); 
	return string([ [paths objectAtIndex:0] cStringUsingEncoding:NSUTF8StringEncoding])+"/";
}

void CreateAppCacheDirIfNeeded()
{
	string path = GetAppCachePath();
	int idx = path.find("Library/Caches");
	if (idx != string::npos)
	{
		path = path.substr(0, idx);
		CreateDirectoryRecursively(path, "Library/Caches");
	}
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
	if (GetDeviceTypeString() == "iPhone2,1") return true;
	return false;
}

bool IsIPodTouchThirdGen()
{
	if (GetDeviceTypeString() == "iPhone1,3") return true;
	return false;
}


bool IsIPAD()
{
	//#if __IPHONE_3_2 <= __IPHONE_OS_VERSION_MAX_ALLOWED
#if 30200 <= __IPHONE_OS_VERSION_MAX_ALLOWED

	//LogMsg("Our idiom is %d (iPad is: %d)", UI_USER_INTERFACE_IDIOM(), UIUserInterfaceIdiomPad);
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
	/*
		assert(30200 == __IPHONE_3_2 && "Uh oh, why did this change?");
		UIScreen* mainscr = [UIScreen mainScreen];
		float x = mainscr.currentMode.size.width;
		float y = mainscr.currentMode.size.height;
	*/	
		return true;
	} else
	{
		//an iPhone/ipod	
		
	}
	#endif
	
	return false;
}

eDeviceMemoryClass GetDeviceMemoryClass()
{
	if (IsIPhone3GS()) return C_DEVICE_MEMORY_CLASS_2;
	if (IsIPodTouchThirdGen()) return C_DEVICE_MEMORY_CLASS_2;
	if (IsIPAD()) return C_DEVICE_MEMORY_CLASS_2;
	if (IsIphone4()) return C_DEVICE_MEMORY_CLASS_3;
	
	//default
	return C_DEVICE_MEMORY_CLASS_4;
}


bool IsIphone4()
{

	static bool bFirstTime = true;
	static bool bAnswer;
	
	if (bFirstTime)
	{
		bFirstTime = false;
		bAnswer = false;
	//#if __IPHONE_3_2 <= __IPHONE_OS_VERSION_MAX_ALLOWED
#if 30200 <= __IPHONE_OS_VERSION_MAX_ALLOWED
	
	if (UI_USER_INTERFACE_IDIOM() != UIUserInterfaceIdiomPad)
	{
		
		assert(30200 == __IPHONE_3_2 && "Uh oh, why did this change?");
		UIScreen* mainscr = [UIScreen mainScreen];
		if ([mainscr respondsToSelector:@selector( currentMode)] == YES) //avoid crash on pre 3.2 iOS
		{
			float x = mainscr.currentMode.size.width;
			float y = mainscr.currentMode.size.height;
			LogMsg("IsiPhone size reporting %.2f, %.2f", x, y);
			bAnswer = (x == 640 && y == 960) ; //if the screen is this big, it's gotta be a retina display
		}

	} else
	{
		//an iPad?
		
	}
#endif
		
	}
	
	return bAnswer;
}

int g_primaryGLX = 0;
int g_primaryGLY = 0;

void SetPrimaryScreenSize(int width, int height)
{
    g_primaryGLX = width;
    g_primaryGLY = height;
}


//Below based on snippet by Joe Booth - http://stackoverflow.com/questions/24150359/is-uiscreen-mainscreen-bounds-size-becoming-orientation-dependent-in-ios8 and modified by Seth A. Robinson ( rtsoft.com )
CGRect iOS7StyleScreenBounds()
{
    CGRect bounds = [UIScreen mainScreen].bounds;
    if (([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0) && UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation))
    {
        //old way, didn't work properly after returning from a lock screen in iOS 10.  Fixed it, but does it work with every case...
        //bounds.size = CGSizeMake(bounds.size.height, bounds.size.width);
        bounds.size = CGSizeMake(MIN(bounds.size.width, bounds.size.height), MAX(bounds.size.width, bounds.size.height));
    }
    return bounds;
}


//this doesn't change even if you rotate, for speed
int GetPrimaryGLX() {return g_primaryGLX;}
int GetPrimaryGLY() {return g_primaryGLY;}

int GetSystemData()
{	
	
	//this is .. just don't use it anymore, unsafe
	
	/*
	//I didn't want the text SignerIdentity existing in the exe for simple text searches to find
	string encoded = "Zqpxp%7EVrt%7E%85%7B%87%8D";
	URLDecoder decode;
	vector<byte> data = decode.decodeData(encoded);
	DecryptPiece((byte*)&data[0], data.size(), 5);
	data.push_back(0); //a null for the string

	//LogMsg((char*)&data[0]);
	NSString *str =  [NSString stringWithCString: (char*)&data[0] encoding: [NSString defaultCStringEncoding]];

	NSBundle *bundle = [NSBundle mainBundle];
	NSDictionary *info = [bundle infoDictionary];
	if ([info keyExists: str] != nil)
	{
	 return C_PIRATED_YES;
	}
	*/
	
	return C_PIRATED_NO;
}

bool CheckDay(const int year, const int month, const int day)
{
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	
	[dateFormatter setDateFormat:@"yyyy"];
    int nowyear = [[dateFormatter stringFromDate:[NSDate date]] intValue];

    [dateFormatter setDateFormat:@"MM"];
    int nowmonth = [[dateFormatter stringFromDate:[NSDate date]] intValue];

    [dateFormatter setDateFormat:@"dd"];
    int nowday = [[dateFormatter stringFromDate:[NSDate date]] intValue];
    [dateFormatter release];

	if ((nowyear == year) && (nowmonth == month) && (nowday == day))
	{
		return true;
	}
	return false;
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

bool LaterThanNow(const int year, const int month, const int day)
{
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	
	[dateFormatter setDateFormat:@"yyyy"];
    int nowyear = [[dateFormatter stringFromDate:[NSDate date]] intValue];

    [dateFormatter setDateFormat:@"MM"];
    int nowmonth = [[dateFormatter stringFromDate:[NSDate date]] intValue];

    [dateFormatter setDateFormat:@"dd"];
    int nowday = [[dateFormatter stringFromDate:[NSDate date]] intValue];
    [dateFormatter release];
    
    if (nowyear< year )
	{
		return false;
	}
	if (nowyear> year )
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
if (IsIPAD()) return false;

 	NSString *deviceType = [UIDevice currentDevice].model;
    if([deviceType isEqualToString:@"iPhone"])
	{
		return true;
	}
	return false;
}

const char* iPhoneVersion()
{
	NSString *deviceType = [UIDevice currentDevice].systemVersion;
	return [deviceType cStringUsingEncoding:NSUTF8StringEncoding];
}

#ifndef RT_NO_UDID
string GetDeviceID()
{
	UIDevice *device = [UIDevice currentDevice];
	NSString *uniqueIdentifier = [device uniqueIdentifier];
	return string([uniqueIdentifier cStringUsingEncoding:NSUTF8StringEncoding]);
}
#else
string GetDeviceID()
{
	return GetMacAddress();
}
#endif

string GetIdentiferForVender()
{

	if ([[UIDevice currentDevice] respondsToSelector:@selector(identifierForVendor)]) {
		// This is will run if it is iOS6
		return  string(  [[[[UIDevice currentDevice] identifierForVendor] UUIDString] cStringUsingEncoding:NSUTF8StringEncoding]);
	} else {
	   // This is will run before iOS6 and you can use openUDID or other 
	   // method to generate an identifier
	}
	
	return "";
}

string GetAdvertisingIdentifier()
{
    if (!NSClassFromString(@"ASIdentifierManager")) 
    {
        return "";
    }
    
    //note: This might return 00000000-0000-0000-0000-000000000000 in iOS 6.0, this is an Apple bug that was fixed later I guess
    
    #ifdef RT_IAP_SUPPORT
    
    return string ([[[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString] cStringUsingEncoding:NSUTF8StringEncoding]);
    #else
    
    return "ERROR: Compile with RT_IAP_SUPPORT defined and the AdSupport.framework added";
    #endif
}

string GetNetworkType()
{
    Reachability *reachability = [Reachability reachabilityForInternetConnection];
    [reachability startNotifier];
    
    NetworkStatus status = [reachability currentReachabilityStatus];
    string response= "NULL";
    
    if(status == NotReachable)
    {
        response = "none";
        
    }
    else if (status == ReachableViaWiFi)
    {
        //WiFi
        response = "wifi";
    }
    else if (status == ReachableViaWWAN)
    {
        response = "mobile";
    }
    return response;
}



//Code taken from http://www.mobiledev.nl/udid-usage-rejected-by-apple-for-ios-apps

//Returns blank string on error

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

string GetRegionString()
{
	NSLocale *currentUsersLocale = [NSLocale currentLocale];
	NSLog(@"Current Locale: %@", [currentUsersLocale localeIdentifier]);
	return [[currentUsersLocale localeIdentifier] cStringUsingEncoding:NSUTF8StringEncoding];
}

bool IsAppInstalled(string packageName)
{
	#ifdef _DEBUG
	  LogMsg("IsAppInstalled not yet handled for this OS");
	#endif
	return false;
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
	
#ifdef _DEBUG
    LogMsg("Scanning dir %s", path.c_str());
#endif
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
#ifdef _DEBUG
        NSLog (@"%d: <%@>", i, [origContents objectAtIndex:i]);
#endif
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
	return true;
}

float GetDeviceOSVersion()
{
	//TODO
	return 0.0f;
}

string GetClipboardText()
{
	string text;
	
	UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];

	if ([pasteboard containsPasteboardTypes: [NSArray arrayWithObject:@"public.utf8-plain-text"]])
	{
		text = [pasteboard.string cStringUsingEncoding:NSUTF8StringEncoding];
	}
	return text;
}

bool IsDesktop() {return false;}

ePlatformID GetPlatformID()
{
	return PLATFORM_ID_IOS;
}
void NotifyOSOfOrientationPreference(eOrientationMode orientation)
{

}

bool HasVibration()
{
	if (IsIphone()) return true;

	return false;
}

void ForceVideoUpdate()
{
//    LogMsg("TODO..video update");
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
