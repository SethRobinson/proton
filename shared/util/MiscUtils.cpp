#include "PlatformPrecomp.h"
#include "MiscUtils.h"
#include "ResourceUtils.h"

//taken from Gamedeveloper magazine's InnerProduct (Sean Barrett 2005-03-15)

// circular shift hash -- produces good results if modding by a prime;
// longword at a time would be faster (need alpha-style "is any byte 0"),
// or just use the first longword

uint32 HashString(const char *str, int32 len)
{
	if (!str) return 0;

	unsigned char *n = (unsigned char *) str;
	uint32 acc = 0x55555555;

	if (len == 0)
	{
		while (*n)
			acc = (acc >> 27) + (acc << 5) + *n++;
	} else
	{
		for (int32 i=0; i < len; i++)
		{
			acc = (acc >> 27) + (acc << 5) + *n++;
		}
	}
	return acc;
}

unsigned int GetHashOfFile(string fName, bool bAddBasePath)
{
	unsigned int hash;
	int size;
	uint8 *pData;

#ifdef _CONSOLE
	unsigned int lsize;
	//we use basic so it won't automatically decompress an .rttex for us
	pData = LoadFileIntoMemoryBasic(fName, &lsize, false, bAddBasePath);
	size = lsize;
#else 
	pData = GetFileManager()->Get(fName, &size, false, false);
#endif

		if (!pData) return 0; //no file, no hash
	hash = HashString((char*)pData, size);
	SAFE_DELETE_ARRAY(pData);
	return hash;
}

bool IsEven(int number)
{
	return (number & 1) == 0;
}


string BoolToEnabled(bool bEnabled)
{
	if (bEnabled) return "Enabled";
	return "Disabled";
}

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

string RemoveLastPartOfDir(string path)
{

	if (path.find_last_of("\\/") == string::npos)
	{
		//well, there is no path in this.. so the path is.. uh, blank I guess.
		return "";
	}
	if (path[path.size()-1] == '/' || path[path.size()-1] == '\\')
	{
		path = path.substr(0, path.find_last_of("\\/"));
	}
	path = path.substr(0, path.find_last_of("\\/")) + PATH_SEPARATOR;
	return path;
}

void ToLowerCase(char *pCharArray)
{
	for (int i=0; pCharArray[i]; i++)
	{
		pCharArray[i] = tolower(pCharArray[i]);
	}
}

void ToUpperCase(char *pCharArray)
{
	for (int i=0; pCharArray[i]; i++)
	{
		pCharArray[i] = toupper(pCharArray[i]);
	}
}

string ToLowerCaseString (const string & s)
{
	string d (s);
	for (unsigned int i=0; i < d.length(); i++)
	{
		d[i] = tolower(d[i]);
	}
	return d;
}  // end of tolower

string ToUpperCaseString (const string & s)
{
	string d (s);
	for (unsigned int i=0; i < d.length(); i++)
	{
		d[i] = toupper(d[i]);
	}
	return d;
}  // end of tolower

void SetIntWithTarget(int32 *p_out_dest, int r_target, int r_amount)
{
	if (*p_out_dest > r_target)
	{
		//make it less
		*p_out_dest -= r_amount;
		if (*p_out_dest < r_target) *p_out_dest = r_target;

	}   else
	{
		//make it more
		*p_out_dest += r_amount;
		if (*p_out_dest > r_target) *p_out_dest = r_target;
	}

}


void SetFloatWithTarget(float *p_out_dest, float r_target, float r_amount)
{
	if (*p_out_dest > r_target)
	{
		//make it less
		*p_out_dest -= r_amount;
		if (*p_out_dest < r_target) *p_out_dest = r_target;

	}   else
	{
		//make it more
		*p_out_dest += r_amount;
		if (*p_out_dest > r_target) *p_out_dest = r_target;
	}

}

int mod(int a, int b)
{

	assert(b != 0 && "You can't use 0 in this."); 
	int const rem = a % b;
	if ( (-1 % 2 == 1) || rem >= 0)
		return rem; else
		return rem + abs(b);
}

int Random(int range)
{
	if(range == 0)
	{
		return 0;
	}
	return rand()%range;
}

int RandomRange(int rangeMin, int rangeMax)
{
	if (rangeMin == rangeMax) return rangeMin; //avoid divide by zero error
	return rand()% (rangeMax-rangeMin)+rangeMin;
}

float RandomRangeFloat(float rangeMin, float rangeMax)
{
	float random = ((float) rand()) / (float) RAND_MAX;
	float diff = rangeMax - rangeMin;
	float r = random * diff;
	return rangeMin + r;
}
	
//randomize a number slightly, by allowing up to modAmount to be removed or added
int GiveOrTake(int baseNum, int modAmount)
{
	if (modAmount == 0) return baseNum; //avoid divide by zero
	return baseNum +  (Random(modAmount*2) - modAmount);
}

float LerpFloat(float f_origin, float f_target, float f_percent)
{
	return (f_origin - ((f_origin-f_target)*f_percent));
}

char *StringReverse(char *str)
{

	int length = (int)strlen(str);
	// we need temp pointers to the beginning and
	// end of the sequence we wish to reverse
	char *start = str, *end = str + length - 1;
	while(start < end)
	{
		// swap characters, and move pointers towards
		// the middle of the sequence
		char temp = *start;
		*start++ = *end;
		*end-- = temp;
	}
	// return the reversed string
	return str;
}

string RemoveTrailingBackslash(const string &path)
{
	string st = path;
	if (st.empty()) return st;

	while(st[st.size()-1] == '/' || st[st.size()-1] == '\\' )
	{
		st = st.substr(0, st.size()-1);
	}

	return st;
}

char *float_to_money( double num, char *buf, int dec)
{
      char tmp[256];
      int bf = 0, cm = 0, tm = 9 - dec + (!dec);
	 
	

	  sprintf(tmp, "%.9f", fabs(num));
      StringReverse(tmp);
      if(dec)
      {
            while( (buf[bf++] = tmp[tm++]) != '.')
                  ;
            while((buf[bf++] = tmp[tm++]) != 0)
            {
                  if(++cm % 3 == 0 && tmp[tm])
                        buf[bf++] = ',';
            }
      
			if (num < 0) 
			{
				buf[bf-1] = '-';
				buf[bf] = 0;
			}
      return StringReverse(buf);
      } else
      {
           //  while( (buf[bf++] = tmp[tm++]) != '.')
            //      ;
            while((buf[bf++] = tmp[tm++]) != 0)
            {
                  if(++cm % 3 == 0 && tmp[tm])
                        buf[bf++] = ',';
            }
	
			if (num < 0) 
			{
				buf[bf-1] = '-';
				buf[bf] = 0;
			}
      return StringReverse(buf);
   
      }
     return NULL;
}

string FloatToMoney(float f, int decimalsOfCents)
{
	char buf[256];
	float_to_money(double(f), buf, decimalsOfCents);
	return string(buf);
}

string DataToByteHexDisplay(string data, int maxNumCharsToShow)
{

	string hexData = "Hex data: ";

	char temp[255];
	for (int i = 0; i < maxNumCharsToShow && i < data.length(); i++)
	{
		sprintf(temp, "%hhx,", data[i]);
		hexData += (string(temp) + " ");
	}
	
	return hexData;
}

string DataToByteHexDisplay(byte *pData, int maxNumCharsToShow)
{

	string hexData = "Hex data: ";

	char temp[255];
	for (int i = 0; i < maxNumCharsToShow; i++)
	{
		sprintf(temp, "%hhx,", pData[i]);
		hexData += (string(temp) + " ");
	}

	return hexData;
}

string IntToMoneyBillions(int bil,int n)
{
	while(n>1000000000)
	{
		bil++;
		n-=1000000000;
	}
	;
	if(bil>0)
	{
		string s;
		int commaSteps=0;
		for(int i=1;i<1000000000;i*=10)
		{
			if(n<i) s="0"+s;
			else
				s=toString((n/i)%10)+s;
			commaSteps++;
			if(commaSteps==3)
			{
				commaSteps=0;
				s=","+s;
			}
		}
		return toString(bil)+s;
	}
	else
	{
		char buf[256];
		float_to_money(double(n), buf, 0);
		return string(buf);
	}
}

string PrefixLeading(const string input, unsigned int leadingCount, string leadingChar, string insertAfterPrefix)
{
	int charsNeeded = int(leadingCount)- (int)input.size();
	if (charsNeeded < 0) return input;
	string s;
	while (charsNeeded--) s += leadingChar;

	s += insertAfterPrefix;

	return s+input;
}


string PostfixLeading(const string input, unsigned int leadingCount, string leadingChar, string insertAfterPrefix)
{
	if (input.length() > leadingCount) return input;
	
	string s = input;
	while (s.length() < leadingCount) s += leadingChar;
	return s + insertAfterPrefix;
}

string PopFirstParmString(string *lineInOut, const string delimiter)
{
	vector<string> pieces = StringTokenize(*lineInOut, delimiter);

	*lineInOut = "";

	if (pieces.size() == 0)
		return "";


	for (uint32 i=1; i < pieces.size(); i++)
	{
		if ( !lineInOut->empty())
		{
			*lineInOut += delimiter;
		}
		*lineInOut += pieces[i];
	}

	return pieces[0];
}


int PopFirstParmStringAsInt(string *lineInOut, const string delimiter)
{
	vector<string> pieces = StringTokenize(*lineInOut, delimiter);

	*lineInOut = "";

	if (pieces.size() == 0)
		return 0;


	for (uint32 i=1; i < pieces.size(); i++)
	{
		if ( !lineInOut->empty())
		{
			*lineInOut += delimiter;
		}
		*lineInOut += pieces[i];
	}

	return atoi(pieces[0].c_str());
}


float PopFirstParmStringAsFloat(string *lineInOut, const string delimiter)
{
	vector<string> pieces = StringTokenize(*lineInOut, delimiter);

	*lineInOut = "";

	if (pieces.size() == 0)
		return 0;


	for (uint32 i=1; i < pieces.size(); i++)
	{
		if ( !lineInOut->empty())
		{
			*lineInOut += delimiter;
		}
		*lineInOut += pieces[i];
	}

	return (float)atof(pieces[0].c_str());
}

string PeekFirstParmString(string *lineInOut, const string delimiter)
{
	vector<string> pieces = StringTokenize(*lineInOut, delimiter);

	if (pieces.size() == 0)
		return "";

	return pieces[0];
}

vector<string> StringTokenize (const  string  & theString,  const  string  & theDelimiter )
{
	vector<string> theStringVector;

	if (!theString.empty())
	{
		size_t  start = 0, end = 0;

		while ( end != string::npos )
		{
			end = theString.find( theDelimiter, start );

			// If at end, use length=maxLength.  Else use length=end-start.
			theStringVector.push_back( theString.substr( start,
														 (end == string::npos) ? string::npos : end - start ) );

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			start = (   ( end > (string::npos - theDelimiter.size()) )
						?  string::npos  :  end + theDelimiter.size()    );
		}
	}

	return theStringVector;
}


bool in_range(int i_num, int i_low, int i_high)
{
	if (i_num > i_high) return false;
	if (i_num < i_low) return false;
	return true;
}

bool force_range(int * i_original, int i_min, int i_max)
{
	if (in_range(*i_original, i_min, i_max)) return false;
	if (*i_original < i_min) *i_original = i_min;
	if (*i_original > i_max) *i_original = i_max;
	return false;
}

void ForceRange(float &a, const float min, const float max)
{
	if (a < min) 
	{
		a = min;
		return;
	}
	
	if (a > max) 
	{
		a = max;
		return;
	}
}

string GetCountryCode() //return the 2 letter ISO country code, or 00 if we failed
{
	string country = "00";
	if (GetRegionString().length() == 5)
	{
		//looks right...
		country = ToLowerCaseString(GetRegionString().substr(3,2));
	}

	return country;
}

inline string TrimRight (const string & s, const string & t)
{ 
	string d (s); 
	string::size_type i (d.find_last_not_of (t));
	if (i == string::npos)
		return "";
	else
		return d.erase (d.find_last_not_of (t) + 1) ; 
}  // end of trim_right

string TrimLeft (const string & s, const string & t) 
{ 
	string d (s); 
	return d.erase (0, s.find_first_not_of (t)) ; 
}  // end of trim_left

string StripWhiteSpace(const string & s)
{
	string d (s); 
	return TrimLeft (TrimRight (d, RT_UTIL_SPACES), RT_UTIL_SPACES) ; 
}

string GetFileNameFromString(const string &path)
{
	if (path.empty()) return "";

	for (size_t i=path.size()-1; i > 0; i--)
	{
		if (path[i] == '/' || path[i] == '\\')
		{
			//well, this must be the cutoff point
			return path.substr(i+1, path.size()-i);
		}
	}
	return path;
}

string GetFileNameWithoutExtension(const string fileName)
{
	string fName = GetFileNameFromString(fileName);

	size_t dotIndex = fName.find_first_of('.');
	if (dotIndex == string::npos) return fName;
	return fName.substr(0, dotIndex);

}
string GetPathFromString(const string &path)
{

	for (size_t i=path.size()-1; i > 0; i--)
	{
		if (path[i] == '/' || path[i] == '\\')
		{
			//well, this must be the cutoff point
			return path.substr(0,i+1);
		}
	}
	return path;
}

//send the desired new extension without the peroid, like "zip", not ".zip"
string ModifyFileExtension(const string fileName, const string extension)
{
	size_t index = fileName.find_last_of('.');
	if (index == string::npos)
	{
		assert(!"Well, it doesn't have an extension to begin with");
		return fileName;
	}

	return fileName.substr(0, index+1) + extension;
}

string GetFileExtension(string fileName)
{
	size_t index = fileName.find_last_of('.');
	if (index == string::npos)
	{
		return "";
	}

	return fileName.substr(index+1, fileName.length());
}

void TruncateString(string &input, size_t len)
{
	if (input.length() > len)
	{
		input = input.substr(0, len);
	}
}

bool IsInString(const string &s, const char *search)
{
	if (s.find(search) != string::npos) return true;
	return false;
}

bool StartsWith(const std::string& text, const std::string& token)
{
	if (text.length() < token.length())
		return false;
	return (text.compare(0, token.length(), token) == 0);
}

void RotationToXYMod(float rotation, float *pXMod ,float *pYMod)
{
	switch (int(rotation))
	{
	case 0:

		*pXMod = 0; //no change
		*pYMod = 0; //no change
		return;
	case 90:
		*pXMod = 0; //no change
		*pYMod = 1; //change
		return;

	case 180:
		*pXMod = 1; //change!
		*pYMod = 1; //change
		return;

	case 270:
		*pXMod = 1; //chnage
		*pYMod = 0; //no change!
		return;

	}

	assert("Unknown degree");
	*pXMod = 0; //no change
	*pYMod = 0; //no change
	return;

}



bool isOrdinaryChar(char c, bool bStrict) 
{
	if (!bStrict)
	{
		if (c >= 32 && c <= 126)
		{
			return true;
		}
		return false;
	}

	if (
		(c >= 45 && c <= 46) ||
		(c >= 48 && c <= 57) ||
		(c >= 65 && c <= 90) ||
		(c >= 97 && c <= 122)) return true;

	return false;
}

string FilterToValidAscii(const string &input, bool bStrict)
{
	string output;

	for (unsigned int i=0; i < input.length(); i++)
	{

		if ( isOrdinaryChar(input[i], bStrict))
		{
			output += input[i];
		}
	}

	return output;
}

bool CaseInsensitiveCompare( const char*a, const char*b )
{
	int i = 0;
	while(1)
	{
		if (a[i] != b[i]) return false;

		if (a[i] == 0) return true;
		i++;
	}

	return true;
}

string IntToTimeSeconds(uint32 seconds, bool bTextFormat, bool showDays) //converts 180 (seconds) to 3:00, suitable for a game "time left" display.  if bTextformat, it would say "3 min" instead.
{
	uint32 originalSeconds = seconds;

	uint32 minutes = seconds/60;
	seconds -= minutes*60;

	uint32 hours = minutes/60;
	minutes -= hours*60;

	uint32 days = hours/24;
	hours -= days*24;

	string r;

	if (!bTextFormat)
	{
		char temp[24];
		
		if (hours > 0 || days > 0)
		{
			if(showDays)
				sprintf(temp, "%d:%02d:%02d", days , hours, minutes);
			else
				sprintf(temp, "%d:%02d:%02d", hours+(days*24), minutes, seconds);

		} else
		{
			sprintf(temp, "%d:%02d", minutes, seconds);
		}
		
		return string(temp);
	}


	if (originalSeconds <= 0)
	{
		return "None";
	}

	if (seconds > 0)
	{
		r += toString(seconds)+" secs";
	}

	if (minutes > 0)
	{
		if (!r.empty())
		{
			r = ", "+r;
		}
		string mins = " mins";

		if (minutes == 1)
		{
			mins = " min";
		}

		r = toString(minutes) +mins+r;

	}

	if (hours > 0)
	{
		if (!r.empty())
		{
			r = ", "+r;
		}

		string stHours = " hours";

		if (hours == 1)
		{
			stHours = " hour";
		}

		r = toString(hours) +stHours+r;
	}

	if (days > 0)
	{
		if (!r.empty())
		{
			r = ", "+r;
		}

		string stDays = " days";

		if (days == 1)
		{
			stDays = " day";
		}

		r = toString(days) +stDays+r;
	}

	if (r.empty()) return "Now!";
	return r;
}

string IntToTime(uint32 ms, bool bTextFormat)
{
	return IntToTimeSeconds(ms/1000, bTextFormat);
}

int StringToInt( const string &s )
{
	return atoi(s.c_str());
}

/*
//oops, this is C++11, can't use for crossplatform stuff yet
unsigned int StringToUInt32(const string& s)
{
	unsigned long lresult = std::stoul(s, 0, 10);
	unsigned int result = lresult;
	assert(result == lresult && "That's weird...");
	return result;
}
*/

float StringToFloat( const string &s )
{
	return (float)atof(s.c_str());
}

bool DateIsOlder(int month, int day, int year, int hour, int min, int sec,
				 int monthB, int dayB, int yearB, int hourB, int minB, int secB)
{

	if (year < yearB) return true;
	if (year > yearB) return false;

	if (month < monthB) return true;
	if (month > monthB) return false;

	if (day < dayB) return true;
	if (day > dayB) return false;
	if (hour < hourB) return true;
	if (hour > hourB) return false;
	if (min < minB) return true;
	if (min > minB) return false;
	if (sec < secB) return true;
	if (sec > secB) return false;

	//er, they are the same;
	return false;
}


string HexToString(string hexString)
{
	if (hexString.size() > 16)
	{
#ifdef _DEBUG
LogMsg("This isn't hex data, it's probably already stringified, returning as such");
return hexString;
#endif
	}
	if (hexString.empty()) return "";

	const unsigned char * pin = (const unsigned char*)hexString.c_str();
	const char * hex = "0123456789ABCDEF";
	char temp[64];
	char * pout = temp;
	uint32 i = 0;
	for(; i < hexString.size()-1; ++i){
		*pout++ = hex[(*pin>>4)&0xF];
		*pout++ = hex[(*pin++)&0xF];
		//*pout++ = ':';
	}
	*pout++ = hex[(*pin>>4)&0xF];
	*pout++ = hex[(*pin)&0xF];
	*pout = 0;

	return string(temp);
}

bool IsVowel(char c)
{
	return (c=='a' || c=='e' || c=='i' || c=='o' || c=='u' ||
		c=='A' || c=='E' || c=='I' || c=='O' || c=='U');
}

