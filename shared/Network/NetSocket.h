//  ***************************************************************
//  NetSocket - Creation date: 06/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef NetSocket_h__
#define NetSocket_h__

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

class NetSocket
{
public:
	NetSocket();
	virtual ~NetSocket();
	bool Init(std::string url, int port); //for clients
	bool InitHost(int port, int connections); //for host sockets
	void Update();
	void Write(char *pBuff, int len);
	void Write(const std::string &msg);
	int GetSocket() {return m_socket;}
	std::vector<char> & GetBuffer() {return m_readBuffer;}
	int GetIdleTimeMS(); //includes both reading and writing
	int GetIdleReadTimeMS();

	void Kill();
	void SetSocket(int socket);
	bool WasDisconnected() {return m_bWasDisconnected;}
	std::string GetClientIPAsString();

protected:

	void UpdateRead();
	void UpdateWrite();

	int m_socket;
	
	std::vector<char> m_readBuffer;
	std::vector<char> m_writeBuffer;
	unsigned int m_idleTimer, m_idleReadTimer; //time of last communication
	bool m_bWasDisconnected;

private:
};

#endif // NetSocket_h__