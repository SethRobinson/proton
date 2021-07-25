#include "PlatformPrecomp.h"

/*
If RT_USE_LIBCURL is defined, NetHTTP uses CURL internally instead, can support HTTPS
*/

#include "Network/NetHTTP.h"
#include "Network/NetUtils.h"
#include "BaseApp.h"
#include "util/TextScanner.h"

#define NET_END_MARK_CHECK_DELAY_MS 333
#define C_END_DOWNLOAD_MARKER_STRING "RTENDMARKERBS1001"
#define C_DEFAULT_IDLE_TIMEOUT_MS (25*1000)

NetHTTP::NetHTTP()
{
	m_pFile = NULL;
	Reset(true);
}


NetHTTP::~NetHTTP()
{

	if (m_pFile)
	{
		fclose(m_pFile);
		RemoveFile(m_fileName);
		m_pFile = NULL;
	}

	SAFE_FREE(m_pReceiveBuff);
}

void NetHTTP::Reset(bool bClearPostdata)
{

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
	m_expectedFileBytes = 0;
	m_downloadData.clear();
	m_replyHeader.clear();
	m_query.clear();
	
	if (bClearPostdata)
	{
		m_postData.clear();
	}
	m_bytesWrittenToFile = 0;
}

void NetHTTP::Setup(string serverName, int port, string query, eEndOfDataSignal eodSignal)
{
	m_bHasEncodedPostData = false;
	m_endOfDataSignal = eodSignal;
	m_serverName = serverName;
	m_port = port;
	m_query = query;
}

bool NetHTTP::SetFileOutput(const string &fName)
{
	assert(!m_pFile);


	assert(!"Uh, Seth never tested this with the libcurl stuff");

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
// 
// void NetHTTP::SetHeaderContentType(string contentType)
// {
// 
// }

bool NetHTTP::AddPostData(const string &name, const byte *pData, int len/*=-1*/)
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
		m_bHasEncodedPostData = true;

		encoder.encodeData((const byte*)name.c_str(), name.length(), m_postData);
		m_postData += '=';
		
		if (len == -1) len = strlen((const char*)pData);

		encoder.encodeData(pData, len, m_postData);
	}
	else
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

#ifdef WINAPI
void AddText(const char *tex, const char *filename);
void LogMsgNoCR(const char* traceStr, ...);
void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size);


void SimpleDump(const char* text,
	FILE* stream, unsigned char* ptr, size_t size)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	LogMsgNoCR("%s, %10.10ld bytes (0x%8.8lx): %s\n",
		text, (long)size, (long)size, ptr);

	return;
	for (i = 0; i < size; i += width) {
		LogMsgNoCR("%4.4lx: ", (long)i);

		/* show hex to the left */
		for (c = 0; c < width; c++) {
			if (i + c < size)
				LogMsgNoCR("%02x ", ptr[i + c]);
			else
				LogMsgNoCR("   ");
		}

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++)
		{
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			LogMsgNoCR("%c", c);
		}
		LogMsgNoCR("\n");
	}
}



static int CURLDebugTrace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	const char *text;
	(void)handle; /* prevent compiler warning */
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
		LogMsgNoCR("== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
		// 	case CURLINFO_DATA_OUT:
		// 		text = "=> Send data";
		// 		break;
		// 	case CURLINFO_SSL_DATA_OUT:
		// 		text = "=> Send SSL data";
		// 		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
		// 	case CURLINFO_DATA_IN:
		// 		text = "<= Recv data";
		// 		break;
		// 	case CURLINFO_SSL_DATA_IN:
		// 		text = "<= Recv SSL data";
		// 		break;
	}

	SimpleDump(text, stderr, (unsigned char *)data, size);
	return 0;
}

#endif

size_t NetHTTP::CURLWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *pThisInstance)
{
	size_t realsize = size * nmemb;
	NetHTTP *pCURLInstance = (NetHTTP *)pThisInstance;

	char *ptr = (char *)realloc(pCURLInstance->m_pReceiveBuff, pCURLInstance->m_receivedSize + realsize + 1);
	if (ptr == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	pCURLInstance->m_pReceiveBuff = ptr;
	memcpy(&(pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize]), contents, realsize);
	pCURLInstance->m_receivedSize += realsize;
	pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize] = 0;
	
	pCURLInstance->SetProgress(pCURLInstance->m_receivedSize, 0);
	return realsize;
}

size_t NetHTTP::CURLReadMemoryCallback(void* ptr, size_t size, size_t nmemb, void* pThisInstance)
{

	int maxBytesToRead = size * nmemb;

	if (maxBytesToRead < 1)
	{
		return 0;
	}

	NetHTTP* pCURLInstance = (NetHTTP*)pThisInstance;

  	if (pCURLInstance->m_CURL_bytesSent == pCURLInstance->m_postData.length())
	{
		return 0;
	}

	/* copy as much data as possible into the 'ptr' buffer, but no more than
	   'size' * 'nmemb' bytes! */

	int bytesToRead = rt_min(maxBytesToRead, pCURLInstance->m_postData.size() - pCURLInstance->m_CURL_bytesSent);
	
	memcpy(ptr, (byte*)pCURLInstance->m_postData.c_str()+ pCURLInstance->m_CURL_bytesSent, bytesToRead);
	pCURLInstance->m_CURL_bytesSent += bytesToRead;

	return  (curl_off_t)bytesToRead;
}

void InitCURLIfNeeded()
{
	static bool oneTimeInittedDone = false;

	if (!oneTimeInittedDone)
	{
		LogMsg("Initting libCURL %s", curl_version());
		if (curl_global_init(CURL_GLOBAL_ALL) == 0)
		{
			oneTimeInittedDone = true;
		}
		else
		{
			LogMsg("Error initting libCURL with curl_global_init, will try again soon");
		}
	}
}

static int seek_cb(void* userp, curl_off_t offset, int origin)
{
	NetHTTP *pNetHTTP = (NetHTTP* )userp;
	
	//lseek(our_fd, offset, origin);
	return CURL_SEEKFUNC_OK;
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
#endif

	string header, stCommand;

	if (m_postData.length() > 0)
	{
		stCommand = "POST";
	}
	else
	{
		stCommand = "GET";
	}

	string finalURL = m_serverName + "/" + m_query;

	//libCURL stuff
	if (m_CURL_handles_still_running != 0)
	{
		LogMsg("Warning: CURL activity already in progress");
	}

	InitCURLIfNeeded();
	
	m_pReceiveBuff = (char*)malloc(1);
	m_receivedSize = 0;

	m_CURL_handle = curl_easy_init();

	if (!m_CURL_handle)
	{
		LogMsg("Error with curl_easy_init, got NULL back.");
	}
	m_CURL_multi_handle = curl_multi_init();
	m_CURL_bytesSent = 0;

	//to figure out problems, uncomment below
	//curl_easy_setopt(m_CURL_handle, CURLOPT_VERBOSE, 1L);
#ifdef WINAPI
	curl_easy_setopt(m_CURL_handle, CURLOPT_DEBUGFUNCTION, CURLDebugTrace);
#endif
	curl_easy_setopt(m_CURL_handle, CURLOPT_URL, finalURL.c_str());
	//curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(m_CURL_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_MAXREDIRS, 5L);

	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEFUNCTION, NetHTTP::CURLWriteMemoryCallback);
	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_CURL_handle, CURLOPT_USERAGENT, "gametrans-agent/1.0");
	curl_easy_setopt(m_CURL_handle, CURLOPT_PRIVATE, this);

	//manually set the certs otherwise it can't find it (a windows only issue?)
	curl_easy_setopt(m_CURL_handle, CURLOPT_CAINFO, "curl-ca-bundle.crt");
	
	//needed to handle moved content (http 301 codes, etc)
	curl_easy_setopt(m_CURL_handle, CURLOPT_SEEKFUNCTION, seek_cb);
	curl_easy_setopt(m_CURL_handle, CURLOPT_SEEKDATA, this);

	if (!m_postData.empty())
	{
		curl_easy_setopt(m_CURL_handle, CURLOPT_POST, 1L);
		curl_easy_setopt(m_CURL_handle, CURLOPT_POSTFIELDSIZE, (curl_off_t)m_postData.size());
		curl_easy_setopt(m_CURL_handle, CURLOPT_READFUNCTION, NetHTTP::CURLReadMemoryCallback);
		curl_easy_setopt(m_CURL_handle, CURLOPT_READDATA, this);
		curl_easy_setopt(m_CURL_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)m_postData.size());
	}

	struct curl_slist *chunk = NULL;

	/* Add a custom header */
	chunk = curl_slist_append(chunk, "Accept: */*");
	
	if (m_bHasEncodedPostData)
	{
		chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
	}
	else
	{
		
		//TODO: This probably shouldn't be the default...
		chunk = curl_slist_append(chunk, "Content-Type: application/json");
	}

	/* set our custom set of headers */
	curl_easy_setopt(m_CURL_handle, CURLOPT_HTTPHEADER, chunk);

#ifdef USE_CHUNKED
	{
		struct curl_slist *chunk = NULL;

		chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		/* use curl_slist_free_all() after the *perform() call to free this
		   list again */
	}
#else
	/* Set the expected POST size. If you want to POST large amounts of data,
	   consider CURLOPT_POSTFIELDSIZE_LARGE */
	   //curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
#endif

#ifdef DISABLE_EXPECT
	/*
	  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
	  header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
	  NOTE: if you want chunked transfer too, you need to combine these two
	  since you can only set one list of headers with CURLOPT_HTTPHEADER. */

	  /* A less good option would be to enforce HTTP 1.0, but that might also
		 have other implications. */
	{
		struct curl_slist *chunk = NULL;

		chunk = curl_slist_append(chunk, "Expect:");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		/* use curl_slist_free_all() after the *perform() call to free this
		   list again */
	}
#endif

	CURLMcode result = curl_multi_add_handle(m_CURL_multi_handle, m_CURL_handle);
	if (result != CURLM_OK)
	{
		LogMsg("SmartCURL error: %s", curl_multi_strerror(result));
	}

	result = curl_multi_perform(m_CURL_multi_handle, &m_CURL_handles_still_running);

	if (result != CURLM_OK)
	{
		LogMsg("SmartCURL error: %s", curl_multi_strerror(result));
	}

	if (m_CURL_handles_still_running == 0)
	{
		LogMsg("SmartCURL - Error, seemed like nothing happened.");
	}

	//****************


#ifdef _DEBUG
	LogMsg("Final URL is %s.   Handles active is %d.", finalURL.c_str(), m_CURL_handles_still_running);
#endif
	m_state = STATE_ACTIVE;
	return true;
}

bool CheckCharVectorForString(vector<char> &v, string marker, int *pIndexOfMarkerEndPosOut)
{
	int correctCount = 0;
	assert(marker.size() > 0);

	for (unsigned int i = 0; i < v.size(); i++)
	{
		if (v[i] == marker[correctCount])
		{
			//so far so good
			correctCount++;
			if (correctCount == marker.size())
			{
				if (pIndexOfMarkerEndPosOut)
				{
					*pIndexOfMarkerEndPosOut = i + 1;
				}
				return true; //found it
			}
		}
		else
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

		string url = t.GetParmString("Location:", 1, " ");

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

	if (m_CURL_handles_still_running == 0) return;
	CURLMcode mc = curl_multi_perform(m_CURL_multi_handle, &m_CURL_handles_still_running);

	if (mc != CURLM_OK && mc != CURLM_CALL_MULTI_PERFORM)
	{
		LogMsg("CURL error");
	}

	CURLMsg *msg; /* for picking up messages with the transfer status */
	int msgs_left; /* how many messages are left */

	/* See how the transfers went */
	while ((msg = curl_multi_info_read(m_CURL_multi_handle, &msgs_left)))
	{
		if (msg->msg == CURLMSG_DONE)
		{
			int http_status_code = 0;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
			NetHTTP *pMe;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pMe);
			assert(pMe == this &&"This should be the case");

		
			//LogMsg("CURL done, HTTP status: %d, downloaded %d bytes. ", http_status_code, pMe->m_receivedSize);
			//LogMsg("Data is %s", pMe->m_pReceiveBuff);

			if (http_status_code == 404)
			{
				OnError(ERROR_404_FILE_NOT_FOUND);
			}

			SetBuffer(pMe->m_pReceiveBuff, pMe->m_receivedSize);
			m_downloadData.push_back(0); //useful if used like a string
			m_state = STATE_FINISHED;
			
			curl_multi_remove_handle(m_CURL_multi_handle, msg->easy_handle);
			curl_easy_cleanup(m_CURL_handle);
		}
		else

		{
	   
			double dataSize = 0;
		curl_easy_getinfo(msg->easy_handle, CURLINFO_SIZE_UPLOAD, &dataSize);
		//LogMsg("Data sent: %d", dataSize);

		}



	}

	if (m_CURL_handles_still_running == 0)
	{
		//finish up
		curl_multi_cleanup(m_CURL_multi_handle);
		//curl_global_cleanup();
		//LogMsg("CURL cleaned up");
	}
}

void NetHTTP::OnError(eError e)
{
	m_error = e;
	m_state = STATE_ABORT;
}

void NetHTTP::FinishDownload()
{
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
	return m_downloadData.size() - 1; //the -1 is for the null we added
}

void NetHTTP::SetBuffer(const char *pData, int byteSize)
{
	m_expectedFileBytes = byteSize;
	m_downloadData.resize(byteSize + 1); //1 extra so we can add a null
	memcpy((char*)&m_downloadData[0], pData, byteSize); //never do this at home, kids
	m_downloadData[byteSize] = 0; //set the NULL too

	if (m_endOfDataSignal == END_OF_DATA_SIGNAL_RTSOFT_MARKER)
	{
		//er.. there has to be a better way then this, but whatever, I don't use rtsoft markers much and when I do it's tiny strings

		string temp = (char*)&m_downloadData[0];
		//remove the marker
		StringReplace(C_END_DOWNLOAD_MARKER_STRING, "", temp);
		//move it back
		string crap;
		m_downloadData = vector<char>(temp.begin(), temp.end());
		if (m_downloadData[m_downloadData.size() - 1] != 0)
		{
			m_downloadData.push_back(char(0)); //add the null
		}
	}

	if (m_pFile)
	{
		fwrite(GetDownloadedData(), GetDownloadedBytes(), 1, m_pFile);
	}
}

void NetHTTP::SetProgress(int bytesDownloaded, int totalBytes)
{
	m_expectedFileBytes = totalBytes;
	m_bytesWrittenToFile = bytesDownloaded;
}

