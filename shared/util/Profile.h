//Change by Seth to be compatible with Proton

//must define ENABLE_PROFILER in your project-wide somewhere for this to work

//#define ENABLE_PROFILER

/*
 * A fancy pants profiler (version 1.2)
 * 
 * This source code is based on Steve Rabin's article
 * "Real-Time In-Game Profiling" from the book Game
 * Programming Gems <http://www.gameprogramminggems.com>
 *
 * Changed around a bit by Matthijs Hollemans
 * <matthijs@shakeyourass.org>
 *
 * This is how you use the profiler:
 *
 *    1. Add the files "Profile.h," "Profile.cpp,"
 *   
 *
 *    2. In the source(s) that you want to profile,
 *       first #define the symbol ENABLE_PROFILER,
 *       and then include "Profile.h". Without this 
 *       symbol defined, the PROFILE_* macros are 
 *       empty, and the profiler code won't execute. 
 *
 *    3. Structure your main loop like this:
 *
 *           PROFILE_INIT
 *       
 *           while (!finished)
 *           {
 *               PROFILE_BEGIN("Main loop")
 *       
 *               PROFILE_BEGIN("Do stuff")
 *               ... do stuff ...
 *               PROFILE_END("Do stuff")
 *       
 *               PROFILE_BEGIN("Do other stuff")
 *               ... do other stuff ...
 *               PROFILE_END("Do other stuff")
 *       
 *               ... do main stuff ...
 *       
 *               PROFILE_END("Main loop")
 *               PROFILE_TALLY
 *           }
 *
 *       PROFILE_INIT must be called once before the
 *       main loop starts; it sets up the profiler and
 *       initializes the high-resolution timer. 
 * 
 *       Inside the loop, you wrap the pieces of code 
 *       that you want to measure inside PROFILE_BEGIN 
 *       and PROFILE_END blocks, giving each of these 
 *       sample points a unique name. 
 *
 *       Don't forget to call PROFILE_TALLY at the end 
 *       of each iteration. This turns the measurements
 *       into a bunch of statistics.
 *
 *    4. Provide the following two functions:
 * 
 *           void profile_clear_buffer(void);
 *           void profile_write_buffer(const char* text);
 *
 *       These are called by PROFILE_TALLY, and should
 *       be used to write the profile statistics into
 *       some kind of text buffer or log file.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code.
 *
 * Portions Copyright (C) 2000 Steve Rabin
 * Portions Copyright (C) 2001 Matthijs Hollemans 
 */

#ifndef PROFILE_H
#define PROFILE_H

#ifdef ENABLE_PROFILER

#define PROFILE_INIT profile_init();
#define PROFILE_BEGIN(name) profile_begin(name);
#define PROFILE_END(name) profile_end(name);
#define PROFILE_TALLY profile_tally();
#define PROFILE_SET_PAUSED(name) profile_set_paused(name);
#define PROFILE_GET_PAUSED profile_get_paused()

void profile_init(void);
void profile_begin(char* name);
void profile_end(char* name);
void profile_tally(void);

void profile_set_paused(bool bPause);
bool profile_get_paused();

#else

#define PROFILE_INIT
#define PROFILE_BEGIN(name)
#define PROFILE_END(name)
#define PROFILE_TALLY
#define PROFILE_SET_PAUSED(name)
#define PROFILE_GET_PAUSED true
#endif

extern void profile_clear_buffer(void);
extern void profile_write_buffer(const char* text);
extern bool g_profile_pause; //set to true to stop updating and freeze screen

#endif // PROFILE_H
