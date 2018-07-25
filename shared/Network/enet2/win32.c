/** 
 @file  win32.c
 @brief ENet Win32 system specific functions
*/
#ifdef _WIN32

#define ENET_BUILDING_LIB 1
#include "enet2/enet.h"
#include <windows.h>
#include <mmsystem.h>
#include <ws2tcpip.h>

static enet_uint32 timeBase = 0;

int
enet_initialize (void)
{
    WORD versionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;
   
    if (WSAStartup (versionRequested, & wsaData))
       return -1;

    if (LOBYTE (wsaData.wVersion) != 1||
        HIBYTE (wsaData.wVersion) != 1)
    {
       WSACleanup ();
       
       return -1;
    }

    timeBeginPeriod (1);

    return 0;
}

void
enet_deinitialize (void)
{
    timeEndPeriod (1);

    WSACleanup ();
}

enet_uint32
enet_host_random_seed (void)
{
    return (enet_uint32) timeGetTime ();
}

enet_uint32
enet_time_get (void)
{
    return (enet_uint32) timeGetTime () - timeBase;
}

void
enet_time_set (enet_uint32 newTimeBase)
{
    timeBase = (enet_uint32) timeGetTime () - newTimeBase;
}

static enet_uint16
enet_af (ENetAddressFamily family)
{
    if (family == ENET_IPV4)
        return AF_INET;
    if (family == ENET_IPV6)
        return AF_INET6;
    return 0;
}

static socklen_t
enet_sa_size (ENetAddressFamily family)
{
    if (family == ENET_IPV4)
        return sizeof (struct sockaddr_in);
    if (family == ENET_IPV6)
        return sizeof (struct sockaddr_in6);
    return 0;
}

static ENetAddressFamily
enet_address_set_address (ENetAddress * address, const struct sockaddr * sin)
{
    memset (address, 0, sizeof (ENetAddress));
    if (sin -> sa_family == AF_INET)
    {
        address -> host = enet_address_map4 ((((struct sockaddr_in *) sin) -> sin_addr.s_addr));
        /* address -> scopeID = 0; */
        address -> port = ENET_NET_TO_HOST_16 (((struct sockaddr_in *) sin) -> sin_port);
        return ENET_IPV4;
    }
    if (sin -> sa_family == AF_INET6)
    {
        address -> host = * (ENetHostAddress *) & ((struct sockaddr_in6 *) sin) -> sin6_addr;
        address -> scopeID = ((struct sockaddr_in6 *) sin) -> sin6_scope_id;
        address -> port = ENET_NET_TO_HOST_16 (((struct sockaddr_in6 *) sin) -> sin6_port);
        return ENET_IPV6;
    }
    return ENET_NO_ADDRESS_FAMILY;
}

static int
enet_address_set_sin (struct sockaddr * sin, const ENetAddress * address, ENetAddressFamily family)
{
    memset (sin, 0, enet_sa_size(family));
    if (family == ENET_IPV4 &&
      (enet_get_address_family (address) == ENET_IPV4 ||
      !memcmp (& address -> host, & ENET_HOST_ANY, sizeof(ENetHostAddress))))
    {
        ((struct sockaddr_in *) sin) -> sin_family = AF_INET;
        ((struct sockaddr_in *) sin) -> sin_addr = * (struct in_addr *) & address -> host.addr[12];
        ((struct sockaddr_in *) sin) -> sin_port = ENET_HOST_TO_NET_16 (address -> port);
        return 0;
    }
    else if (family == ENET_IPV6)
    {
        ((struct sockaddr_in6 *) sin) -> sin6_family = AF_INET6;
        ((struct sockaddr_in6 *) sin) -> sin6_addr = * (struct in6_addr *) & address -> host;
        ((struct sockaddr_in6 *) sin) -> sin6_scope_id = address -> scopeID;
        ((struct sockaddr_in6 *) sin) -> sin6_port = ENET_HOST_TO_NET_16 (address -> port);
        return 0;
    }
    return -1;
}

int
enet_address_set_host (ENetAddress * address, const char * name)
{
    enet_uint16 port = address -> port;
    struct addrinfo hints;
    struct addrinfo * result;
    struct addrinfo * res;

    memset(& hints, 0, sizeof (hints));
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;

    if ( getaddrinfo(name, NULL, &hints, &result) )
        return -1;

    for (res = result; res != NULL; res = res -> ai_next)
    {
        if ( enet_address_set_address(address, res -> ai_addr) != ENET_NO_ADDRESS_FAMILY )
            break;
    }

    address -> port = port;
    freeaddrinfo(result);
    if (res == NULL) return -1;

    return 0;
}

static int
enet_address_get_host_x (const ENetAddress * address, char * name, size_t nameLength, int flags)
{
    struct sockaddr_storage sin;
    enet_address_set_sin((struct sockaddr *) & sin, address, ENET_IPV6);

    if ( getnameinfo((struct sockaddr *) & sin, enet_sa_size (ENET_IPV6), name, nameLength, NULL, 0, flags))
        return -1;

    return 0;
}

int
enet_address_get_host_ip (const ENetAddress * address, char * name, size_t nameLength)
{
    return enet_address_get_host_x(address, name, nameLength, NI_NUMERICHOST);
}

int
enet_address_get_host (const ENetAddress * address, char * name, size_t nameLength)
{
    return enet_address_get_host_x(address, name, nameLength, 0);
}

int
enet_socket_bind (ENetSocket socket, const ENetAddress * address, ENetAddressFamily family)
{
    struct sockaddr_storage sin;

    if (address != NULL)
    {
        enet_address_set_sin((struct sockaddr *) & sin, address, family);
    }
    else
    {
        ENetAddress address_ = { ENET_HOST_ANY_INIT, 0, 0 };
        enet_address_set_sin((struct sockaddr *) & sin, & address_, family);
    }

    return bind (socket, (struct sockaddr *) & sin, enet_sa_size(family)) == SOCKET_ERROR ? -1 : 0;
}

int
enet_socket_listen (ENetSocket socket, int backlog)
{
    return listen (socket, backlog < 0 ? SOMAXCONN : backlog) == SOCKET_ERROR ? -1 : 0;
}

ENetSocket
enet_socket_create (ENetSocketType type, ENetAddressFamily family)
{
    ENetSocket sock = socket (enet_af (family), type == ENET_SOCKET_TYPE_DATAGRAM ? SOCK_DGRAM : SOCK_STREAM, 0);
    return sock;
}

int
enet_socket_set_option (ENetSocket socket, ENetSocketOption option, int value)
{
    int result = SOCKET_ERROR;
    switch (option)
    {
        case ENET_SOCKOPT_NONBLOCK:
        {
            u_long nonBlocking = (u_long) value;
            result = ioctlsocket (socket, FIONBIO, & nonBlocking);
            break;
        }

        case ENET_SOCKOPT_BROADCAST:
            result = setsockopt (socket, SOL_SOCKET, SO_BROADCAST, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_REUSEADDR:
            result = setsockopt (socket, SOL_SOCKET, SO_REUSEADDR, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_RCVBUF:
            result = setsockopt (socket, SOL_SOCKET, SO_RCVBUF, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_SNDBUF:
            result = setsockopt (socket, SOL_SOCKET, SO_SNDBUF, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_RCVTIMEO:
            result = setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_SNDTIMEO:
            result = setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, (char *) & value, sizeof (int));
            break;

        case ENET_SOCKOPT_NODELAY:
            result = setsockopt (socket, IPPROTO_TCP, TCP_NODELAY, (char *) & value, sizeof (int));
            break;

        default:
            break;
    }
    return result == SOCKET_ERROR ? -1 : 0;
}

int
enet_socket_connect (ENetSocket socket, const ENetAddress * address, ENetAddressFamily family)
{
    struct sockaddr_storage sin;
    enet_address_set_sin((struct sockaddr *) & sin, address, family);

    return connect (socket, (struct sockaddr *) & sin, enet_sa_size (family)) == SOCKET_ERROR ? -1 : 0;
}

ENetSocket
enet_socket_accept (ENetSocket socket, ENetAddress * address, ENetAddressFamily family)
{
    SOCKET result;
    struct sockaddr_storage sin;
    socklen_t sinLength = enet_sa_size (family);

    result = accept (socket, 
                     address != NULL ? (struct sockaddr *) & sin : NULL,
                     address != NULL ? & sinLength : NULL);

    if (result == INVALID_SOCKET)
      return ENET_SOCKET_NULL;

    if (address != NULL)
    {
        enet_address_set_address(address, (struct sockaddr *) & sin);
    }

    return result;
}

int
enet_socket_shutdown (ENetSocket socket, ENetSocketShutdown how)
{
    return shutdown (socket, (int) how) == SOCKET_ERROR ? -1 : 0;
}

void
enet_socket_destroy (ENetSocket socket)
{
    if (socket != INVALID_SOCKET)
      closesocket (socket);
}

int
enet_socket_send (ENetSocket socket,
                  const ENetAddress * address,
                  const ENetBuffer * buffers,
                  size_t bufferCount,
                  ENetAddressFamily family)
{
    struct sockaddr_storage sin;
    DWORD sentLength;

    if (address != NULL)
    {
        enet_address_set_sin((struct sockaddr *) & sin, address, family);
    }

    if (WSASendTo (socket, 
                   (LPWSABUF) buffers,
                   (DWORD) bufferCount,
                   & sentLength,
                   0,
                   address != NULL ? (struct sockaddr *) & sin : 0,
                   address != NULL ? enet_sa_size (family) : 0,
                   NULL,
                   NULL) == SOCKET_ERROR)
    {
       if (WSAGetLastError () == WSAEWOULDBLOCK)
         return 0;

       return -1;
    }

    return (int) sentLength;
}

int
enet_socket_receive (ENetSocket socket,
                     ENetAddress * address,
                     ENetBuffer * buffers,
                     size_t bufferCount,
                     ENetAddressFamily family)
{
    INT sinLength = enet_sa_size (family);
    DWORD flags = 0,
          recvLength;
    struct sockaddr_storage sin;

    if (WSARecvFrom (socket,
                     (LPWSABUF) buffers,
                     (DWORD) bufferCount,
                     & recvLength,
                     & flags,
                     address != NULL ? (struct sockaddr *) & sin : NULL,
                     address != NULL ? & sinLength : NULL,
                     NULL,
                     NULL) == SOCKET_ERROR)
    {
       switch (WSAGetLastError ())
       {
       case WSAEWOULDBLOCK:
       case WSAECONNRESET:
          return 0;
       }

       return -1;
    }

    if (flags & MSG_PARTIAL)
      return -1;

    if (address != NULL)
    {
        enet_address_set_address(address, (struct sockaddr *) & sin);
    }

    return (int) recvLength;
}

int
enet_socketset_select (ENetSocket maxSocket, ENetSocketSet * readSet, ENetSocketSet * writeSet, enet_uint32 timeout)
{
    struct timeval timeVal;

    timeVal.tv_sec = timeout / 1000;
    timeVal.tv_usec = (timeout % 1000) * 1000;

    return select (maxSocket + 1, readSet, writeSet, NULL, & timeVal);
}

int
enet_socket_wait (ENetSocket socket4, ENetSocket socket6, enet_uint32 * condition, enet_uint32 timeout)
{
    fd_set readSet, writeSet;
    struct timeval timeVal;
    int selectCount;
    ENetSocket maxSocket;

    timeVal.tv_sec = timeout / 1000;
    timeVal.tv_usec = (timeout % 1000) * 1000;

    FD_ZERO (& readSet);
    FD_ZERO (& writeSet);

    if (* condition & ENET_SOCKET_WAIT_SEND)
    {
        if (socket4 != ENET_SOCKET_NULL)
            FD_SET (socket4, & writeSet);
        if (socket6 != ENET_SOCKET_NULL)
            FD_SET (socket6, & writeSet);
    }

    if (* condition & ENET_SOCKET_WAIT_RECEIVE)
    {
        if (socket4 != ENET_SOCKET_NULL)
            FD_SET (socket4, & readSet);
        if (socket6 != ENET_SOCKET_NULL)
            FD_SET (socket6, & readSet);
    }

    maxSocket = 0;
    if (socket4 != ENET_SOCKET_NULL)
        maxSocket = socket4;
    if (socket6 != ENET_SOCKET_NULL && socket6 > maxSocket)
        maxSocket = socket6;

    selectCount = select (maxSocket + 1, & readSet, & writeSet, NULL, & timeVal);

    if (selectCount < 0)
      return -1;

    * condition = ENET_SOCKET_WAIT_NONE;

    if (selectCount == 0)
      return 0;

    if ( (socket4 != ENET_SOCKET_NULL && FD_ISSET (socket4, & writeSet)) ||
        (socket6 != ENET_SOCKET_NULL && FD_ISSET (socket6, & writeSet)) )
        * condition |= ENET_SOCKET_WAIT_SEND;

    if ( (socket4 != ENET_SOCKET_NULL && FD_ISSET (socket4, & readSet)) ||
        (socket6 != ENET_SOCKET_NULL && FD_ISSET (socket6, & readSet)) )
        * condition |= ENET_SOCKET_WAIT_RECEIVE;

    return 0;
}

#endif

