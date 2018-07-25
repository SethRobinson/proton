#include "PlatformPrecomp.h"
#include "NetSocket.h"
#include "util/MiscUtils.h"

#ifndef WINAPI
	#include <sys/types.h> 
	#include <sys/socket.h>
	#include <sys/wait.h> 
	#include <netinet/in.h> 
	#include <netdb.h> 
	#include <arpa/inet.h>

#ifdef ANDROID_NDK

#include <fcntl.h>

#elif defined(PLATFORM_BBX)
#include <fcntl.h>

#else
	#include <sys/fcntl.h>
#endif


#include <sys/ioctl.h>
#define INVALID_SOCKET  (~0)
#define rt_closesocket(x) close(x)

#if defined(RT_WEBOS_ARM) || defined(ANDROID_NDK) || defined (RTLINUX) 
	#include <linux/sockios.h>
	#include <errno.h>

#elif defined (PLATFORM_FLASH)

#include <sys/sockio.h>
#include <sys/errno.h>

#elif defined (PLATFORM_HTML5)
#include <sys/errno.h>
#else
	//default
	#include <sys/sockio.h>
#endif



#else


#ifndef ECONNREFUSED
	#define ECONNREFUSED            WSAECONNREFUSED
#endif

	#define rt_closesocket(x) closesocket(x)

#endif

NetSocket::NetSocket()
{
	m_socket = INVALID_SOCKET;
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
		m_socket = INVALID_SOCKET;
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
				s, maxlen);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
				s, maxlen);
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

	//ipv6 way
#ifdef RT_IPV6

	struct addrinfo hints, *servinfo, *p;
	int rv;

	string stPort = toString(port);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((rv = getaddrinfo(url.c_str(), stPort.c_str(), &hints, &servinfo)) != 0) 
	{
		//LogMsg("getaddrinfo: %s", gai_strerror(rv));
		return false;
	}

	// loop through all the results and connect to the first we can
	int typeCount = 0;
	
    bool bPreferipv4 =false; //very bad idea if you want ipv6 support
	
	
	bool bipv4Exists = false;
	
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		typeCount++;
		if (p->ai_addr->sa_family == AF_INET)
			{
				bipv4Exists = true;
			}
		char str[INET6_ADDRSTRLEN];
		get_ip_str(p->ai_addr, str,INET6_ADDRSTRLEN);
		//LogMsg("(%s) IP %d: %s", url.c_str(), typeCount, str);
	}
	
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		
		if (bPreferipv4 && bipv4Exists)
			{
				if (p->ai_addr->sa_family != AF_INET)
				{
					//ignore ipv6 addresses
					continue;
				}
			}
		
		char str[INET6_ADDRSTRLEN];
		get_ip_str(p->ai_addr, str,INET6_ADDRSTRLEN);
		//LogMsg("Connecting to %s",  str);
		
	
		if (p->ai_protocol == IPPROTO_TCP)
		{
			//LogMsg("Protocol is TCP");
		} else
		{
			//LogMsg("Protocol is %d",p->ai_protocol );
		}

		if ((m_socket = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) 
		{
			//LogMsg("Skipping socket...");
			continue;
		}
		


#ifdef WINAPI

		//u_long iMode = 0;
		//ioctlsocket(m_socket, FIOASYNC, &iMode);
/*
		if (WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_READ) == SOCKET_ERROR)
		{
			LogMsg("Error setting socket FD_READ: %d", WSAGetLastError());
		}

		if (WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_WRITE) == SOCKET_ERROR)
		{
			LogMsg("Error setting socket FD_WRITE: %d", WSAGetLastError());
		}
		if (WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_CONNECT) == SOCKET_ERROR)
		{
			LogMsg("Error setting socket FD_CONNECT: %d", WSAGetLastError());
		}
		if (WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_OOB) == SOCKET_ERROR)
		{
			LogMsg("Error setting socket FD_OOB: %d", WSAGetLastError());
		} 
		*/

#else
		fcntl (m_socket, F_SETFL, O_NONBLOCK);

#endif

		if (connect(m_socket, p->ai_addr, p->ai_addrlen) == -1) 
		{

			if (errno != 115 && errno != 36) //EINPROGRESS is 115 or 36, depending.   but not defined on some platforms so doing it manually
			{
#ifdef WINAPI
				LogError("Socket connect error: %d?", WSAGetLastError());
#else
				LogMsg("Socket connect error on socket %d, error %d", m_socket, errno);
#endif

				rt_closesocket(m_socket);
				continue;
			} else
			{
				//it's not ready, but that's to be expected as we aren't blocking
			}
		}

		break; // if we get here, we must have connected successfully
	}
	if (p == NULL) 
	{
		// looped off the end of the list with no connection
		LogError("Failed to connect");
		return false;
	}

#else
	//old ipv4 way

	
	struct sockaddr_in sa;
	struct hostent     *hp;
	
	if ((hp= gethostbyname(url.c_str())) == NULL) 
	{
		
#ifndef PLATFORM_BBX
		//no errno on bbx.  Wait, why am I even setting this?  Does this matter?
		errno= ECONNREFUSED;                       
#endif
		return false;                             
	}

	memset(&sa,0,sizeof(sa));
	memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length);    
	sa.sin_family= hp->h_addrtype;
	sa.sin_port= htons((u_short)port);

	if ((m_socket= (int)socket(hp->h_addrtype,SOCK_STREAM,0)) < 0)    
		return false;

#ifdef WINAPI

	//u_long iMode = 0;
	//ioctlsocket(m_socket, FIOASYNC, &iMode);

	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_READ); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_WRITE); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_CONNECT); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_OOB); 

#else
		fcntl (m_socket, F_SETFL, O_NONBLOCK);
	
#endif
	
	int ret = connect(m_socket,(struct sockaddr *)&sa,sizeof sa);

    if (ret == -1)
    {
	  //um, it returns -1 when it works properly on Windows.  Docs wrong?!  Huh?!
      //  LogError("Couldn't open socket.");
      //  return false;
    }

#endif

	return true;
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
		LogMsg("socket command: INVALID_SOCKET");
		return false;
	}

	//u_long arg = 1;
	
	
	//ioctlsocket(m_socket, FIONBIO, &arg);

	/* bind the socket to the internet address */
	if (::bind(m_socket, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		rt_closesocket(m_socket);
		Kill();
		LogMsg("bind: INVALID_SOCKET");
		return false;
	}


#ifdef WINAPI

	//u_long iMode = 0;
	//ioctlsocket(m_socket, FIOASYNC, &iMode);
    ULONG NonBlock;
	
	NonBlock = 1;
	if (ioctlsocket(m_socket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		LogError("ioctlsocket() failed \n");
		return false;
	}
	
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_READ); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_WRITE); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_CONNECT); 
	WSAAsyncSelect(m_socket, GetForegroundWindow(), WM_USER+1, FD_OOB); 


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
	m_idleTimer = GetSystemTimeTick();
#ifndef WINAPI
	fcntl(m_socket, F_SETFL, O_NONBLOCK);
#endif

}

string NetSocket::GetClientIPAsString()
{
	if (m_socket == INVALID_SOCKET) return "NOT CONNECTED";

	sockaddr_in addr;
#ifdef WIN32
	//avoid needing ws2tcpip.h
	int addrsize = sizeof(addr);
#else
	//linux
	socklen_t addrsize = sizeof(addr);
#endif

	int result = getpeername(m_socket, (sockaddr*) &addr, &addrsize);
	//printf("Result = %d\n", result);
	char* ip = inet_ntoa(addr.sin_addr);
	int port = addr.sin_port;
	//printf("IP: %s ... PORT: %d\n", ip, port);
	return ip;

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
		bytesRead = ::recv (m_socket, &buff[0], (int)buff.size(), 0);
	
		if (bytesRead == 0)
		{
			//all done
			
			if (!m_bWasDisconnected)
			{
				#ifdef _DEBUG
					LogMsg("Client disconnected.  Buffer size is %d", m_readBuffer.size());
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

	int bytesWritten = ::send (m_socket, &m_writeBuffer[0], (int)m_writeBuffer.size(), 0);

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
