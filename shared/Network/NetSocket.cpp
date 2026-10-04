#include "PlatformPrecomp.h"
#include "NetSocket.h"
#include "HostResolver.h"
#include "util/MiscUtils.h"

#ifndef WINAPI
	#include <sys/types.h> 
	#include <sys/socket.h>
	#include <sys/wait.h> 
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h> 
	#include <arpa/inet.h>

#ifdef ANDROID_NDK

#include <fcntl.h>

#elif defined(PLATFORM_BBX) || defined(PLATFORM_PSP2)
#include <fcntl.h>

#else
	#include <fcntl.h> //not sys/fcntl.h, emscripten warns about that spelling
#endif

#ifndef PLATFORM_PSP2
#include <sys/ioctl.h>
#endif

#define INVALID_SOCKET  (~0)
#define rt_closesocket(x) close(x)

#if defined(RT_WEBOS_ARM) || defined(ANDROID_NDK) || defined (RTLINUX)
	#include <linux/sockios.h>
	#include <errno.h>

#elif defined (PLATFORM_FLASH)

#include <sys/sockio.h>
#include <sys/errno.h>

#elif defined (PLATFORM_HTML5)
#include <errno.h> //not sys/errno.h, emscripten warns about that spelling
#elif defined (PLATFORM_PSP2)
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#else
	//default
	#include <sys/sockio.h>
#endif



#else

#include <ws2tcpip.h>
#include <mstcpip.h> //for tcp_keepalive / SIO_KEEPALIVE_VALS
#pragma comment(lib, "Ws2_32.lib")

#ifndef ECONNREFUSED
	#define ECONNREFUSED            WSAECONNREFUSED
#endif

	#define rt_closesocket(x) closesocket(x)

#endif

NetSocket::NetSocket()
{
	m_socket = (int)INVALID_SOCKET; //it's 64 bit on win64 but truncating to -1 compares back fine
	m_bWasDisconnected = false;
}

NetSocket::~NetSocket()
{
	Kill();
}

#define C_NET_MAXHOSTNAME 254

void NetSocket::Kill()
{
	m_bWasDisconnected = false;

	if (m_socket != INVALID_SOCKET)
	{
		#ifdef _DEBUG
		//LogMsg("Killed socket %d", m_socket);
		#endif
		rt_closesocket(m_socket);
		m_socket = (int)INVALID_SOCKET;
	}

	m_readBuffer.clear();
	m_writeBuffer.clear();

}


//Convert a struct sockaddr address to a string, IPv4 and IPv6:
#ifdef RT_IPV6
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
	switch(sa->sa_family) {
		case AF_INET:
			inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
				s, (socklen_t)maxlen);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
				s, (socklen_t)maxlen);
			break;

		default:
			strncpy(s, "Unknown AF", maxlen);
			return NULL;
	}

	return s;
}
#endif

bool NetSocket::Init( string url, int port )
{
	Kill();
	//connect to another one

	m_idleTimer = m_idleReadTimer = GetSystemTimeTick();

	//the host's addresses: from the resolver's cache when an earlier Init or
	//an app's HostResolver::Prefetch put them there, else resolved here and
	//now, which blocks (most of a second for an mDNS name like hal.local:
	//that is the stall Prefetch exists to avoid, see HostResolver.h)
	std::vector<HostResolver::Address> addrs;
	if (!HostResolver::Lookup(url, addrs))
	{
		int err = HostResolver::ResolveNow(url, addrs);
		if (err != 0 || addrs.empty())
		{
			if (m_loggingEnabled) LogMsg("NetSocket: can't resolve %s (getaddrinfo error %d)", url.c_str(), err);
			return false;
		}
		if (m_loggingEnabled) LogMsg("NetSocket: resolved %s on the main thread (not prefetched)", url.c_str());
	}

	//the first address that takes a socket, IPv4 preferred: an RT_IPV6 build
	//gets IPv6 addresses in the list too and only tries one when there is no
	//IPv4 address (very bad idea if you want ipv6 support, as it always was)
	bool bHaveIPv4 = false;
	for (size_t i = 0; i < addrs.size(); i++)
		if (addrs[i].family == AF_INET) bHaveIPv4 = true;

	for (size_t i = 0; i < addrs.size(); i++)
	{
		const HostResolver::Address &a = addrs[i];
		if (bHaveIPv4 && a.family != AF_INET)
			continue;

		m_socket = (int)socket(a.family, a.socktype, a.protocol);
		if (m_socket < 0)
		{
			m_socket = (int)INVALID_SOCKET;
			continue;
		}

#ifdef WINAPI
		//Make the socket non-blocking directly.  This used to be done as a side
		//effect of WSAAsyncSelect(m_socket, GetForegroundWindow(), ...), but that
		//fails whenever our window isn't the foreground window (the handle then
		//belongs to another process), silently leaving the socket BLOCKING and
		//freezing the main thread inside recv().  Nothing ever handled the
		//WM_USER+1 messages anyway; NetHTTP/NetSocket poll from Update().
		{
			u_long nonBlocking = 1;
			ioctlsocket(m_socket, FIONBIO, &nonBlocking);
		}
#else
		fcntl(m_socket, F_SETFL, O_NONBLOCK);
#endif

		//the cached address carries no port
		struct sockaddr_storage sa;
		memset(&sa, 0, sizeof(sa));
		memcpy(&sa, a.addr, a.addrLen);
		if (sa.ss_family == AF_INET)
			((struct sockaddr_in *)&sa)->sin_port = htons((unsigned short)port);
#ifdef RT_IPV6
		else if (sa.ss_family == AF_INET6)
			((struct sockaddr_in6 *)&sa)->sin6_port = htons((unsigned short)port);
#endif

		//a non-blocking connect reports "in progress" as an error: that is the
		//expected outcome, the connection completes on its own
		int ret = connect(m_socket, (struct sockaddr *)&sa, a.addrLen);
		if (ret != 0)
		{
#ifdef WINAPI
			int err = WSAGetLastError();
			bool bInProgress = (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS);
#else
			int err = errno;
			bool bInProgress = (err == EINPROGRESS || err == EWOULDBLOCK || err == EAGAIN);
#endif
			if (!bInProgress)
			{
				if (m_loggingEnabled) LogError("Socket connect error: %d", err);
				rt_closesocket(m_socket);
				m_socket = (int)INVALID_SOCKET;
				continue;
			}
		}
		return true;
	}

	if (m_loggingEnabled) LogError("Failed to connect to %s", url.c_str());
	return false;
}

bool NetSocket::InitHost( int port, int connections )
{
	Kill();

	sockaddr_in sa;

	memset(&sa, 0, sizeof(sa));

	sa.sin_family = PF_INET;             
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(port);          
	m_socket = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET )
	{
		if (m_loggingEnabled) LogMsg("socket command: INVALID_SOCKET");
		return false;
	}

	//Allow re-binding the port immediately after the app closes/restarts instead of waiting
	//out the kernel's TIME_WAIT (which otherwise makes a quick restart fail to bind).
	{
		int reuse = 1;
#ifdef WINAPI
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
#else
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
	}

	//u_long arg = 1;
	
	
	//ioctlsocket(m_socket, FIONBIO, &arg);

	/* bind the socket to the internet address */
	if (::bind(m_socket, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		rt_closesocket(m_socket);
		Kill();
		if (m_loggingEnabled) LogMsg("bind: INVALID_SOCKET");
		return false;
	}


#ifdef WINAPI

	//u_long iMode = 0;
	//ioctlsocket(m_socket, FIOASYNC, &iMode);
    ULONG NonBlock;
	
	NonBlock = 1;
	if (ioctlsocket(m_socket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		if (m_loggingEnabled) LogError("ioctlsocket() failed \n");
		return false;
	}

#pragma warning(suppress:4996) //WSAAsyncSelect is deprecated but the window-message based notify is what we want here
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER + 1, FD_READ | FD_WRITE | FD_CONNECT | FD_OOB);

#else
	//int x;
	//x=fcntl(m_socket,F_GETFL,0);
	//fcntl(m_socket,F_SETFL,x | O_NONBLOCK);
	fcntl(m_socket, F_SETFL, O_NONBLOCK);
	

#endif


	listen(m_socket, connections); 
	return true;
}


void NetSocket::SetSocket( int socket )
{
	Kill();
	m_socket = socket;
	//Initialize BOTH idle timers - SetSocket() is used for accepted (incoming) connections,
	//and callers that cull dead peers rely on GetIdleReadTimeMS() being meaningful from the
	//start.  Without this, m_idleReadTimer is left at its (often zero) default and the very
	//first idle check would think the connection had been silent since the epoch.
	m_idleTimer = m_idleReadTimer = GetSystemTimeTick();
#ifdef WINAPI
	// Disable Nagle's algorithm for low latency on accepted connections
	int flag = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));

	// Enable TCP keepalive so a peer that drops off the network (e.g. a WiFi device that
	// roams away or powers off) is detected at the OS level instead of leaving a half-open
	// socket that recv() never reports.  Tune it to probe aggressively (~5s idle, 1s apart)
	// so detection happens in seconds rather than the default ~2 hours.
	int keepAlive = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive));

	struct tcp_keepalive ka = {};
	ka.onoff = 1;
	ka.keepalivetime = 5000;     //start probing after 5s of idle
	ka.keepaliveinterval = 1000; //then probe every 1s
	DWORD bytesReturned = 0;
	WSAIoctl(m_socket, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), NULL, 0, &bytesReturned, NULL, NULL);
#else
	fcntl(m_socket, F_SETFL, O_NONBLOCK);
	int flag = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

	//Same intent as the Windows branch: turn on keepalive and (where supported) tighten the
	//probe timing so half-open connections die quickly.
	int keepAlive = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));
#ifdef TCP_KEEPIDLE
	int keepIdle = 5;
	setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle));
#endif
#ifdef TCP_KEEPINTVL
	int keepIntvl = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepIntvl, sizeof(keepIntvl));
#endif
#ifdef TCP_KEEPCNT
	int keepCnt = 3;
	setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPCNT, &keepCnt, sizeof(keepCnt));
#endif
#endif

}

string NetSocket::GetClientIPAsString()
{
	if (m_socket == INVALID_SOCKET) return "NOT CONNECTED";

	sockaddr_storage addr{};
#ifdef WIN32
	//avoid needing ws2tcpip.h
	int addrsize = sizeof(addr);
#else
	//linux
	socklen_t addrsize = sizeof(addr);
#endif

	//Note - Preserve old behaviour where getpeername() failure will return 0.0.0.0 so code relying on that won't break (though it would break on windows already even with the old version, but hey!)

	if (getpeername(m_socket, (sockaddr*)&addr, &addrsize) != 0)
	{
#ifdef WIN32
		//int err = WSAGetLastError();
#else
		//int err = errno;
#endif
		return "0.0.0.0";
	}
	
	char ipBuf[INET6_ADDRSTRLEN]{};

	switch (addr.ss_family)
	{
	case AF_INET6:
	{
		//Get IPv6
		sockaddr_in6* pIn6 = (sockaddr_in6*)&addr;
		const char* ntopResult = inet_ntop(AF_INET6, &pIn6->sin6_addr, ipBuf, sizeof(ipBuf));

		if (!ntopResult)
		{
			return "0.0.0.0";
		}

		break;
	}
	case AF_INET:
	{
		//Get IPv4
		sockaddr_in* pIn = (sockaddr_in*)&addr;
		
		const char* ntopResult = inet_ntop(AF_INET, &pIn->sin_addr, ipBuf, sizeof(ipBuf));

		if (!ntopResult)
		{
			return "0.0.0.0";
		}

		break;
	}
	default:
	{
		return "Unknown socket family";
	}
	}

	return std::string(ipBuf);
}

void NetSocket::Update()
{	
	UpdateRead();
	UpdateWrite();
}

void NetSocket::UpdateRead()
{
	if (m_socket == INVALID_SOCKET) return;
		
	vector <char> buff;
	buff.resize(1024);
	int bytesRead;

	do
	{
		bytesRead = (int)::recv (m_socket, &buff[0], (int)buff.size(), 0);
	
		if (bytesRead == 0)
		{
			//all done
			
			if (!m_bWasDisconnected)
			{
				#ifdef _DEBUG
					if (m_loggingEnabled) LogMsg("Client disconnected.  Buffer size is %d", (int)m_readBuffer.size());
				#endif
				m_bWasDisconnected = true;
			}
			return;
		}

		if (bytesRead == -1)
		{
			return;
			//not ready
		}

		//copy it into our real lbuffer
		m_readBuffer.insert(m_readBuffer.end(), buff.begin(), buff.begin()+bytesRead);
		#ifdef _DEBUG
		//LogMsg("Read %d bytes", bytesRead);
#ifdef WIN32
		//LogMsg(&buff[0]);  //can't really do this, because %'s will crash it
		//OutputDebugString(&buff[0]);
		//OutputDebugString("\n");
#endif
		#endif
		m_idleTimer = m_idleReadTimer = GetSystemTimeTick();

	} while (bytesRead >= int(buff.size()));

}

void NetSocket::UpdateWrite()
{
	
	if (m_socket == INVALID_SOCKET || m_writeBuffer.empty()) return;

	int bytesWritten = (int)::send (m_socket, &m_writeBuffer[0], (int)m_writeBuffer.size(), 0);

	if (bytesWritten <= 0)
	{
		//socket probably hasn't connected yet
		return;
	}
	m_writeBuffer.erase(m_writeBuffer.begin(), m_writeBuffer.begin()+bytesWritten);
	m_idleTimer = GetSystemTimeTick();

#ifdef _DEBUG
	//LogMsg("wrote %d, %d left", bytesWritten, m_writeBuffer.size());
#endif
}

void NetSocket::Write( const string &msg )
{
	if (msg.empty()) return;
	m_writeBuffer.insert(m_writeBuffer.end(), msg.begin(), msg.end());
	UpdateWrite();
}

void NetSocket::Write( char *pBuff, int len )
{
	m_writeBuffer.resize(m_writeBuffer.size()+len);
	memcpy(&m_writeBuffer[m_writeBuffer.size()-len], pBuff, len);
	
	UpdateWrite();

}



int NetSocket::GetIdleTimeMS()
{
	return GetSystemTimeTick()-m_idleTimer;
}


int NetSocket::GetIdleReadTimeMS()
{
	return GetSystemTimeTick()-m_idleReadTimer;
}


//*** HostResolver (HostResolver.h): host names resolved on a worker thread
//and cached, so Init above rarely has to block on a lookup. It lives in
//this file so that no project has to add a source file for it: every app
//that compiles NetSocket.cpp gets it. Platforms without threads (HTML5)
//resolve in place

#include <map>
#include <mutex>
#include <chrono>
#include <memory>
#if !defined(PLATFORM_HTML5)
	#define RT_HOSTRESOLVER_THREADS
	#include <thread>
	#include <atomic>
#endif

namespace
{
	struct HostEntry
	{
		std::vector<HostResolver::Address> addrs;
		std::chrono::steady_clock::time_point stamp; //when addrs was last set (or a refresh last failed)
		bool bPending = false;                       //a background lookup is in flight
		int lastError = 0;
	};

#ifdef RT_HOSTRESOLVER_THREADS
	//a lookup's thread, kept joinable: a thread that outlives the CRT's exit
	//crashes it, so Shutdown joins them all, and Prefetch reaps the finished
	//ones (a finished thread joins at once)
	struct Worker
	{
		std::thread thread;
		std::shared_ptr<std::atomic<bool> > done;
	};
#endif

	struct HostCache
	{
		std::mutex mutex;
		std::map<std::string, HostEntry> entries;
		unsigned int refreshAgeMS = 60000;
		int pending = 0; //lookups in flight
#ifdef RT_HOSTRESOLVER_THREADS
		std::vector<Worker> workers;
#endif
	};

	//made on first use, freed by Shutdown once every worker has been joined
	//(a worker that had to be left running keeps it alive instead: the
	//engine's Debug leak report then names it, which is the right outcome)
	HostCache *g_pHostCache = NULL;
	HostCache * GetCache()
	{
		if (!g_pHostCache)
			g_pHostCache = new HostCache;
		return g_pHostCache;
	}

	//no engine helpers here: this runs on the worker too
	std::string KeyOf(const std::string &host)
	{
		std::string key = host;
		for (size_t i = 0; i < key.length(); i++)
			key[i] = (char)tolower((unsigned char)key[i]);
		return key;
	}

	int ResolveWithGetaddrinfo(const std::string &host, std::vector<HostResolver::Address> &out)
	{
		out.clear();
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
#ifdef RT_IPV6
		hints.ai_family = AF_UNSPEC;
#else
		hints.ai_family = AF_INET;
#endif
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		struct addrinfo *pInfo = NULL;
		int rv = getaddrinfo(host.c_str(), NULL, &hints, &pInfo);
		if (rv != 0)
			return rv;
		for (struct addrinfo *p = pInfo; p != NULL; p = p->ai_next)
		{
			if (!p->ai_addr || p->ai_addrlen > sizeof(HostResolver::Address().addr))
				continue;
			HostResolver::Address a;
			a.family = p->ai_family;
			a.socktype = p->ai_socktype;
			a.protocol = p->ai_protocol;
			memcpy(a.addr, p->ai_addr, p->ai_addrlen);
			a.addrLen = (int)p->ai_addrlen;
			out.push_back(a);
		}
		freeaddrinfo(pInfo);
		return out.empty() ? -1 : 0;
	}

	void Store(const std::string &key, const std::vector<HostResolver::Address> &addrs, int err)
	{
		HostCache *pCache = GetCache();
		std::lock_guard<std::mutex> lock(pCache->mutex);
		HostEntry &e = pCache->entries[key];
		if (e.bPending)
		{
			e.bPending = false;
			pCache->pending--;
		}
		e.lastError = err;
		//a lookup that failed keeps the old answer (stale beats a stall) and
		//waits another refresh period before it is tried again
		if (err == 0 && !addrs.empty())
			e.addrs = addrs;
		e.stamp = std::chrono::steady_clock::now();
	}

#ifdef RT_HOSTRESOLVER_THREADS
	//joins the workers whose lookup has landed (instant) and drops them;
	//with the cache's mutex held
	void ReapFinishedWorkers(HostCache *pCache)
	{
		for (size_t i = 0; i < pCache->workers.size();)
		{
			if (pCache->workers[i].done->load())
			{
				if (pCache->workers[i].thread.joinable())
					pCache->workers[i].thread.join();
				pCache->workers.erase(pCache->workers.begin() + i);
			}
			else
				i++;
		}
	}
#endif
}

bool HostResolver::Lookup(const std::string &host, std::vector<Address> &out)
{
	out.clear();
	std::string key = KeyOf(host);
	HostCache *pCache = GetCache();
	bool bStale = false;
	{
		std::lock_guard<std::mutex> lock(pCache->mutex);
		std::map<std::string, HostEntry>::iterator it = pCache->entries.find(key);
		if (it == pCache->entries.end() || it->second.addrs.empty())
			return false;
		out = it->second.addrs;
		long long ageMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - it->second.stamp).count();
		bStale = ageMS > (long long)pCache->refreshAgeMS && !it->second.bPending;
	}
	if (bStale)
		Prefetch(host);
	return true;
}

int HostResolver::ResolveNow(const std::string &host, std::vector<Address> &out)
{
	int err = ResolveWithGetaddrinfo(host, out);
	Store(KeyOf(host), out, err);
	return err;
}

void HostResolver::Prefetch(const std::string &host)
{
	if (host.empty())
		return;
	std::string key = KeyOf(host);
	HostCache *pCache = GetCache();
	{
		std::lock_guard<std::mutex> lock(pCache->mutex);
		HostEntry &e = pCache->entries[key];
		if (e.bPending)
			return;
		e.bPending = true;
		pCache->pending++;
	}

#ifdef RT_HOSTRESOLVER_THREADS
	try
	{
		Worker w;
		w.done = std::make_shared<std::atomic<bool> >(false);
		std::shared_ptr<std::atomic<bool> > done = w.done;
		std::string h = host;
		w.thread = std::thread([h, key, done]()
		{
			std::vector<Address> addrs;
			int err = ResolveWithGetaddrinfo(h, addrs);
			Store(key, addrs, err);
			done->store(true);
		});
		std::lock_guard<std::mutex> lock(pCache->mutex);
		ReapFinishedWorkers(pCache);
		pCache->workers.push_back(std::move(w));
		return;
	}
	catch (...)
	{
		//no thread to be had: resolve here instead
	}
#endif
	std::vector<Address> addrs;
	ResolveNow(host, addrs);
}

bool HostResolver::Shutdown(unsigned int maxMS)
{
#ifdef RT_HOSTRESOLVER_THREADS
	std::vector<Worker> workers;
	{
		HostCache *pCache = GetCache();
		std::lock_guard<std::mutex> lock(pCache->mutex);
		workers.swap(pCache->workers);
	}
	bool bAllJoined = true;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	for (size_t i = 0; i < workers.size(); i++)
	{
		std::thread &t = workers[i].thread;
		if (!t.joinable())
			continue;
#ifdef WINAPI
		long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		long long left = (long long)maxMS - elapsed;
		if (left < 0) left = 0;
		if (WaitForSingleObject((HANDLE)t.native_handle(), (DWORD)left) == WAIT_OBJECT_0)
			t.join();
		else
		{
			t.detach(); //still inside getaddrinfo: the caller skips WSACleanup
			bAllJoined = false;
		}
#else
		(void)maxMS;
		t.join(); //no timed join here; a lookup ends within seconds anyway
#endif
	}
	if (bAllJoined && g_pHostCache)
	{
		delete g_pHostCache; //nothing can touch it now
		g_pHostCache = NULL;
	}
	return bAllJoined;
#else
	(void)maxMS;
	delete g_pHostCache;
	g_pHostCache = NULL;
	return true;
#endif
}

bool HostResolver::IsPending(const std::string &host)
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	std::map<std::string, HostEntry>::iterator it = pCache->entries.find(KeyOf(host));
	return it != pCache->entries.end() && it->second.bPending;
}

bool HostResolver::IsCached(const std::string &host)
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	std::map<std::string, HostEntry>::iterator it = pCache->entries.find(KeyOf(host));
	return it != pCache->entries.end() && !it->second.addrs.empty();
}

void HostResolver::Forget(const std::string &host)
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	std::map<std::string, HostEntry>::iterator it = pCache->entries.find(KeyOf(host));
	if (it == pCache->entries.end())
		return;
	if (it->second.bPending)
		it->second.addrs.clear(); //the lookup in flight will refill it
	else
		pCache->entries.erase(it);
}

int HostResolver::GetPendingCount()
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	return pCache->pending;
}

void HostResolver::SetRefreshAgeMS(unsigned int ms)
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	pCache->refreshAgeMS = ms;
}

std::string HostResolver::GetDebugText()
{
	HostCache *pCache = GetCache();
	std::lock_guard<std::mutex> lock(pCache->mutex);
	std::string s = "hosts:";
	int pending = 0;
	for (std::map<std::string, HostEntry>::iterator it = pCache->entries.begin(); it != pCache->entries.end(); ++it)
	{
		s += " " + it->first;
		if (it->second.addrs.empty())
			s += it->second.bPending ? "(resolving)" : "(unresolved)";
		if (it->second.bPending)
			pending++;
	}
	if (pCache->entries.empty())
		s += " none";
	if (pending)
		s += " (" + std::to_string(pending) + " lookup" + (pending == 1 ? "" : "s") + " in flight)";
	return s;
}
