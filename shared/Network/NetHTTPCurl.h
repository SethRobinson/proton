/*
 * NetHTTPCurl - one HTTP or HTTPS request through libcurl's multi interface
 * (Sep 2026), a class an app can use BESIDE NetHTTP.
 *
 * NetHTTP has one backend per build: the socket code (plain HTTP/1.0, the SSE
 * stream mode, SetPostHeaderOverride, SetIdleTimeoutMS, GetResultCode) or,
 * with RT_USE_LIBCURL, NetHTTP_libCURL.cpp (HTTPS, but no custom headers, no
 * timeout, no status code). An app that needs HTTPS for one thing and the
 * socket backend for the rest (RTGameBot: a local LLM streamed over plain
 * HTTP, and Twitch's API for the stream's viewer count) uses this class for
 * that one thing. It is polled from Update() on the main thread like every
 * other engine client: curl_multi_perform never blocks (the name lookup runs
 * on libcurl's own thread when the library has AsynchDNS, which GlobalInit
 * logs once; a build without it stalls the frame that starts a request, and
 * the log line says BLOCKING so the caller knows to prefetch or pin).
 *
 *   NetHTTPCurl net;
 *   net.Setup("https://api.example.com/v1/thing?x=1"); //also HostResolver::Prefetch(host)
 *   net.AddHeader("Authorization: Bearer abc");
 *   net.SetPostData("a=1&b=2", "application/x-www-form-urlencoded"); //a GET without this
 *   net.SetTimeoutMS(10000, 30000);
 *   net.Start();
 *   ...every frame: net.Update();
 *   if (net.GetState() == NetHTTPCurl::STATE_FINISHED) use net.GetResultCode(), net.GetBody()
 *   if (net.GetState() == NetHTTPCurl::STATE_ABORT) see net.GetError(), net.GetErrorText()
 *   net.Reset() to use it again (Setup, headers and body are cleared; the
 *   timeouts, agent, CA bundle and body cap stay)
 *
 * What a project needs: this .cpp compiled (the HTML5 build compiles it to
 * nothing: the browser does HTTPS there), Network/NetSocket.cpp (HostResolver
 * lives in it), the libcurl headers on the include path
 * (shared/win/include/curl on Windows) and libcurl linked: on Windows
 * shared/win/lib/x64/libcurl.dll.a (an import library MSVC links as it is)
 * with libcurl-x64.dll, libssl-1_1-x64.dll, libcrypto-1_1-x64.dll and
 * curl-ca-bundle.crt copied next to the exe (the bundle path is relative to
 * the working directory; SetCABundle for another place). NEVER set
 * CURLOPT_VERBOSE here: it would print every Authorization header to the log.
 * curl_global_init is not thread-safe, so Start (which calls GlobalInit) is
 * for the main thread only; an app calls GlobalShutdown from its Kill, after
 * its last instance is Reset and before the platform's WSACleanup.
 */

#ifndef NetHTTPCurl_h__
#define NetHTTPCurl_h__

#include <string>
#include <vector>

//curl.h's own typedefs, repeated so the header does not need it (a file that
//includes both sees identical typedefs, which C++ allows)
typedef void CURL;
typedef void CURLM;
struct curl_slist;

class NetHTTPCurl
{
public:

	enum eState
	{
		STATE_IDLE,     //Setup and Start to begin
		STATE_ACTIVE,   //in flight: keep calling Update
		STATE_FINISHED, //the reply is in: GetResultCode, GetBody
		STATE_ABORT     //failed: GetError, GetErrorText
	};

	enum eError
	{
		ERROR_NONE,
		ERROR_SETUP,        //no URL, a URL that doesn't parse, libcurl refusing to start
		ERROR_CANT_RESOLVE, //the name lookup failed
		ERROR_CONNECT,      //nothing answered on the port
		ERROR_TIMED_OUT,      //the connect or the whole transfer took longer than SetTimeoutMS allows
		ERROR_SSL,          //the TLS handshake or the certificate check failed (the CA bundle missing, most likely)
		ERROR_TOO_BIG,      //the body passed SetMaxBodyBytes
		ERROR_OTHER         //anything else: GetErrorText has libcurl's words
	};

	NetHTTPCurl();
	~NetHTTPCurl(); //aborts a transfer in flight and frees everything

	void Reset(); //aborts a transfer in flight; clears the URL, the headers, the body and the reply, so Setup can be called again
	void Setup(const std::string &url); //"https://host[:port]/path?query" (http too); prefetches the host's address (HostResolver)
	void AddHeader(const std::string &line); //"Client-Id: abc", no CRLF; any number, cleared by Reset
	void SetPostData(const std::string &body, const std::string &contentType); //sent as is with that Content-Type; without it the request is a GET
	void SetTimeoutMS(int connectMS, int totalMS); //the connect and the whole transfer (defaults 10 s and 30 s)
	void SetUserAgent(const std::string &agent);   //default "protoncurl/1.1"
	void SetCABundle(const std::string &path);     //the certificate bundle (Windows default "curl-ca-bundle.crt", next to the exe)
	void SetMaxBodyBytes(size_t bytes);            //default 4 MB; a bigger reply is cut off with ERROR_TOO_BIG
	bool Start();  //false = ERROR_SETUP (nothing was sent); anything later shows in GetState/GetError
	void Update(); //every frame, main thread

	eState GetState() const { return m_state; }
	eError GetError() const { return m_error; }
	bool IsActive() const { return m_state == STATE_ACTIVE; }
	int GetResultCode() const { return m_resultCode; }              //the HTTP status (200, 401...), 0 until the transfer ended
	const std::string & GetBody() const { return m_body; }          //the reply body (c_str() is null-terminated, as a string is)
	const std::string & GetErrorText() const { return m_errorText; } //libcurl's words for an ERROR; may quote the host, so for a log, never a screen
	const std::string & GetHost() const { return m_host; }
	int GetPort() const { return m_port; }
	bool IsHTTPS() const { return m_bHTTPS; }

	static bool GlobalInit();     //curl_global_init once (Start calls it); logs the library version and its resolver; false when libcurl refuses
	static void GlobalShutdown(); //curl_global_cleanup when GlobalInit ran: from an app's Kill, after every instance is Reset
	static bool HasAsyncDNS();    //after GlobalInit: the library resolves names on its own thread (CURL_VERSION_ASYNCHDNS)
	static const char * ErrorName(eError e); //"timed out", "TLS failed"...: a fixed phrase for a status line
	//"https://host:8443/a/b?c=1" -> host, 8443, "/a/b?c=1", true; the port
	//defaults to 443 for https and 80 for http; false (outputs cleared) for
	//anything without a scheme or a host
	static bool ParseURL(const std::string &url, std::string &hostOut, int &portOut, std::string &pathOut, bool &bHTTPSOut);

private:

	static size_t WriteCallback(char *pData, size_t size, size_t nmemb, void *pThis);
	void CleanupHandles(); //removes and frees the easy and multi handles and the header list; safe when they are already gone
	void Fail(eError e, const std::string &text);

	CURL *m_pEasy;
	CURLM *m_pMulti;
	struct curl_slist *m_pHeaderList;
	std::string m_url, m_host, m_path, m_postData, m_contentType, m_userAgent, m_caBundle, m_body, m_errorText;
	std::vector<std::string> m_headers;
	char m_errorBuf[256]; //CURLOPT_ERRORBUFFER (the .cpp checks it is at least CURL_ERROR_SIZE)
	int m_port;
	bool m_bHTTPS, m_bPost, m_bTooBig;
	int m_connectTimeoutMS, m_timeoutMS;
	size_t m_maxBodyBytes;
	eState m_state;
	eError m_error;
	int m_resultCode;
	int m_running; //curl_multi_perform's handle count
};

#endif // NetHTTPCurl_h__
