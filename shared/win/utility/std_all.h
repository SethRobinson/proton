/* Copyright (C) Seth A. Robinson, 2003. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Seth A. Robinson, 2003"
 */

#pragma once
#include "all.h"
#include <ctime>
#include <cassert>
#include <string>

using namespace std;

bool force_range(int * i_original, int i_min, int i_max);
bool fforce_range(float * i_original, float i_min, float i_max);
bool lforce_range(long * l_original, long l_min, long l_max);
int GetDaysSinceDate(int i_month,int i_day, int i_year);
int GetDaysSinceDate(time_t from_time);
string ToLowerCaseString (const string & s);
void StringReplace(const std::string& what, const std::string& with, std::string& in);

bool SeparateString (const char str[], int num, char liney, char *return1);
#ifndef _UNICODE
char *show_date(void);
char *show_time(void);
char *show_small_date(void);
char *show_date_month_and_day(void);

#else


TCHAR *show_date(void);
TCHAR *show_small_date(void);
TCHAR *show_time(void);
#endif
char *show_army_time(void);

bool WindowsIs9xVersion();
float lerp_float(float f_origin, float f_target, float f_percent);
void set_float_with_target(float *p_float, float f_target, float f_friction);
void set_long_with_target(long *p_long, long f_target, long f_friction);
void create_url_file(char url[255]);
void winall_create_url_file_full(char url[]);
void winall_create_url_file(char url[255]);

