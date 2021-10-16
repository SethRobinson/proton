#ifndef misc_util_h__
#define misc_util_h__

#include "PlatformSetup.h"


const int32 C_DINK_SCREENSIZE_X = 640;
const int32 C_DINK_SCREENSIZE_Y = 480;

void getdir(char final[]);

void dink_decompress (unsigned char *in, char * destBuf);
void decompress_nocomp (byte *in, char destBuf[]);
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif
void strchar(char *string, char ch) /* This acts in the same way as strcat except it combines a string and a single character, updating the null at the end. */;
bool separate_string (const char str[255], int num, char liney, char *return1);
void replace(const char *this1, char *that, char *line);
char * lmon(int money, char *dest);
void reverse(char *st);
#endif // misc_util_h__