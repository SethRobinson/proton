#include "PlatformPrecomp.h"

#include "NetUtils.h"
#include "util/ResourceUtils.h"
#include "util/CRandom.h"
#include "util/MiscUtils.h"

#ifdef WINAPI
//MSVC20015 gets an error if we don't include this manually.  I guess we could just do this for everybody and not just Win?
	#include <iterator>
#endif

//************ for base64 encoding, needed for http auth stuff *********

//both taken from: https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c/41094722#41094722

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

std::string base64_decode(const void* data, const size_t len)
{
	unsigned char* p = (unsigned char*)data;
	int pad = len > 0 && (len % 4 || p[len - 1] == '=');
	const size_t L = ((len + 3) / 4 - pad) * 4;
	std::string str(L / 4 * 3 + pad, '\0');

	for (size_t i = 0, j = 0; i < L; i += 4)
	{
		int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
		str[j++] = n >> 16;
		str[j++] = n >> 8 & 0xFF;
		str[j++] = n & 0xFF;
	}
	if (pad)
	{
		int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
		str[str.size() - 1] = n >> 16;

		if (len > L + 2 && p[L + 2] != '=')
		{
			n |= B64index[p[L + 2]] << 6;
			str.push_back(n >> 8 & 0xFF);
		}
	}
	return str;
}



static const unsigned char base64_table[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
* base64_encode - Base64 encode
* @src: Data to be encoded
* @len: Length of the data to be encoded
* @out_len: Pointer to output length variable, or %NULL if not used
* Returns: Allocated buffer of out_len bytes of encoded data,
* or empty string on failure
*/
std::string base64_encode(const unsigned char *src, size_t len)
{
	unsigned char *out, *pos;
	const unsigned char *end, *in;

	size_t olen;

	olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

	if (olen < len)
		return std::string(); /* integer overflow */

	std::string outStr;
	outStr.resize(olen);
	out = (unsigned char*)&outStr[0];

	end = src + len;
	in = src;
	pos = out;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
	}

	return outStr;
}


/** web.h
* 1.Declares a class to encode strings converting a String 
* into a MIME format called "x-www-form-urlencoded" format. 
*	To convert a String, each character is examined in turn: 
*		1) The ASCII characters 'a' through 'z', 'A' through 'Z', and '0' through '9' remain the same. 
* 	2) The space character ' ' is converted into a plus sign '+'. 
*		3) All other characters are converted into the 3-character string "%xy", where xy is the two-digit hexadecimal representation of the lower 8-bits of the character. 
* 2.Declares a class to decode such strings
* 3. Declares the WebForm class to wrap win32 HTTP calls
* Author: Vijay Mathew Pandyalakal
* Date: 18/10/03

Stolen from CodeProject
**/


void DecToHexString ( uint32 value, byte * pOut, int16 charArrayMaxSize) 
{ 
	static byte  digit; 
	static int  i; 

	for (i = charArrayMaxSize - 1; i >= 0; i--) 
	{ 
		digit = byte((value & 0x0f) + 0x30);
		if (digit > 0x39) digit += 0x07; 
		pOut[i] = digit; 
		value >>= 4; 
	} 
} 


void URLEncoder::encodeData(const byte *pData, int len, string &finalReturn)
{
	char tmp[6];
	tmp[0] = '%';
	tmp[3] = 0;
	assert(len != 0 && len != -1 && "You need to send the length too.");
	string ret = "";

	vector<char> buff;
	buff.reserve(len*2);

	for(int i=0;i<len;i++) 
	{
		if(isOrdinaryChar((char)pData[i])) 
		{
			buff.push_back((char)pData[i]);
		}else if(pData[i] == ' ') 
		{
			buff.push_back('+');
		}else 
		{
			DecToHexString(pData[i], (byte*)&tmp[1], 2); //SETH, this should be faster than sprintf.  start after the % part
			//sprintf(tmp,"%%%02x",pData[i]); //SETH added 02 for leading zeros, serious bug fix.  Sent fix to author
			buff.push_back(tmp[0]);
			buff.push_back(tmp[1]);
			buff.push_back(tmp[2]);
		}
	}

	ret.reserve(ret.size()+buff.size()); //minimize mem allocations
	back_insert_iterator<string> dest (finalReturn);
	copy (buff.begin(), buff.end(), dest);
}


bool URLEncoder::isOrdinaryChar(char c) 
{
	if (
		(c >= 48 && c <= 57) ||
		(c >= 65 && c <= 90) ||
		(c >= 97 && c <= 122)) return true;
	return false;
}

string URLDecoder::decode(string str) 
{
	int len = (int)str.length();
	char* buff = new char[len + 1];
	strcpy(buff,str.c_str());
	string ret = "";
	for(int i=0;i<len;i++) 
	{
		if(buff[i] == '+')
		{
			ret = ret + " ";
		}else if(buff[i] == '%')
		{
			char tmp[4];
			char hex[4];			
			hex[0] = buff[++i];
			hex[1] = buff[++i];
			hex[2] = '\0';		
			//int hex_i = atoi(hex);
			sprintf(tmp,"%c",convertToDec(hex));
			ret = ret + tmp;
		}else {
			ret = ret + buff[i];
		}
	}
	delete[] buff;
	return ret;
}

vector<byte> URLDecoder::decodeData(const string str) 
{
	int len = (int)str.length();
	vector<byte> outBuff;
	outBuff.reserve(len);
	char hex[4];			
	hex[2] = '\0';		

	for(int i=0;i<len;i++) 
	{
		if(str[i] == '+')
		{
			outBuff.push_back(' ');
		}else if(str[i] == '%')
		{
			hex[0] = str[++i];
			hex[1] = str[++i];
			//int hex_i = atoi(hex);
			outBuff.push_back(byte(convertToDec(hex)));
		}  else 
		{
			outBuff.push_back(str[i]);
		}
	}

	return outBuff;
}
int URLDecoder::convertToDec(const char* hex)
{
	char buff[12];
	sprintf(buff,"%s",hex);
	int ret = 0;
	int len = (int)strlen(buff);
	for(int i=0;i<len;i++) 
	{
		char tmp[4];
		tmp[0] = buff[i];
		tmp[1] = '\0';
		getAsDec(tmp);
		int tmp_i = atoi(tmp);
		int rs = 1;
		for(int j=i;j<(len-1);j++)
		{
			rs *= 16;
		}
		ret += (rs * tmp_i);
	}
	return ret;
}

void URLDecoder::getAsDec(char* hex) {
	char tmp = tolower(hex[0]);
	if(tmp == 'a') {
		strcpy(hex,"10");
	}else if(tmp == 'b') {
		strcpy(hex,"11");
	}else if(tmp == 'c') {
		strcpy(hex,"12");
	}else if(tmp == 'd') {
		strcpy(hex,"13");
	}else if(tmp == 'e') {
		strcpy(hex,"14");
	}else if(tmp == 'f') {
		strcpy(hex,"15");
	}else if(tmp == 'g') {
		strcpy(hex,"16");
	}
}



string GetDomainFromURL(string url)
{
	int pos = (int)url.find("/");

	if (pos != string::npos)
	{
		return url.substr(0, pos);
	}

	return url;
}


//converts http://www.rtsoft.com/crap/crap.htm:81  to rtsoft.com, /crap/crap.htm, 81
void BreakDownURLIntoPieces(string url, string &domainOut, string &requestOut, int &port)
{
	port = 80;

	StringReplace("http://", "", url); //don't want that part

	if (url[0] == 'w' && url[1] == 'w' && url[2] == 'w'&& url[3] == '.')
	{
		StringReplace("www.", "", url); //don't want that part
	}

	int pos = (int)url.find(":");
	if (pos != string::npos)
	{
		port = atol( url.substr(pos+1, url.size()- (pos+1)).c_str());
		url.erase(pos, url.size()-pos);
	}

	domainOut = GetDomainFromURL(url);

	requestOut = "";
	if (domainOut.empty()) return;
	if (url.size() == domainOut.size()) return; //something wrong
	requestOut = url.substr(domainOut.size()+1, url.size()- ( domainOut.size()+1 )  );
}



void GetSimpleGUID(uint32 *guid)
{
	int nowyear, nowmonth, nowday, nowhour, nowmin, nowsec;
	GetDateAndTime(&nowmonth, &nowday, &nowyear, &nowhour, &nowmin, &nowsec);

	CRandom r;


	guid[0] =  ((nowmonth+( (nowyear-2014)*12)) *(259200))+ (nowday*86400)+ (nowhour*3600)+nowsec;
	guid[1] = (uint32)Random(RT_RAND_MAX)*(uint32)Random(RT_RAND_MAX)+(uint32)Random(RT_RAND_MAX);

	r.SetRandomSeed(guid[0]+Random(RT_RAND_MAX)+nowyear);

	guid[2] = r.Random(200000000); // +Random(RT_RAND_MAX);
	guid[3] = (uint32)Random(RT_RAND_MAX)*(uint32)Random(RT_RAND_MAX)+(uint32)Random(RT_RAND_MAX);;
};

string GetSimpleGUIDAsString()
{

	uint32 guid[4];
	GetSimpleGUID((uint32*)&guid);

	char temp[32];

	string final;
	//convert to string
	for (int i=0; i < 4; i++)
	{
		memset(temp, 0, 32);
		DecToHexString(guid[i], (byte*)temp, 8);
		final += string(temp);
	}


	return final;
}
