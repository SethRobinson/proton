#include "PlatformPrecomp.h"
#include "Network/NetHTTPCurl.h"

#ifndef PLATFORM_HTML5

#include <curl/curl.h>
#include "Network/HostResolver.h"
#include "BaseApp.h"

static_assert(256 >= CURL_ERROR_SIZE, "m_errorBuf must hold CURL_ERROR_SIZE bytes");

static bool s_bNetHTTPCurlInitted = false;
static bool s_bNetHTTPCurlAsyncDNS = false;

bool NetHTTPCurl::GlobalInit()
{
	if (s_bNetHTTPCurlInitted)
		return true;
	if (curl_global_init(CURL_GLOBAL_ALL) != 0)
	{
		LogMsg("NetHTTPCurl: curl_global_init failed");
		return false;
	}
	s_bNetHTTPCurlInitted = true;
	curl_version_info_data *pInfo = curl_version_info(CURLVERSION_NOW);
	s_bNetHTTPCurlAsyncDNS = pInfo && (pInfo->features & CURL_VERSION_ASYNCHDNS) != 0;
	LogMsg("NetHTTPCurl: %s, %s name resolver", curl_version(),
		s_bNetHTTPCurlAsyncDNS ? "threaded (AsynchDNS)" : "BLOCKING (no AsynchDNS: a lookup stalls the frame that starts a request)");
	return true;
}

void NetHTTPCurl::GlobalShutdown()
{
	if (!s_bNetHTTPCurlInitted)
		return;
	curl_global_cleanup();
	s_bNetHTTPCurlInitted = false;
}

bool NetHTTPCurl::HasAsyncDNS()
{
	return s_bNetHTTPCurlAsyncDNS;
}

const char * NetHTTPCurl::ErrorName(eError e)
{
	switch (e)
	{
	case ERROR_NONE: return "no error";
	case ERROR_SETUP: return "not started";
	case ERROR_CANT_RESOLVE: return "name lookup failed";
	case ERROR_CONNECT: return "connection failed";
	case ERROR_TIMED_OUT: return "timed out";
	case ERROR_SSL: return "TLS failed";
	case ERROR_TOO_BIG: return "reply too big";
	default: return "failed";
	}
}

bool NetHTTPCurl::ParseURL(const std::string &url, std::string &hostOut, int &portOut, std::string &pathOut, bool &bHTTPSOut)
{
	hostOut.clear();
	pathOut.clear();
	portOut = 0;
	bHTTPSOut = false;
	std::string rest;
	if (url.compare(0, 8, "https://") == 0)
	{
		bHTTPSOut = true;
		rest = url.substr(8);
	}
	else if (url.compare(0, 7, "http://") == 0)
		rest = url.substr(7);
	else
		return false;

	size_t cut = rest.find_first_of("/?");
	std::string hostPort = cut == std::string::npos ? rest : rest.substr(0, cut);
	pathOut = cut == std::string::npos ? std::string("/") : rest.substr(cut);
	if (pathOut[0] == '?')
		pathOut = "/" + pathOut;

	size_t colon = hostPort.rfind(':');
	if (colon != std::string::npos && hostPort.find(']') == std::string::npos) //not an IPv6 literal
	{
		portOut = atoi(hostPort.c_str() + colon + 1);
		hostOut = hostPort.substr(0, colon);
		if (portOut <= 0 || portOut > 65535)
		{
			hostOut.clear();
			pathOut.clear();
			portOut = 0;
			return false;
		}
	}
	else
		hostOut = hostPort;

	if (hostOut.empty())
	{
		pathOut.clear();
		portOut = 0;
		return false;
	}
	if (portOut == 0)
		portOut = bHTTPSOut ? 443 : 80;
	return true;
}

NetHTTPCurl::NetHTTPCurl()
{
	m_pEasy = NULL;
	m_pMulti = NULL;
	m_pHeaderList = NULL;
	m_userAgent = "protoncurl/1.1";
#ifdef WINAPI
	m_caBundle = "curl-ca-bundle.crt";
#endif
	m_connectTimeoutMS = 10000;
	m_timeoutMS = 30000;
	m_maxBodyBytes = 4 * 1024 * 1024;
	m_errorBuf[0] = 0;
	m_port = 0;
	m_bHTTPS = m_bPost = m_bTooBig = false;
	m_state = STATE_IDLE;
	m_error = ERROR_NONE;
	m_resultCode = 0;
	m_running = 0;
}

NetHTTPCurl::~NetHTTPCurl()
{
	CleanupHandles();
}

void NetHTTPCurl::CleanupHandles()
{
	if (m_pMulti && m_pEasy)
		curl_multi_remove_handle((CURLM *)m_pMulti, (CURL *)m_pEasy);
	if (m_pEasy)
	{
		curl_easy_cleanup((CURL *)m_pEasy);
		m_pEasy = NULL;
	}
	if (m_pMulti)
	{
		curl_multi_cleanup((CURLM *)m_pMulti);
		m_pMulti = NULL;
	}
	if (m_pHeaderList)
	{
		curl_slist_free_all(m_pHeaderList);
		m_pHeaderList = NULL;
	}
	m_running = 0;
}

void NetHTTPCurl::Reset()
{
	CleanupHandles();
	m_url.clear();
	m_host.clear();
	m_path.clear();
	m_postData.clear();
	m_contentType.clear();
	m_headers.clear();
	m_body.clear();
	m_errorText.clear();
	m_errorBuf[0] = 0;
	m_port = 0;
	m_bHTTPS = m_bPost = m_bTooBig = false;
	m_state = STATE_IDLE;
	m_error = ERROR_NONE;
	m_resultCode = 0;
}

void NetHTTPCurl::Setup(const std::string &url)
{
	m_url = url;
	if (ParseURL(url, m_host, m_port, m_path, m_bHTTPS))
		HostResolver::Prefetch(m_host); //nothing when it is cached or in flight
}

void NetHTTPCurl::AddHeader(const std::string &line)
{
	m_headers.push_back(line);
}

void NetHTTPCurl::SetPostData(const std::string &body, const std::string &contentType)
{
	m_postData = body;
	m_contentType = contentType;
	m_bPost = true;
}

void NetHTTPCurl::SetTimeoutMS(int connectMS, int totalMS)
{
	m_connectTimeoutMS = connectMS;
	m_timeoutMS = totalMS;
}

void NetHTTPCurl::SetUserAgent(const std::string &agent)
{
	m_userAgent = agent;
}

void NetHTTPCurl::SetCABundle(const std::string &path)
{
	m_caBundle = path;
}

void NetHTTPCurl::SetMaxBodyBytes(size_t bytes)
{
	m_maxBodyBytes = bytes;
}

void NetHTTPCurl::Fail(eError e, const std::string &text)
{
	m_error = e;
	m_errorText = text;
	m_state = STATE_ABORT;
}

size_t NetHTTPCurl::WriteCallback(char *pData, size_t size, size_t nmemb, void *pThis)
{
	NetHTTPCurl *pMe = (NetHTTPCurl *)pThis;
	size_t n = size * nmemb;
	if (pMe->m_body.size() + n > pMe->m_maxBodyBytes)
	{
		pMe->m_bTooBig = true;
		return 0; //libcurl ends the transfer with CURLE_WRITE_ERROR
	}
	pMe->m_body.append(pData, n);
	return n;
}

static NetHTTPCurl::eError MapCurlCode(CURLcode rc, bool bTooBig)
{
	switch (rc)
	{
	case CURLE_COULDNT_RESOLVE_HOST:
	case CURLE_COULDNT_RESOLVE_PROXY:
		return NetHTTPCurl::ERROR_CANT_RESOLVE;
	case CURLE_COULDNT_CONNECT:
		return NetHTTPCurl::ERROR_CONNECT;
	case CURLE_OPERATION_TIMEDOUT:
		return NetHTTPCurl::ERROR_TIMED_OUT;
	case CURLE_SSL_CONNECT_ERROR:
	case CURLE_PEER_FAILED_VERIFICATION:
	case CURLE_SSL_CACERT_BADFILE:
	case CURLE_SSL_CIPHER:
	case CURLE_SSL_ENGINE_NOTFOUND:
	case CURLE_SSL_ENGINE_SETFAILED:
	case CURLE_SSL_CERTPROBLEM:
	case CURLE_SSL_ISSUER_ERROR:
		return NetHTTPCurl::ERROR_SSL;
	case CURLE_WRITE_ERROR:
		return bTooBig ? NetHTTPCurl::ERROR_TOO_BIG : NetHTTPCurl::ERROR_OTHER;
	default:
		return NetHTTPCurl::ERROR_OTHER;
	}
}

bool NetHTTPCurl::Start()
{
	CleanupHandles();
	m_body.clear();
	m_errorText.clear();
	m_errorBuf[0] = 0;
	m_resultCode = 0;
	m_error = ERROR_NONE;
	m_bTooBig = false;
	m_state = STATE_IDLE;

	if (m_url.empty() || m_host.empty())
	{
		Fail(ERROR_SETUP, "no URL that parses (Setup first)");
		return false;
	}
	if (!GlobalInit())
	{
		Fail(ERROR_SETUP, "libcurl could not be initialized");
		return false;
	}

	CURL *pEasy = curl_easy_init();
	CURLM *pMulti = pEasy ? curl_multi_init() : NULL;
	if (!pEasy || !pMulti)
	{
		if (pEasy) curl_easy_cleanup(pEasy);
		Fail(ERROR_SETUP, "curl_easy_init or curl_multi_init returned NULL");
		return false;
	}
	m_pEasy = pEasy;
	m_pMulti = pMulti;

	curl_easy_setopt(pEasy, CURLOPT_URL, m_url.c_str());
	curl_easy_setopt(pEasy, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pEasy, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(pEasy, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pEasy, CURLOPT_CONNECTTIMEOUT_MS, (long)m_connectTimeoutMS);
	curl_easy_setopt(pEasy, CURLOPT_TIMEOUT_MS, (long)m_timeoutMS);
	curl_easy_setopt(pEasy, CURLOPT_USERAGENT, m_userAgent.c_str());
	curl_easy_setopt(pEasy, CURLOPT_ERRORBUFFER, m_errorBuf);
	curl_easy_setopt(pEasy, CURLOPT_WRITEFUNCTION, NetHTTPCurl::WriteCallback);
	curl_easy_setopt(pEasy, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(pEasy, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(pEasy, CURLOPT_PRIVATE, this);
	//never CURLOPT_VERBOSE: it prints every request header, Authorization included

	//the certificate bundle, as NetHTTP_libCURL.cpp sets it per platform
	//(Linux and the Mac find the system store on their own)
	if (GetPlatformID() == PLATFORM_ID_ANDROID)
	{
		curl_easy_setopt(pEasy, CURLOPT_CAPATH, GetSavePath().c_str());
		curl_easy_setopt(pEasy, CURLOPT_CAINFO, (GetSavePath() + "curl-ca-bundle.crt").c_str());
	}
	else if (GetPlatformID() == PLATFORM_ID_IOS)
	{
		curl_easy_setopt(pEasy, CURLOPT_CAPATH, GetBaseAppPath().c_str());
		curl_easy_setopt(pEasy, CURLOPT_CAINFO, (GetBaseAppPath() + "curl-ca-bundle.crt").c_str());
	}
	else if (!m_caBundle.empty())
	{
		static bool s_bBundleChecked = false;
		if (!s_bBundleChecked)
		{
			//one look, so a missing bundle is named in the log before the
			//first TLS failure is
			s_bBundleChecked = true;
			FILE *fp = fopen(m_caBundle.c_str(), "rb");
			if (fp)
				fclose(fp);
			else
				LogMsg("NetHTTPCurl: %s not found next to the exe: every HTTPS request will fail its certificate check", m_caBundle.c_str());
		}
		curl_easy_setopt(pEasy, CURLOPT_CAINFO, m_caBundle.c_str());
	}

	if (m_bPost)
	{
		curl_easy_setopt(pEasy, CURLOPT_POST, 1L);
		curl_easy_setopt(pEasy, CURLOPT_POSTFIELDS, m_postData.c_str()); //the string outlives the transfer (a member)
		curl_easy_setopt(pEasy, CURLOPT_POSTFIELDSIZE, (long)m_postData.size());
	}

	for (size_t i = 0; i < m_headers.size(); i++)
		m_pHeaderList = curl_slist_append(m_pHeaderList, m_headers[i].c_str());
	if (m_bPost && !m_contentType.empty())
		m_pHeaderList = curl_slist_append(m_pHeaderList, ("Content-Type: " + m_contentType).c_str());
	m_pHeaderList = curl_slist_append(m_pHeaderList, "Expect:"); //no 100-continue wait on a POST
	curl_easy_setopt(pEasy, CURLOPT_HTTPHEADER, m_pHeaderList);

	CURLMcode mc = curl_multi_add_handle(pMulti, pEasy);
	if (mc != CURLM_OK)
	{
		Fail(ERROR_SETUP, curl_multi_strerror(mc));
		CleanupHandles();
		return false;
	}
	m_state = STATE_ACTIVE;
	Update(); //the first step: the lookup or the connect begins now
	return true;
}

void NetHTTPCurl::Update()
{
	if (m_state != STATE_ACTIVE || !m_pMulti)
		return;

	CURLMcode mc = curl_multi_perform((CURLM *)m_pMulti, &m_running);
	if (mc != CURLM_OK && mc != CURLM_CALL_MULTI_PERFORM)
	{
		Fail(ERROR_OTHER, curl_multi_strerror(mc));
		CleanupHandles();
		return;
	}

	int msgsLeft = 0;
	CURLMsg *pMsg;
	while ((pMsg = curl_multi_info_read((CURLM *)m_pMulti, &msgsLeft)) != NULL)
	{
		if (pMsg->msg != CURLMSG_DONE)
			continue;
		long code = 0;
		curl_easy_getinfo((CURL *)m_pEasy, CURLINFO_RESPONSE_CODE, &code);
		m_resultCode = (int)code;
		CURLcode rc = pMsg->data.result;
		if (rc == CURLE_OK)
			m_state = STATE_FINISHED;
		else
			Fail(MapCurlCode(rc, m_bTooBig), m_errorBuf[0] ? std::string(m_errorBuf) : std::string(curl_easy_strerror(rc)));
		CleanupHandles(); //in both paths: the handles are done with (the message pointed into the multi handle, so this comes last)
		return;
	}
}

#endif //PLATFORM_HTML5
