#include "PlatformPrecomp.h"

/*

Note:  html5 builds automatically use NetHTTP_HTML5.cpp instead of NetHTTP.cpp.  The usage is the same but internally they use
emscripten_async_wget2_data.  Due to javascript security issues, you can only download files on website the .html is hosted on, unless
cross scripting privileges are setup.

*/

#include "Network/NetHTTP.h"
#include "Network/NetUtils.h"
#include "BaseApp.h"
#include "util/TextScanner.h"
#include <emscripten/emscripten.h>

#define NET_END_MARK_CHECK_DELAY_MS 333
#define C_END_DOWNLOAD_MARKER_STRING "RTENDMARKERBS1001"
#define C_DEFAULT_IDLE_TIMEOUT_MS (25*1000)


NetHTTP::NetHTTP()
{
	m_emscriptenWgetHandle = -1;
	m_pFile = NULL;
	Reset(true);
}

void NetHTTP::KillConnectionIfNeeded()
{
	if (m_emscriptenWgetHandle != -1)
	{
#ifdef _DEBUG
	LogMsg("Aborting emscripten wget handle %d",m_emscriptenWgetHandle );
#endif
		emscripten_async_wget2_abort(m_emscriptenWgetHandle);
		m_emscriptenWgetHandle = -1;
	}
}

NetHTTP::~NetHTTP()
{

	KillConnectionIfNeeded();

	if (m_pFile)
	{
		fclose(m_pFile);
		RemoveFile(m_fileName);
		m_pFile = NULL;
	}
}

void NetHTTP::Reset(bool bClearPostdata)
{

	KillConnectionIfNeeded();

	if (m_pFile)
	{
		fclose(m_pFile);
		RemoveFile(m_fileName);
		m_pFile = NULL;
	}

	m_endOfDataSignal = END_OF_DATA_SIGNAL_RTSOFT_MARKER; //don't change the default, Seth's software relies on it
	m_state = STATE_IDLE;
	m_error = ERROR_NONE;
	m_timer = 0;
	m_idleTimeOutMS = C_DEFAULT_IDLE_TIMEOUT_MS;
	m_expectedFileBytes= 0;
	m_downloadData.clear();
	m_replyHeader.clear();
	m_query.clear();
	if (bClearPostdata)
	{
		m_postData.clear();
	}
	m_bytesWrittenToFile = 0;
}

void NetHTTP::Setup( string serverName, int port, string query, eEndOfDataSignal eodSignal )
{
	if (!IsInString(serverName, "://") && (port == 80 || port == 443))
	{
		//add http so it won't freak later
		serverName = "https://"+serverName;
		port = 443;
	}
	
	m_endOfDataSignal = eodSignal;
	m_serverName = serverName;
	m_port = port;
	m_query = query;
}

bool NetHTTP::SetFileOutput(const string &fName)
{
	assert(!m_pFile);

	m_pFile = fopen(fName.c_str(), "wb");
	
	m_fileName = fName; //save for later so we can delete this file if anything goes wrong
	
	if (!m_pFile)
	{
		LogMsg("Unable to write to %s", m_fileName.c_str());
		OnError(ERROR_WRITING_FILE);
		return false;
	}

	return true;
}

bool NetHTTP::AddPostData( const string &name, const byte *pData, int len/*=-1*/ )
{
	if (m_postData.length() != 0)
	{
		//adding to other named post data, so we need a separator
		m_postData += "&";
	}

	//at this stage we need to encode it for safe html transfer, before we get the length

		//at this stage we need to encode it for safe html transfer, before we get the length
		URLEncoder encoder;

		if (!name.empty())
		{
		encoder.encodeData((const byte*)name.c_str(), name.length(), m_postData);
		m_postData += '=';
		if (len == -1) len = strlen((const char*) pData);

		encoder.encodeData(pData, len, m_postData);
		} else
		{
		//it's put data.  No name for it
		m_contentType = "put";
		m_postData = string((char*)pData);
		}


	//m_postData += name+'='+string((char*)pData);


#ifdef _DEBUG

	/*
	LogMsg("Postdata is now %s", m_postData.c_str());


	URLDecoder decoder;

	string encoded;
	encoder.encodeData(pData, len, encoded);
	vector<byte> decoded = decoder.decodeData(encoded);
	int sizeOfDecoded = decoded.size();
	LogMsg("Size of decoded should be %u", decoded.size());
	assert(memcmp(pData, &decoded[0], len) == 0 && "Encryption of string error, they don't match");
	*/

#endif

	return true;
}


string NetHTTP::BuildHTTPHeader()
{

	return "";
}

bool NetHTTP::Start()
{
	m_bytesWrittenToFile = 0;
	m_error = ERROR_NONE;
	m_downloadData.clear();
	m_downloadHeader.clear();
	m_expectedFileBytes = 0;

	
#ifdef _DEBUG
LogMsg("Opening %s on port %d.  Postdata has %d chars", m_serverName.c_str(), m_port, m_postData.length());

//LogMsg("Opening %s on port %d with postdata of %s", m_serverName.c_str(), m_port, m_postData.c_str());
#endif

	string header, stCommand;

	if (m_postData.length() > 0)
	{
		stCommand = "POST";
	} else
	{
		stCommand = "GET";
	}
	
	string finalURL = m_serverName+"/"+m_query;

	m_emscriptenWgetHandle = emscripten_async_wget2_data( finalURL.c_str(), stCommand.c_str(), m_postData.c_str(), this, 1, NetHTTP::onLoaded, NetHTTP::onError, NetHTTP::onProgress);

#ifdef _DEBUG
	LogMsg("Final URL is %s.   Handle is %d.", finalURL.c_str(),  m_emscriptenWgetHandle);
#endif
	m_state = STATE_ACTIVE;
	return true;
}


bool CheckCharVectorForString(vector<char> &v, string marker, int *pIndexOfMarkerEndPosOut)
{
	int correctCount = 0;
	assert(marker.size() > 0);

	for (unsigned int i=0; i < v.size(); i++)
	{
		if (v[i] == marker[correctCount] )
		{
			//so far so good
			correctCount++;
			if (correctCount == marker.size())
			{
				if (pIndexOfMarkerEndPosOut)
				{
					*pIndexOfMarkerEndPosOut = i+1;
				}
				return true; //found it
			}
		} else
		{
			//nah
			correctCount = 0;
		}
	}

	return false;
}

int NetHTTP::ScanDownloadedHeader()
{
	TextScanner t(m_downloadHeader.c_str());
	string temp = t.GetParmString("Content-Length", 1, ":");
	m_expectedFileBytes = atoi(temp.c_str());

	int resultCode = atol(SeparateStringSTL(t.m_lines[0], 1, ' ').c_str());
	switch (resultCode)
	{
		case 404:
			OnError(ERROR_404_FILE_NOT_FOUND);
			break;
		
		case 301: //moved permanently
		case 302: //moved temporarily
		
		string url = t.GetParmString("Location:",1, " ");

		if (!url.empty())
		{
			string domain;
			string request;
			int port = 80;
			BreakDownURLIntoPieces(url, domain, request, port);
			string fNameTemp = m_fileName;
			Reset(false); //end connection, setup new one
			if (!fNameTemp.empty())
			{
				SetFileOutput(fNameTemp);
			}
			Setup(domain, port, request, m_endOfDataSignal);
			Start();
		}
		
	}

	return resultCode;
		
}

void NetHTTP::Update()
{
	
}

void NetHTTP::OnError(eError e)
{
	m_error = e;
	m_state = STATE_ABORT;
	KillConnectionIfNeeded();
}

void NetHTTP::FinishDownload()
{
	m_emscriptenWgetHandle = -1;

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
		m_state = STATE_FINISHED;
		return;
	}
	
	m_state = STATE_FINISHED;
}

const byte * NetHTTP::GetDownloadedData()
{
	if (m_downloadData.empty())
	{
		return NULL; 
	}
	return (const byte*)&m_downloadData[0];
}

int NetHTTP::GetDownloadedBytes()
{
	if (m_pFile || m_bytesWrittenToFile != 0)
	{
		return m_bytesWrittenToFile;
	}
	
	if (m_downloadData.size() == 0) return 0;
	return m_downloadData.size()-1; //the -1 is for the null we added
}

void NetHTTP::SetBuffer(const char *pData, int byteSize)
{
	m_expectedFileBytes = byteSize;
	m_downloadData.resize(byteSize+1); //1 extra so we can add a null
	memcpy((char*)&m_downloadData[0], pData, byteSize); //never do this at home, kids
	m_downloadData[byteSize]=0; //set the NULL too

	if (m_endOfDataSignal == END_OF_DATA_SIGNAL_RTSOFT_MARKER)
	{
		//er.. there has to be a better way then this, but whatever, I don't use rtsoft markers much and when I do it's tiny strings
		
		string temp = (char*)&m_downloadData[0];
		//remove the marker
		StringReplace(C_END_DOWNLOAD_MARKER_STRING, "",temp);
		//move it back
		string crap;
		m_downloadData = vector<char>(temp.begin(), temp.end());
		if (m_downloadData[m_downloadData.size()-1] != 0)
		{
			m_downloadData.push_back(char(0)); //add the null
		}
	}

	if (m_pFile)
	{
		fwrite(GetDownloadedData(), GetDownloadedBytes(), 1, m_pFile);
	}

}

void NetHTTP::onLoaded( unsigned int handle, void* parent, void * file, unsigned int byteSize) 
{
	NetHTTP *pMe = (NetHTTP*)parent;
	pMe->SetBuffer((const char*)file, byteSize);
	pMe->FinishDownload();

#ifdef _DEBUG
	LogMsg("Finished download - got %d bytes.  State is %d", byteSize, pMe->GetState());
#endif

	//http* req = reinterpret_cast<http*>(parent);
	//req->onLoaded(file);
}

void NetHTTP::onError(unsigned int handle, void* parent, int statuserror, const char *message) 
{
//#ifdef _DEBUG
	LogMsg("Got error %d (%s) - keep in mind you have to run this from the same website you're downloading from!", statuserror, message);
//#endif
	eError error = ERROR_CANT_RESOLVE_URL;
	if (statuserror == 404) error = ERROR_404_FILE_NOT_FOUND;
	NetHTTP *pMe = (NetHTTP*)parent;
	pMe->OnError(error);
}

void NetHTTP::SetProgress(int bytesDownloaded, int totalBytes)
{
	m_expectedFileBytes = totalBytes;
	m_bytesWrittenToFile = bytesDownloaded;
}

void NetHTTP::onProgress(unsigned int handle, void* parent, int bytesDownloaded, int totalBytes) 
{
	NetHTTP *pMe = (NetHTTP*)parent;

	pMe->SetProgress(bytesDownloaded, totalBytes);

#ifdef _DEBUG
	//LogMsg("progress %d of %d, state on object is %d. ", bytesDownloaded, totalBytes, (int)pMe->GetState());
#endif
}

