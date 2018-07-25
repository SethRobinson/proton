/*
 * A fancy pants profiler (version 1.0)
 * 
 * See "profile.h" for usage information.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code.
 *
 * Portions Copyright (C) Steve Rabin, 2000
 * Portions Copyright (C) Matthijs Hollemans, 2001
 */

#include "MiscUtils.h"
#include "Profile.h"

bool g_profile_pause = false;


/****************************************************************************/

// Describes the contents of a single sample point for the current 
// frame. In every new frame all the sample points are cleared.
typedef struct 
{
	bool isValid;         // Whether this array cell is being used
	bool isOpen;          // is profile_begin but _end not yet?
	int callCount;        // Number of times called in this frame
	char name[256];       // Name of sample point
	long startTime;       // The current open profile start time
	long totalTime;       // All samples this frame added together
	long childrenTime;    // Time taken by all children
	int numParents;       // Number of profile parents
} 
sample_point_t;

// Describes the history of a sample point.
typedef struct 
{
	bool isValid;         // Whether this array cell is being used
	char name[256];       // Name of the sample point
	float avg;            // Average time per frame (percentage)
	float min;            // Minimum time per frame (percentage)
	float max;            // Maximum time per frame (percentage)
}
sample_history_t;

// The maximum number of supported sample points.
#define MAX_SAMPLE_POINTS 50

static sample_point_t samples[MAX_SAMPLE_POINTS];
static sample_history_t history[MAX_SAMPLE_POINTS];

// The times at which the current frame started and ended.
static long startTime;
static long endTime;

// Adds a new measurement for a certain sample point to the history.
static void store_profile_in_history(char* name, float percent);

// Gets the statistics for a certain sample point from the history.
static void get_profile_from_history(
	char* name, float* ave, float* min, float* max);

/*****************************************************************************
 profile_init
*****************************************************************************/

void profile_init(void)
{

	for (int i = 0; i < MAX_SAMPLE_POINTS; ++i) 
	{
		samples[i].isValid = false;
		samples[i].isOpen = false;
		history[i].isValid = false;
	}

	startTime = GetSystemTimeTick();
	endTime = startTime;
}

/*****************************************************************************
 profile_begin
*****************************************************************************/

void profile_begin(char* name)
{
	if (g_profile_pause) return;

	int i = 0;

	// Was this sample point used earlier this frame?
	while ((i < MAX_SAMPLE_POINTS) && samples[i].isValid) 
	{
		if (strcmp(samples[i].name, name) == 0) 
		{
			if (samples[i].isOpen) 
			{
				assert(!"PROFILE_BEGIN called without a PROFILE_END");
			}

			samples[i].isOpen = true;
			samples[i].callCount++;
			samples[i].startTime = GetSystemTimeTick();

			return;
		}

		i++;	
	}
	
	if (i >= MAX_SAMPLE_POINTS) 
	{
		assert(!"Exceeded max available sample points");
		return;
	}

	// This a new sample point (we haven't seen it in this frame yet).
	strcpy(samples[i].name, name);
	samples[i].isValid = true;
	samples[i].isOpen = true;
	samples[i].callCount = 1;
	samples[i].startTime = GetSystemTimeTick();
	samples[i].totalTime = 0;
	samples[i].childrenTime = 0;
}

/*****************************************************************************
 profile_end
*****************************************************************************/

void profile_end(char* name)
{
	if (g_profile_pause) return;

	int i = 0;

	while ((i < MAX_SAMPLE_POINTS) && samples[i].isValid)
	{
		if (strcmp(samples[i].name, name) == 0)
		{
			if (!samples[i].isOpen) 
			{
				assert(!"PROFILE_END called without a PROFILE_BEGIN");
			}

			samples[i].isOpen = false;

			int parent = -1;
			int inner = 0;

			samples[i].numParents = 0;
			
			// Find the immediate parent to this sample point.
			while (samples[inner].isValid) 
			{
				// Any open profiles are parents...
				if (samples[inner].isOpen)
				{  
					samples[i].numParents++;

					// Replace invalid parent (index).
					if (parent < 0)
					{
						parent = inner;
					}
					// Replace with more immediate parent.
					else if (samples[inner].startTime 
								>= samples[parent].startTime)
					{
						parent = inner;
					}
				}

				inner++;
			}
			
			endTime = GetSystemTimeTick();

			samples[i].totalTime += endTime - samples[i].startTime;
			
			// Add this sample point's time to the parent's child total.
			if (parent >= 0)
			{  
				samples[parent].childrenTime 
					+= endTime - samples[i].startTime;
			}
			return;
		}

		i++;	
	}
}

/*****************************************************************************
 profile_tally
*****************************************************************************/

void profile_tally(void)
{
	if (g_profile_pause) 
	{
	 // g_profile_pause = false;
	  return;
	}
	long elapsed = endTime - startTime;
	if (elapsed > 0)
	{
		char line[256], name[256], indentedName[256];
		char ave[16], min[16], max[16], num[16], cur[16];
	
		profile_clear_buffer();
		profile_write_buffer("  Cur% |  Avg% |  Min% |  Max% |   # | Sample Name\n");
		profile_write_buffer("-------+-------+-------+-------+-----+----------------\n");
	
		int i = 0;
	
		while ((i < MAX_SAMPLE_POINTS) && samples[i].isValid) 
		{		
			// Figure out how long this sample point took 
			// relative to the others.
			long sampleTime = samples[i].totalTime - samples[i].childrenTime;
			float percentTime = 100.0f * sampleTime / elapsed;
			
			float aveTime = percentTime;
			float minTime = percentTime;
			float maxTime = percentTime;
	
			// Add new measurement into the history.
			store_profile_in_history(samples[i].name, percentTime);
	
			// And get the average, min, and max.
			get_profile_from_history(
				samples[i].name, &aveTime, &minTime, &maxTime);
			
			// Format the data.
			sprintf(cur, "%3.1f", percentTime);
			sprintf(ave, "%3.1f", aveTime);
			sprintf(min, "%3.1f", minTime);
			sprintf(max, "%3.1f", maxTime);
			sprintf(num, "%3d", samples[i].callCount);
			
			strcpy(indentedName, samples[i].name);
	
			// Rather inefficient indenting algorithm...
			for (int indent = 0; indent < samples[i].numParents; ++indent)
			{
				sprintf(name, "   %s", indentedName);
				strcpy(indentedName, name);
			}
			
			sprintf(line, " %5s | %5s | %5s | %5s | %3s | %s\n", 
							cur, ave, min, max, num, indentedName);
	

			profile_write_buffer(line);
	
			i++;
		}
	}

	// Reset sample points for next frame.
	for (int i = 0; i < MAX_SAMPLE_POINTS; ++i) 
	{
		samples[i].isValid = false;
	}

	startTime = GetSystemTimeTick();
}

/*****************************************************************************
 store_profile_in_history
*****************************************************************************/

void store_profile_in_history(char* name, float percent)
{
	// Determine how much the new value impacts the average percentage.
	float newRatio = 0.1f;
	float oldRatio = 1.0f - newRatio;

	int i = 0;
	
	// Update existing sample point in history.
	while ((i < MAX_SAMPLE_POINTS) && history[i].isValid) 
	{
		if (strcmp(history[i].name, name) == 0)
		{  
			history[i].avg = (history[i].avg * oldRatio) 
									+ (percent * newRatio);

			if (percent < history[i].min) 
			{
				history[i].min = percent;
			}
			else
			{
				history[i].min = (history[i].min * oldRatio) 
									+ (percent * newRatio);
			}
	
			if (history[i].min < 0.0f) 
			{
				history[i].min = 0.0f;
			}
	
			if (percent > history[i].max) 
			{
				history[i].max = percent;
			}
			else
			{
				history[i].max = (history[i].max * oldRatio) 
									+ (percent * newRatio);
			}

			return;
		}

		i++;
	}

	if (i >= MAX_SAMPLE_POINTS)
	{  
		assert(!"Exceeded max available sample points");
	}

	// Add new sample point to history.
	strcpy(history[i].name, name);
	history[i].isValid = true;
	history[i].avg = percent;
	history[i].min = percent;
	history[i].max = percent;
}

/*****************************************************************************
 get_profile_from_history
*****************************************************************************/

void get_profile_from_history(char* name, float* ave, float* min, float* max)
{
	int i = 0;

	while ((i < MAX_SAMPLE_POINTS) && history[i].isValid) 
	{
		if (strcmp(history[i].name, name) == 0)
		{  
			*ave = history[i].avg;
			*min = history[i].min;
			*max = history[i].max;
			return;
		}

		i++;
	}	

	// This sample was not found in the history
	*ave = *min = *max = 0.0f;
}

void profile_set_paused( bool bPause )
{
	g_profile_pause = bPause;
}

bool profile_get_paused()
{
	return g_profile_pause;
}