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
	m_receiveBuffCapacity = 0;
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
	m_receiveBuffCapacity = 0;
}

void NetHTTP::Reset(bool bClearPostdata)
{
	SAFE_FREE(m_pReceiveBuff);
	m_receiveBuffCapacity = 0;

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
	m_bHasEncodedPostData = false;

	if (bClearPostdata)
	{
		m_postData.clear();
	}
	m_bytesWrittenToFile = 0;
}

void NetHTTP::Setup(string serverName, int port, string query, eEndOfDataSignal eodSignal)
{
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
// 
// void NetHTTP::SetHeaderContentType(string contentType)
// {
// 
// }

bool NetHTTP::AddPostData(const string &name, const uint8 *pData, int len/*=-1*/)
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

		encoder.encodeData((const uint8*)name.c_str(), name.length(), m_postData);
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

void AddText(const char *tex, const char *filename);
void LogMsgNoCR(const char* traceStr, ...);


void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size);

void SimpleDump(const char* text,
	FILE* stream, unsigned char* ptr, size_t size)
{
	unsigned int width = 0x10;

	//this will crash when downloading a file, so maybe bad right now.  (it's a debug mode only thing so no biggie)

	//LogMsg("%s, %10.10ld bytes (0x%8.8lx): %s\n",
	//	text, (long)size, (long)size, ptr);

	return;
	
	/*
	size_t i;
	size_t c;

	for (i = 0; i < size; i += width) {
		LogMsgNoCR("%4.4lx: ", (long)i);

		
		for (c = 0; c < width; c++) {
			if (i + c < size)
				LogMsgNoCR("%02x ", ptr[i + c]);
			else
				LogMsgNoCR("   ");
		}

		
		for (c = 0; (c < width) && (i + c < size); c++)
		{
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			LogMsgNoCR("%c", c);
		}
		LogMsgNoCR("\n");
	}
	*/
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
		LogMsg("== Info: %s", data);
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
	//LogMsg("more info but can't display it... fix it, Seth");
	return 0;
}

size_t NetHTTP::CURLWriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* pThisInstance)
{
	size_t realsize = size * nmemb;
	NetHTTP* pCURLInstance = (NetHTTP*)pThisInstance;


	//if we're downloading a file, we'll directly write to m_pFile and leave early
	if (pCURLInstance->m_pFile)
	{
		fwrite(contents, realsize, 1, pCURLInstance->m_pFile);
		pCURLInstance->m_bytesWrittenToFile += realsize;
		pCURLInstance->m_receivedSize += realsize;

		return realsize;
	}

	// Calculate how much space we need
	size_t newSize = pCURLInstance->m_receivedSize + realsize + 1;

	// Check if we need to resize the buffer
	if (newSize > pCURLInstance->m_receiveBuffCapacity) {
		// Double the capacity, or more if needed
		size_t newCapacity = pCURLInstance->m_receiveBuffCapacity;
		if (newCapacity == 0) {
			// First allocation - start with a reasonable size
			newCapacity = 262144; // 256KB initial allocation
		}

		// Keep doubling until we have enough space, but cap at 10MB
		while (newCapacity < newSize && newCapacity < 10 * 1024 * 1024) {
			newCapacity *= 2;
		}

		// Ensure we don't exceed 10MB
		if (newCapacity > 10 * 1024 * 1024) {
			newCapacity = 10 * 1024 * 1024;
		}

		// Make sure we have at least enough space for this chunk
		if (newCapacity < newSize) {
			// If the current chunk would exceed our max buffer, just allocate exactly what we need
			newCapacity = newSize;
		}

		// Reallocate with the new capacity
		char* ptr = (char*)realloc(pCURLInstance->m_pReceiveBuff, newCapacity);
		if (ptr == NULL) {
			/* out of memory! */
			printf("not enough memory (realloc returned NULL)\n");
			return 0;
		}

		pCURLInstance->m_pReceiveBuff = ptr;
		pCURLInstance->m_receiveBuffCapacity = newCapacity;
	}

	// Copy the new data into the buffer
	memcpy(&(pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize]), contents, realsize);
	pCURLInstance->m_receivedSize += realsize;
	pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize] = 0;

	// Update progress less frequently for large downloads
		pCURLInstance->SetProgress(pCURLInstance->m_receivedSize, -1);
	
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
	
	memcpy(ptr, (uint8*)pCURLInstance->m_postData.c_str()+ pCURLInstance->m_CURL_bytesSent, bytesToRead);
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

size_t write_data_to_nothing(void* ptr, size_t size, size_t count, void* stream)
{
	//just pretend like we care
	return size * count;
}

static size_t header_callback(char* buffer, size_t size,
	size_t nitems, void* userdata)
{
	/* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
	/* 'userdata' is set with CURLOPT_HEADERDATA */
	NetHTTP* pThis = (NetHTTP*)userdata;

	//let's do some questionable string handling to find the content-length tag if it exists
	string temp;
	temp.resize(nitems + 1);
	memcpy(&temp[0], buffer, nitems);
	temp[nitems] = 0; //terminate it so we can do normal string handling
	
	//LogMsg("Got %s", temp.c_str());
	
	auto valArray = StringTokenize(temp, ":");
	if (valArray.size() == 2)
	{
		//is this it?
		if (valArray[0] == "Content-Length")
		{
			pThis->SetExpectedBytes(atoi(valArray[1].c_str()));
		}
	}

	return nitems * size;
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


	if (m_pFile)
	{
		//let's grab the filesize first in this horrible blocking method

		//nevermind, this doesn't work and seems to download the whole thing?!

		/*
		CURL* curl = curl_easy_init();
		if (curl)
		{
#ifdef _DEBUG
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CURLDebugTrace);
#endif
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); //don't actually load it

			curl_easy_setopt(curl, CURLOPT_URL, finalURL.c_str());
			//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_nothing);
			
			if (GetPlatformID() == PLATFORM_ID_ANDROID)
			{
				//Android needs the physical file copied for this to work, so include it in your assets and do something like this on app startup:
				//Don't ask me why, but it fails if we don't set the path both ways like this
				curl_easy_setopt(curl, CURLOPT_CAPATH, GetSavePath().c_str());
				curl_easy_setopt(curl, CURLOPT_CAINFO, (GetSavePath() + "curl-ca-bundle.crt").c_str());
			}
			else
			{
				//manually set the cert on windows otherwise it can't find it.  I've only used this curl stuff on android and windows so no idea on other platforms.
				curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
			}

			
			CURLcode res = curl_easy_perform(curl);

			if (!res) {
				
				curl_off_t cl;
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
				if (!res) {
					LogMsg("Download size: %d", cl);
					m_expectedFileBytes = cl;
				}
				else
				{
					LogMsg("Failed to understand download size");
					m_expectedFileBytes = 0;
				}
			}
			curl_easy_cleanup(curl)
		}
		;
		*/
	}

	m_CURL_handle = curl_easy_init();

	if (!m_CURL_handle)
	{
		LogMsg("Error with curl_easy_init, got NULL back.");
	}
	m_CURL_multi_handle = curl_multi_init();
	m_CURL_bytesSent = 0;

	//to figure out problems, uncomment below
#ifdef _DEBUG
	//LogMsg("CURL in debug mode");
	//curl_easy_setopt(m_CURL_handle, CURLOPT_VERBOSE, 1L);
	//curl_easy_setopt(m_CURL_handle, CURLOPT_DEBUGFUNCTION, CURLDebugTrace);
#endif
	curl_easy_setopt(m_CURL_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

	curl_easy_setopt(m_CURL_handle, CURLOPT_URL, finalURL.c_str());
	//curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(m_CURL_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_HEADERFUNCTION,	header_callback);
	curl_easy_setopt(m_CURL_handle, CURLOPT_HEADERDATA, this);


	curl_easy_setopt(m_CURL_handle, CURLOPT_USERAGENT, "protoncurl-agent/1.0");
	curl_easy_setopt(m_CURL_handle, CURLOPT_PRIVATE, this);
	//needed to handle moved content (http 301 codes, etc)
	curl_easy_setopt(m_CURL_handle, CURLOPT_SEEKFUNCTION, seek_cb);
	curl_easy_setopt(m_CURL_handle, CURLOPT_SEEKDATA, this);

	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEFUNCTION, NetHTTP::CURLWriteMemoryCallback);
	curl_easy_setopt(m_CURL_handle, CURLOPT_BUFFERSIZE, 1024 * 1024L); // 1MB chunks
	curl_easy_setopt(m_CURL_handle, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)0);
	curl_easy_setopt(m_CURL_handle, CURLOPT_MAX_SEND_SPEED_LARGE, (curl_off_t)0);


	// Enable TCP keepalive
	curl_easy_setopt(m_CURL_handle, CURLOPT_TCP_KEEPALIVE, 1L);


	if (GetPlatformID() == PLATFORM_ID_ANDROID)
	{
		//Android needs the physical file copied for this to work, so include it in your assets and do something like this on app startup:
		//Don't ask me why, but it fails if we don't set the path both ways like this
		curl_easy_setopt(m_CURL_handle, CURLOPT_CAPATH, GetSavePath().c_str());
		curl_easy_setopt(m_CURL_handle, CURLOPT_CAINFO, (GetSavePath()+"curl-ca-bundle.crt").c_str());
	} else if (GetPlatformID() ==  PLATFORM_ID_IOS)
    {
        curl_easy_setopt(m_CURL_handle, CURLOPT_CAPATH, GetBaseAppPath().c_str());
        curl_easy_setopt(m_CURL_handle, CURLOPT_CAINFO, (GetBaseAppPath()+"curl-ca-bundle.crt").c_str());
        
        
      } else
	{
		//manually set the cert on windows otherwise it can't find it.  I've only used this curl stuff on android and windows so no idea on other platforms.
		curl_easy_setopt(m_CURL_handle, CURLOPT_CAINFO, "curl-ca-bundle.crt");
	}


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
		//chunk = curl_slist_append(chunk, "Content-Type: application/json");
		chunk = curl_slist_append(chunk, "Content-Type: application/html");
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
			 
		
#ifdef _DEBUG
			LogMsg("CURL done, HTTP status: %d, downloaded %d bytes. ", http_status_code, pMe->m_receivedSize);
			//LogMsg("Data is %s", pMe->m_pReceiveBuff);
#endif

			if (http_status_code == 404)
			{
				OnError(ERROR_404_FILE_NOT_FOUND);
			}

			SetBuffer(pMe->m_pReceiveBuff, pMe->m_receivedSize);
			m_downloadData.push_back(0); //useful if used like a string
			FinishDownload();
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
		return;
	}

}

const uint8 * NetHTTP::GetDownloadedData()
{
	if (m_downloadData.empty())
	{
		return NULL;
	}
	return (const uint8*)&m_downloadData[0];
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
	
	if (m_pFile)
	{
		
	}
	else
	{
		m_downloadData.resize(byteSize + 1); //1 extra so we can add a null
		memcpy((char*)&m_downloadData[0], pData, byteSize); //never do this at home, kids
		m_downloadData[byteSize] = 0; //set the NULL too
	}

	if (m_endOfDataSignal == END_OF_DATA_SIGNAL_RTSOFT_MARKER && m_pFile == NULL)
	{
		//er.. there has to be a better way then this, but whatever, I don't use rtsoft markers much and when I do it's tiny strings

		string temp = (char*)&m_downloadData[0];
		//remove the marker
		StringReplace(C_END_DOWNLOAD_MARKER_STRING, "", temp);
		//move it back
		string crap;
		m_downloadData = vector<uint8>(temp.begin(), temp.end());
		if (!m_downloadData.empty()) //make sure we put a null on the end
		{
			if (m_downloadData[m_downloadData.size() - 1] != 0)
			{
				m_downloadData.push_back(char(0)); //add the null
			}
		}
	}

}

void NetHTTP::SetProgress(int bytesDownloaded, int totalBytes)
{
	if (totalBytes != -1)
		m_expectedFileBytes = totalBytes;
	m_bytesWrittenToFile = bytesDownloaded;
}

