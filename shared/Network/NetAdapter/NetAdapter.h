// - ------------------------------------------------------------------------------------------ - //
// NET ADAPTER -- A cross platform library for getting presentable Network Adapter information.
// By Mike Kasprzak -- http://www.toonormal.com http://www.sykhronics.com -- twitter @mikekasprzak
// http://code.google.com/p/net-adapter/
//
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either
// in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and
// by any means.
// 
// In jurisdictions that recognize copyright laws, the author or authors of this software dedicate
// any and all copyright interest in the software to the public domain. We make this dedication for
// the benefit of the public at large and to the detriment of our heirs and successors. We intend 
// this dedication to be an overt act of relinquishment in perpetuity of all present and future 
// rights to this software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
// - ------------------------------------------------------------------------------------------ - //
#ifndef __NET_ADAPTER_H__
#define __NET_ADAPTER_H__
// - ------------------------------------------------------------------------------------------ - //
// Important Notes: 
// 1. On Windows, this (and all network calls) must be made AFTER Winsock2 is initialized. If
//    you are "playing nice" with other networking libraries, you can let them init for you.
//    Otherwise, do something like this in your _WIN32 build:
//		#include <Winsock2.h>
//
//		void EnableNetworking() {
//			WSADATA wsaData;
//			WSAStartup( MAKEWORD(2,2), &wsaData );		// 2.2 is XP+, and Win95 with an update //
//      }
//		void DisableNetworking() {
//			WSACleanup();
//		}
// 2. The concept of the "primary" net adapter is actually a guess.
//    On Windows it's the first adapter with a DNS Server set.
//    On Linux it's either "eth0", or the first adapter without a localhost IP (127.0.0.1).
// 3. Net Adapter can be built with -DNET_ADAPTER_STUB to force usage of the stub (i.e. dummy code).
// - ------------------------------------------------------------------------------------------ - //
// // Usage 1: Get the Current only //
// pNetAdapterInfo* Adapters = new_pNetAdapterInfo();							// Get adapters //
// const NetAdapterInfo* Current = get_primary_pNetAdapterInfo( Adapters );		// Get primary adapter //
// printf( "%s: %s (%s)\n", Current->Name, Current->IP, Current->MAC );
// delete_pNetAdapterInfo( Adapters );											// Clean up //
//
// // Usage 2: Get all adapters //
// pNetAdapterInfo* Adapters = new_pNetAdapterInfo();
// size_t AdapterCount = count_pNetAdapterInfo(Adapters);
// for ( size_t Index = 0; Index < AdapterCount; Index++ ) {
//     const NetAdapterInfo* Current = get_pNetAdapterInfo( Adapters, Index );	// get_, not get_primary_ //
//     printf( "%i - %s: %s (%s)\n", Index, Current->Name, Current->IP, Current->MAC );
// }
// delete_pNetAdapterInfo( Adapters );
// - ------------------------------------------------------------------------------------------ - //
// Libraries Used:
//   Visual Studio (automatic): ws2_32.lib Iphlpapi.lib
//   MinGW: -lws2_32 -lIphlpapi (i.e. libws2_32.a libIphlpapi.a) 
//   Linux: ?
// - ------------------------------------------------------------------------------------------ - //
// Changelog:
// 0.01 -- Initial Release. 
//         Windows XP to Windows 8 (non Metro) support.
//         Linux support.
//         Thanks to Seth Robinson for his VC2005 fixes.
// - ------------------------------------------------------------------------------------------ - //
#include <string.h>							// size_t //
// - ------------------------------------------------------------------------------------------ - //
#define NET_ADAPTER_IP_LENGTH	(8*4)+7+1	// 8 hex 16bit numbers, 7 colons, null terminator //
#define NET_ADAPTER_MAC_LENGTH	(8*2)+7+1	// 8 hex 8bit numbers, 7 colons, null terminator //
// - ------------------------------------------------------------------------------------------ - //
struct NetAdapterInfo {
	char Name[128+1];						// A copy of its "nice" name. //
	
	char IP[NET_ADAPTER_IP_LENGTH];			// Enough for an IPv6 (though only v4 supported) //
	char MAC[NET_ADAPTER_MAC_LENGTH];		// Enough for an 8-part MAC. Usually 6 parts. //
	
	struct {
		// Data is in LSB first format. This is suitable for display, but may not match the data //
		//   formatting of your platforms socket library. //
		unsigned char IPv4[4];				// Typical IPv4 address as an array of bytes //
//		unsigned short IPv6[8];				// Typical IPv6 address as an array of shorts //
		unsigned char MAC[6];				// typical MAC address as an array of bytes //
	} Data;
	
	// Data that isn't available everywhere (or I haven't figured out yet) //
	// subnet mask -- Linux             -- i.e. 255.255.255.0 on a typical LAN
	// UDP broadcast address -- Linux   -- i.e. 192.168.0.255 on a typical LAN
	// UDP multicast address -- Windows -- i.e. 224.0.0.1, but there are often many
	// DNS Server -- Windows            -- i.e. 192.168.0.1 on a typical LAN
	
	char NetMask[NET_ADAPTER_IP_LENGTH];	// Linux/Unix Only //
	char Broadcast[NET_ADAPTER_IP_LENGTH];	// Linux/Unix Only (could be derived from NetMask) //
	
	char Description[256+1];				// Windows Only //
	char DNS[NET_ADAPTER_IP_LENGTH];		// Windows Only //
};
// - ------------------------------------------------------------------------------------------ - //
typedef NetAdapterInfo* pNetAdapterInfo;
// - ------------------------------------------------------------------------------------------ - //
pNetAdapterInfo* new_pNetAdapterInfo();									// Returns Multiple Adapters //
int delete_pNetAdapterInfo( pNetAdapterInfo* Adapters );				// Deletes those Adapters //

const size_t count_pNetAdapterInfo( const pNetAdapterInfo* Adapters );	// How Many Adapters //

// Both return a single Adapter (notice the missing "p" prefix) //
const NetAdapterInfo* get_pNetAdapterInfo( const pNetAdapterInfo* Adapters, const size_t Index );
const NetAdapterInfo* get_primary_pNetAdapterInfo( const pNetAdapterInfo* Adapters );
// - ------------------------------------------------------------------------------------------ - //
#endif // __NET_ADAPTER_H__
// - ------------------------------------------------------------------------------------------ - //
