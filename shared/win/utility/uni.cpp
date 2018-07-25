/* Copyright (C) Seth A. Robinson, 2003. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Seth A. Robinson, 2003"
 */


//class for string conversions
#include "uni.h"       

#include <objbase.h>

	HRESULT AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW)
	{
    ULONG cCharacters;
    DWORD dwError;
    // If input is null then just return the same.
    if (NULL == pszA)
    {
        *ppszW = NULL;
        return NOERROR;
    }
 
    // Determine number of wide characters to be allocated for the     // Unicode string.
    cCharacters =  strlen(pszA)+1;
 
    // Use of the OLE allocator is required if the resultant Unicode     // string will be passed to another COM component and if that
	// component will free it. Otherwise you can use your own allocator.   
    *ppszW = (LPOLESTR) malloc(cCharacters*2);
    if (NULL == *ppszW)
    {
    
       LogError(_T("Out of memory in UNI, serious error."));
		return E_OUTOFMEMORY;
    }
  	
	// Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
                  *ppszW, cCharacters))
    {
        dwError = GetLastError();
        LogError(_T("Giant ass error %d in Uni::MultiByteToWideChar"), dwError);
        free(*ppszW);
        *ppszW = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
 
    return NOERROR;
 
}
 
/*
* UnicodeToAnsi converts the Unicode string pszW to an ANSI string * and returns the ANSI string through ppszA. Space for the
* the converted string is allocated by UnicodeToAnsi.
*/
 
HRESULT UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{
    ULONG cbAnsi, cCharacters;
    DWORD dwError;
 
    // If input is null then just return the same.
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return NOERROR;
    }
 
    cCharacters = wcslen(pszW)+1;
    // Determine number of bytes to be allocated for ANSI string. An
    // ANSI string can have at most 2 bytes per character (for Double     // Byte Character Strings.)
    cbAnsi = cCharacters*2;
 
    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You     // can use your own allocator.
    *ppszA = (LPSTR) malloc(cbAnsi);
    if (NULL == *ppszA)
        return E_OUTOFMEMORY;
 
    // Convert to ANSI.
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,                   cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        free(*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
 
    return NOERROR;
}
 

uni::uni(const char st_source[])
{
 init_vars();
  //convert source to unicode
  
 set(st_source);
 //let's also store a copy in our own st_data for no good reason
 if (st_source)
 {
 
 st_data = (CHAR*) malloc(strlen(st_source)+1);
 strcpy(st_data, st_source);
 }

}

char * uni::to_st()
{
  if (this->st_data)
  {
      free(st_data);
      st_data = NULL;
  }
 
  if (us_data == NULL)
  {
      LogError(_T("US_data equaled NULL in uni.cpp, trying to fix."));
      set_us((unsigned short*)L"NULL");
  }

  //make a copy to a string
  UnicodeToAnsi((LPCOLESTR)this->us_data, &st_data);
 
  //return pointer to converted string
  return st_data;
}

TCHAR * uni::GetAuto()
{
#ifdef _UNICODE
    return us_data;  //unicode mode, get as unicode
#else
    return st_data; //we're compiling in ANSI mode, get ansi
#endif
}

void uni::init_vars()
{
  us_data = NULL;
  st_data = NULL;

}

uni::uni(int i_size)
{
 	init_vars();
  //init unicode member
  	if (i_size > 0)
	{
	
	//LogMsg("Created string of %d bytes.",i_size);
	this->us_data = (unsigned short*) malloc(2*i_size);
 	memset(us_data,0,2*i_size);
	}
}


void uni::set(const char st_source[])
{
 	
    if (us_data) 
	{
		free(us_data);
		us_data = NULL;
	}
	AnsiToUnicode(st_source, (LPOLESTR*)&this->us_data);
	return;
}

uni::~uni()
{
  //free any memory
  
	if (us_data) free(us_data);
	if (st_data) free(st_data);
}

void uni::set_us(const unsigned short *us_new)
{
   	if (us_data)
	{
	  	free(us_data);
		us_data = NULL;
	}
    if (!us_new)
    {
        //big time error
        return;
    }
   
    int i_size = wcslen((wchar_t*)us_new)+1; //it's a wide char, so we need 2
	this->us_data = (unsigned short*) malloc(2*i_size);
	memcpy(us_data,us_new,2*i_size);
	to_st();
}


uni::uni(const unsigned short *us_new)
{
  	init_vars();
	set_us(us_new);

}