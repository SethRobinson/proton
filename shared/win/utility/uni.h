/* Copyright (C) Seth A. Robinson, 2003. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Seth A. Robinson, 2003"
 */


/*

Converts between ANSI/UNICODE easily.

Can be used like this:

LogMsg(_T("the unicode string is: %s") , uni(st_ansi).us_data);

or:

uni u;

u.set("ANSI string");

//u.us_data now contains the wide character version

uni u(some_unicode_string);

u.st_data now contains the ANSI version.

GetAuto() will return ANSI or UNICODE depending on if you have _UNICODE defined.


*/  
    

#pragma once

#include "tchar.h"

//this must be defined in your program somewhere, if a serious error happens this will get
//called with info about it so you can display it on your status window or log or whatever.
//all my code uses this format, no I don't want to use a stream, I like printf style formatting.
void LogError(const TCHAR * lpFormat, ...);

class uni
{
    
public:
  
    uni( const char st_source[]);
    uni(int i_size);
    uni(const unsigned short *us_new);
    ~uni();
  
    void uni::set(const char st_source[]);
    void init_vars();
    
    char * to_st();
    void set_us( const unsigned short *us_new);
    
    TCHAR * GetAuto(); // returns the type, converted to match TCHAR
  
    unsigned short *us_data;
    char *st_data;

};
