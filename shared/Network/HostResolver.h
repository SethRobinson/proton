/*
 * HostResolver - host names resolved off the main thread and cached (Sep 2026).
 *
 * NetSocket::Init used to resolve its host name right there, with a blocking
 * gethostbyname/getaddrinfo. On a LAN with mDNS names (hal.local,
 * glados.local) Windows takes most of a second per lookup and caches the
 * answer only briefly, so every HTTP request an app started (an LLM turn, a
 * TTS chunk) froze the main thread for that long: RTGameBot measured 860 ms
 * per request. Now NetSocket::Init asks this cache first and connects at
 * once on a hit; a miss still resolves in place and stores the answer, so
 * nothing changes for a caller, but an app that calls Prefetch() for the
 * hosts it will talk to (at startup, when a setting names a new server)
 * never blocks at all: the lookup runs on a worker thread and lands here.
 * A hit older than the refresh age (60 s) starts a background refresh (one
 * per host in flight) and still returns the addresses it has, so a server
 * that moves is found again within a minute of use, and a refresh that
 * fails keeps the old answer (stale beats a stall).
 *
 * Threading: a worker thread only calls getaddrinfo and touches the cache
 * under its mutex; it never calls into the engine (not thread-safe) and
 * never logs. The workers are kept joinable and Shutdown() joins them and
 * frees the cache: a thread still running when the process exits crashes
 * the CRT on the way out (the Windows main calls Shutdown before
 * WSACleanup). Platforms without threads (HTML5) resolve in place.
 */

#ifndef HostResolver_h__
#define HostResolver_h__

#include <string>
#include <vector>

class HostResolver
{
public:

	//one address of a host, port-less: the connect sets the port
	struct Address
	{
		int family = 0, socktype = 0, protocol = 0;
		unsigned char addr[128]; //a sockaddr_storage
		int addrLen = 0;
	};

	//the cached addresses of a host, true on a hit. An entry older than
	//the refresh age is still returned, and a background refresh starts
	static bool Lookup(const std::string &host, std::vector<Address> &out);
	//resolve on this thread (blocking, up to a second on an mDNS name) and
	//cache the answer; getaddrinfo's error code, 0 when out has addresses
	static int ResolveNow(const std::string &host, std::vector<Address> &out);
	//start a background lookup of the host (nothing if one is in flight), so
	//a later NetSocket::Init finds it cached
	static void Prefetch(const std::string &host);
	static bool IsPending(const std::string &host); //a background lookup of it is in flight
	static bool IsCached(const std::string &host);  //an answer is on hand (fresh or stale)
	//joins the worker threads, waiting at most maxMS for one still inside
	//getaddrinfo; true when every one is gone. The platform main calls it
	//before WSACleanup and the CRT's exit: a worker that is still running
	//then (its own thread cleanup goes through the CRT) crashes the exit
	//(seen as a Debug CRT breakpoint), and Winsock torn down under it does
	//too. A worker that is still stuck afterwards is detached and the
	//caller should skip WSACleanup (the OS reclaims everything at exit)
	static bool Shutdown(unsigned int maxMS);
	static int GetPendingCount(); //lookups in flight
	static void Forget(const std::string &host);    //drop the entry: the next Init resolves anew
	static void SetRefreshAgeMS(unsigned int ms);   //how old a hit may be before a refresh starts (default 60 s)
	static std::string GetDebugText();              //"hosts: hal.local, glados.local (1 lookup in flight)"
};

#endif // HostResolver_h__
