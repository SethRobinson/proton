#ifndef NetUtils_h__
#define NetUtils_h__

class URLEncoder 
{
public:
	void encodeData(const byte *pData, int len, string &finalReturn);

private:
	static bool isOrdinaryChar(char c);
};

class URLDecoder 
{
public:
	static string decode(string str);
	vector<byte> decodeData(const string str);

private:
	static int convertToDec(const char* hex);
	static void getAsDec(char* hex);
};
void BreakDownURLIntoPieces(string url, string &domainOut, string &requestOut, int &port);
string GetDomainFromURL(string url);
std::string base64_decode(const void* data, const size_t len);
std::string base64_encode(const unsigned char *src, size_t len);
void DecToHexString ( uint32 value, byte * pOut, int16 charArrayMaxSize);
void GetSimpleGUID(uint32 *guid); //makes GUID by using date and whatever current rand() seed is, along with some weird stuff
string GetSimpleGUIDAsString(); //makes GUID by using date and whatever current rand() seed is, along with some weird stuff
	
#endif // NetUtils_h__