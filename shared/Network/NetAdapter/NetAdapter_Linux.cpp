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
#if !defined(NET_ADAPTER_STUB)
#if defined(__linux__) // Linux (not Mac or BSD) //
// - ------------------------------------------------------------------------------------------ - //
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>				// IFF_BROADCAST
#include <sys/socket.h>
#include <linux/if_packet.h>	// sockaddr_ll
#include <netdb.h>
// - ------------------------------------------------------------------------------------------ - //
// http://www.kernel.org/doc/man-pages/online/pages/man3/getifaddrs.3.html
// http://stackoverflow.com/questions/6762766/mac-address-with-getifaddrs
// - ------------------------------------------------------------------------------------------ - //
#include "NetAdapter.h"
#include "NetAdapter_Internal.h"
// - ------------------------------------------------------------------------------------------ - //

// - ------------------------------------------------------------------------------------------ - //
#include <stdio.h>
#include <stdlib.h>
// - ------------------------------------------------------------------------------------------ - //
#define safe_sprintf snprintf
// - ------------------------------------------------------------------------------------------ - //


// - ------------------------------------------------------------------------------------------ - //
pNetAdapterInfo* new_pNetAdapterInfo() {
	ifaddrs* IFA;
	if ( getifaddrs( &IFA ) == 0 ) {

		// Count the number Interfaces with IPv4 addresses //
		size_t IPv4Count = 0;
		for( ifaddrs* Current = IFA; Current != 0; Current = Current->ifa_next ) {
			// If an IPv4 //
			if ( Current->ifa_addr->sa_family == AF_INET ) {
				IPv4Count++;
			}
		}
		
		// *** //
		
		// Allocate the pNetAdapterInfo's //
		pNetAdapterInfo* Adapters = new pNetAdapterInfo[IPv4Count+1];
	
		// Allocate NetAdapterInfos per IP //	
		for ( size_t idx = 0; idx < IPv4Count; idx++ ) {
			Adapters[idx] = new_NetAdapterInfo();	// Internal Function //
		}
		
		Adapters[IPv4Count] = 0; // Last one a null pointer //

		// *** //
		
		// Iterate though and populate data //
		size_t Index = 0;
		for( ifaddrs* Current = IFA; Current != 0; Current = Current->ifa_next ) {
			// If an IPv4 //
			if ( Current->ifa_addr->sa_family == AF_INET ) {
				// Copy IP as string //
				getnameinfo( Current->ifa_addr, sizeof(struct sockaddr_in), Adapters[Index]->IP, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );

				// Copy IP as byte array //
				sockaddr_in* SAI = (sockaddr_in*)Current->ifa_addr;
				const unsigned char* DataAddr = (const unsigned char*)&(SAI->sin_addr.s_addr);

				int* IPv4 = (int*)Adapters[Index]->Data.IPv4;
				*IPv4 = *(int*)DataAddr;
			
//				printf("%i.%i.%i.%i\n", Adapters[Index]->Data.IPv4[0], Adapters[Index]->Data.IPv4[1], Adapters[Index]->Data.IPv4[2], Adapters[Index]->Data.IPv4[3] );
			
				// Copy Name //
				safe_sprintf( Adapters[Index]->Name, sizeof(Adapters[Index]->Name), "%s", Current->ifa_name );
				
				// Copy Netmask //
				getnameinfo( Current->ifa_netmask, sizeof(struct sockaddr_in), Adapters[Index]->NetMask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
			
				// Copy Broadcast Address //
	            if ( Current->ifa_flags & IFF_BROADCAST ) {
					getnameinfo( Current->ifa_broadaddr, sizeof(struct sockaddr_in), Adapters[Index]->Broadcast, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
				}
				
				//if ( Current->ifa_flags & IFF_POINTOPOINT ) {
				//	getnameinfo( Current->ifa_dstaddr, sizeof(struct sockaddr_in), Adapters[Index]->Broadcast, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );					
				//}
				
				Index++;	
			}
		}
		
		// 2nd pass, extract other data (MAC Address, IPv6) //
		Index = 0;
		for( ifaddrs* Current = IFA; Current != 0; Current = Current->ifa_next ) {
			// If an AF_PACKET device (i.e. hardware device) //
			if ( Current->ifa_addr->sa_family == AF_PACKET ) {
				for( int idx=0; idx < IPv4Count; idx++ ) {
					if ( strcmp( Adapters[idx]->Name, Current->ifa_name ) == 0 ) {
						sockaddr_ll* s = (sockaddr_ll*)Current->ifa_addr;

						// NOTE: I'm assuming MAC address is 6 bytes long //
						memcpy( Adapters[idx]->Data.MAC, s->sll_addr, 6 );
						
						safe_sprintf( Adapters[idx]->MAC, sizeof(Adapters[idx]->MAC), "%02x:%02x:%02x:%02x:%02x:%02x",
							s->sll_addr[0],
							s->sll_addr[1],
							s->sll_addr[2],
							s->sll_addr[3],
							s->sll_addr[4],
							s->sll_addr[5]
							);
					}
				}
			}
			else if ( Current->ifa_addr->sa_family == AF_INET6 ) {
				// IPv6 Address //
			}
		}
		

		freeifaddrs( IFA );

		return Adapters;
	}
	
	return 0;
}
// - ------------------------------------------------------------------------------------------ - //
const NetAdapterInfo* get_primary_pNetAdapterInfo( const pNetAdapterInfo* Adapters ) {
	if ( Adapters ) {
		NetAdapterInfo* NotLocalHost = 0;
		static unsigned char LocalHostIPv4[] = {127,0,0,1};
		
		size_t Index = 0;
		while ( Adapters[Index] != 0 ) {
			// Assume "eth0" is the primary //
			if ( strcmp( Adapters[Index]->Name, "eth0" ) == 0 )
				return Adapters[Index];
			
			// If no "eth0", then assume the first non LocalHost address is primary //
			if ( NotLocalHost == 0 ) {
				int* a = (int*)LocalHostIPv4;
				int* b = (int*)Adapters[Index]->Data.IPv4;
				if ( *a != *b ) {
					NotLocalHost = Adapters[Index];
				}
			}
			
			Index++;
		};
		
		return NotLocalHost;
	}
	else {
		// Hey you goof! You gave me a null pointer! //
		return 0;
	}
}
// - ------------------------------------------------------------------------------------------ - //
#endif // __linux__ //
#endif // !NET_ADAPTER_STUB //
// - ------------------------------------------------------------------------------------------ - //
