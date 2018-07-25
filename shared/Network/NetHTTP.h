//  ***************************************************************
//  NetHTTP - Creation date: 06/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
Example of usage:

Here is the basic flow of usage:

NetHTTP myNet;

myNet.Setup("rtsoft.com", 80, "prey/getscores.php", NetHTTP::END_OF_DATA_SIGNAL_HTTP);

//also add any number of "post" data pieces that you want the .php to handle:

string anyData = "somedata";

myNet.AddPostData("name", &anyData.at[0], anyData.length();

For a PUT, use AddPutData instead.

myNet.Start();

while (1)
{
	myNet.Update();


	if (myNet.GetError() != NetHTTP::ERROR_NONE) 
	{
		//Big error, show message
	}

	if (myNet.GetState() == NetHTTP::STATE_FINISHED)
	{
		//transaction is finished
		LogMsg("%s", myNet.GetDownloadedData()); //assuming it's a string.  Note that myNet will automatically add a null at the end.
	}

}

Note:  html5 builds automatically use NetHTTP_HTML5.cpp instead of NetHTTP.cpp.  The usage is the same but internally they use
emscripten_async_wget2_data.  Due to javascript security issues, you can only download files on website the .html is hosted on, unless
cross scripting privileges are setup.
*/

#ifndef NetHTTP_h__
#define NetHTTP_h__

#include "util/MiscUtils.h"
#include "NetSocket.h"

class NetHTTP
{
public:
	NetHTTP();
	virtual ~NetHTTP();

	enum eState
	{
		STATE_IDLE,
		STATE_ACTIVE,
		STATE_FINISHED,
		STATE_FORWARD, //about to forward to another URL
		STATE_ABORT
	};

	enum eEndOfDataSignal
	{
		END_OF_DATA_SIGNAL_RTSOFT_MARKER = 0, //sort of legacy, but if the characters in C_END_DOWNLOAD_MARKER_STRING are detected, 
											  //the connection is ended. If content-length is sent by the server, that is used also.
		END_OF_DATA_SIGNAL_HTTP //standard way, double linefeeds will end the connection. If content-length is sent by the server,
								//that is also used.
	};
	enum eError
	{
		ERROR_NONE,
		ERROR_CANT_RESOLVE_URL,
		ERROR_COMMUNICATION_TIMEOUT,
		ERROR_WRITING_FILE,
		ERROR_404_FILE_NOT_FOUND
	};

	void Setup(string serverName, int port, string query, eEndOfDataSignal eodSignal = END_OF_DATA_SIGNAL_RTSOFT_MARKER);
	bool AddPostData(const string &name, const byte *pData, int len=-1);
	bool AddPutData(const string data); //can only be set once.  Can't be used with AddPostData
	bool SetFileOutput(const string &fName); //call this before Start, allows you to save to a file instead of memory

	bool Start();
	
	eState GetState() {return m_state;}
	eError GetError() {return m_error;}

	int GetDownloadedBytes();
	int GetExpectedBytes() {return m_expectedFileBytes;} //0 if unknown
	const byte * GetDownloadedData();
	void Update();
	void Reset(bool bClearPostData=true); //completely clears it out so it can be used again

private:

#ifdef PLATFORM_HTML5

	static void onLoaded( unsigned int handle, void* parent, void * file, unsigned int byteSize);
	static void onError(unsigned int handle, void* parent, int statuserror, const char *message);
	static void onProgress(unsigned int handle, void* parent, int bytesDownloaded, int totalBytes);

	void SetBuffer(const char *pData, int byteSize);
	void SetProgress(int bytesDownloaded, int totalBytes);
	void KillConnectionIfNeeded();

	int m_emscriptenWgetHandle;
#endif

	string BuildHTTPHeader();
	void FinishDownload();
	void OnError(eError e);
	int ScanDownloadedHeader(); //returns http result code

	string m_serverName; //eg, "www.rtsoft.com"
	string m_query;  //eg, "crap/index.html"
	string m_replyHeader; //store what the server sends us
	int m_port;
	eEndOfDataSignal m_endOfDataSignal;
	NetSocket m_netSocket;
	eState m_state;
	eError m_error;
	vector<char> m_downloadData; //holds the actual file/stream of what we've got
	string m_downloadHeader;
	uint32 m_expectedFileBytes; //0 if content-length is unknown
	string m_postData; //optional, for if we want to send post data
	unsigned int m_timer; //we scan for an end of mark, we don't want to do it too often
	int m_idleTimeOutMS;
	FILE *m_pFile; //if not null, we're putting out download into this
	int m_bytesWrittenToFile;
	string m_fileName;
	string m_contentType;
};

bool CheckCharVectorForString(vector<char> &v, string marker, int *pIndexOfMarkerEndPosOut=NULL);
#endif // NetHTTP_h__