#ifndef MiscUtils_h__
#define MiscUtils_h__

#include <string>

#include "PlatformSetup.h"

uint32 HashString(const char *str, int32 len=0); //if 0, stops on null, like for a string
unsigned int GetHashOfFile(std::string fName, bool bAddBasePath); //returns 0 if file doesn't exist

bool IsEven(int number);
void ToLowerCase(char *pCharArray);
void ToUpperCase(char *pCharArray);
std::string ToLowerCaseString (const std::string & s);
std::string ToUpperCaseString (const std::string & s);

bool CaseInsensitiveCompare(const char*a, const char*b);
void SetIntWithTarget(int32 *p_out_dest, int r_target, int r_amount);
int mod(int a, int b);
/**
 * Returns a random number limited by \a range.
 *
 * If \a range is positive the returned value is on the interval [0, range[.
 *
 * If \a range is negative the returned value is on the interval [0, -range[.
 */
int Random(int range);
/**
 * Returns a random number between \a rangeMin and \a rangeMax.
 * The range includes \a rangeMin but not \a rangeMax, in other words
 * the returned value is on the interval [rangeMin, rangeMax[.
 *
 * This function is meant to be used so that \a rangeMin < \a rangeMax.
 * However the behaviour is well defined for other combinations as well.
 * - If \a rangeMin equals \a rangeMax then \a rangeMin is returned.
 * - If \a rangeMin != \a rangeMax the returned value is on the interval
 *   [rangeMin, rangeMin+abs(rangeMax-rangeMin)[.
 */
int RandomRange(int rangeMin, int rangeMax);
/**
 * Returns a random floating point number between \a rangeMin and \a rangeMax.
 *
 * The behaviour of this function is similar to RandomRange(). The notes on
 * that function apply to this function too.
 *
 * \see RandomRange()
 */
float RandomRangeFloat(float rangeMin, float rangeMax);
float LerpFloat(float f_origin, float f_target, float f_percent);
std::string RemoveLastPartOfDir(std::string path);
std::string BoolToEnabled(bool bEnabled);
char *float_to_money( double num, char *buf, int dec); //for legacy code..
std::string FloatToMoney(float f, int decimalsOfCents = 0);
std::string DataToByteHexDisplay(std::string data, int maxNumCharsToShow);
std::string DataToByteHexDisplay(byte *pData, int maxNumCharsToShow);
std::string IntToMoneyBillions(int bil, int n);
std::string IntToTime(uint32 ms, bool bTextFormat = false); //converts 18000 (ms) to 3:00, suitable for a game "time left" display.  if bTextformat, it would say "3 min" instead.
std::string IntToTimeSeconds(uint32 seconds, bool bTextFormat = false , bool showDays = false); //converts 180 (seconds) to 3:00, suitable for a game "time left" display.  if bTextformat, it would say "3 min" instead.

/**
 * Splits \a str into multiple parts delimited by \a delimiter.
 *
 * Examples:
 *
 * \code
 * string str("foo_bar_baz");
 * StringTokenize(str, "_"); // Returned parts are "foo", "bar" and "baz"
 * \endcode
 *
 * \code
 * string str("foo***bar****baz");
 * StringTokenize(str, "**"); // Returned parts are "foo", "*bar", "" (an empty string) and "baz"
 * \endcode
 */
std::vector<std::string> StringTokenize(const std::string& str, const std::string& delimiter);
std::string PopFirstParmString(std::string *lineInOut, const std::string delimiter = "|"); //removes the first parm from the string sent in, and returns it
std::string PeekFirstParmString(std::string *lineInOut, const std::string delimiter="|");	// returns the first parm from the string, but leaves it intact
void ForceRange(float &a, const float min, const float max);

#define rt_min(rangeMin,rangeMax)    (((rangeMin) < (rangeMax)) ? (rangeMin) : (rangeMax))
#define rt_max(rangeMin,rangeMax)            (((rangeMin) > (rangeMax)) ? (rangeMin) : (rangeMax))

//helper to turn anything into a string, like ints/floats
template< class C>
std::string toString(C value)
{
	std::ostringstream o;
	o << value;
	return o.str();
}

int StringToInt(const std::string &s);
float StringToFloat(const std::string &s);
bool IsVowel(char c);

std::string PrefixLeading(const std::string input, unsigned int leadingCount, std::string leadingChar, std::string insertAfterPrefix = "");
std::string PostfixLeading(const std::string input, unsigned int leadingCount, std::string leadingChar, std::string insertAfterPrefix = "");

bool force_range(int * i_original, int i_min, int i_max);
std::string GetCountryCode(); //return the 2 letter ISO country code, or 00 if we failed
std::string StripWhiteSpace(const std::string &s);
#define RT_UTIL_SPACES " \t\r\n"
std::string TrimLeft (const std::string & s, const std::string & t = RT_UTIL_SPACES);
std::string TrimRight (const std::string & s, const std::string & t = RT_UTIL_SPACES);
std::string GetFileNameFromString(const std::string &path);
std::string GetFileNameWithoutExtension(const std::string fileName);
std::string GetPathFromString(const std::string &path);
std::string RemoveTrailingBackslash(const std::string &path);
std::string GetFileExtension(std::string fileName);
std::string ModifyFileExtension(const std::string fileName, const std::string extension);
void TruncateString(std::string &input, size_t len);
bool IsInString(const std::string &s, const char *search);
bool StartsWith(const std::string& text, const std::string& token);
void RotationToXYMod(float rotation, float *pXMod ,float *pYMod); //given a rotation in degrees, tells you what to flip
void SetFloatWithTarget(float *p_out_dest, float r_target, float r_amount);
std::string FilterToValidAscii(const std::string &input, bool bStrict);
bool isOrdinaryChar(char c, bool bStrict);
int GiveOrTake(int baseNum, int modAmount); //Let's you slightly randomize a #. (5, 2) would return between 3 and 7. (5, give or take 2)
bool DateIsOlder(int month, int day, int year, int hour, int min, int sec, int monthB, int dayB, int yearB, int hourB, int minB, int secB);
std::string HexToString(std::string hexString);
int PopFirstParmStringAsInt(std::string *lineInOut, const std::string delimiter);
float PopFirstParmStringAsFloat(std::string *lineInOut, const std::string delimiter);
#endif
