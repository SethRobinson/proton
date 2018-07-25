#include "PlatformPrecomp.h"
#include "GameTimer.h"

#define C_MAXIMUM_DELTA_ALLOWED 100 //phones aren't allowed to be slower than this

//this FPS averaging isn't needed for windows, but the iphone timer seems to be pretty crap...
#define FPS_AVERAGING_HISTORY 8  //average out the last X amount of frames for a visually smoother fps, due to crap timing

#define C_SHADOW_OFFSET 3

uint32 g_lastGameTime = 0; //used by Seth, don't remove 


#define MAX_MS_TICKS_PER_FRAME 100

uint32 GetSystemTimeAccurateRangeChecked()
{

	static uint32 tick = uint32(0);
	static uint32 lasttick = uint32(GetSystemTimeAccurate());

	uint32 thisTick = uint32(GetSystemTimeAccurate());
	uint32 change = thisTick-lasttick;
	
	lasttick = thisTick;
	
	if (change > MAX_MS_TICKS_PER_FRAME)
	{
		change = MAX_MS_TICKS_PER_FRAME;
	}
	tick += change;
	return tick;
}

void GameTimer::SetGameTick( unsigned int tick )
{
	m_gameTimer = tick;
}


int GameTimer::GetHistorySize()
{
	return m_tickHistory.size();
}

float GameTimer::GetDeltaHistory()
{
	float history = 0;

	for (uint32 i=0; i < m_tickHistory.size(); i++)
	{
		history += m_tickHistory[i];
	}

	return history;
}


GameTimer::GameTimer()
{
	m_shadowOffset =0;
	m_shadowGameTick = m_lastTimeMS = m_timeMS = g_lastGameTime = 0;
	m_bGameTimerPaused = false;
	m_fps = m_fpsTemp = 0;
	m_fpsTimer = 0;
	m_deltaFloat = 1;
	m_gameTimer = 0;
}

GameTimer::~GameTimer()
{
}

void GameTimer::Update()
{

	m_timeMS = g_lastGameTime = uint32(GetSystemTimeAccurateRangeChecked());
	
	m_deltaMS =  m_timeMS - m_lastTimeMS;
	//if (m_deltaMS == 0) goto loop;
	if (m_deltaMS > C_MAXIMUM_DELTA_ALLOWED) m_deltaMS = C_MAXIMUM_DELTA_ALLOWED;

	m_tickHistory.push_back(float(m_deltaMS));
	if (m_tickHistory.size() > FPS_AVERAGING_HISTORY)
	{
		m_tickHistory.pop_front();
	}
	float tempDelta = 0;

	for (uint32 i=0; i < m_tickHistory.size(); i++)
	{
		tempDelta += float(m_tickHistory[i]);
	}
	tempDelta /= float(m_tickHistory.size());


	int tickDiff = int32(m_deltaMS)-int32(tempDelta);

	m_deltaMS = (int)tempDelta;
	m_lastTimeMS =  m_timeMS;

	if (tickDiff >0 && tickDiff < 5)
	{
		//attempt to compensate for our tweaks so we don't slow/speed up time in the general scheme of things
		m_lastTimeMS -= tickDiff;
	}

	//we have a maximum delta because above a certain point all collision detection would
	//be broken. 

	if (!m_bGameTimerPaused)
	{
		//advance game timer 
		m_gameTimer += m_deltaMS;
		m_shadowGameTick += (m_deltaMS+C_SHADOW_OFFSET);
		m_shadowOffset += C_SHADOW_OFFSET;
	} else
	{

	}
	
	m_deltaFloat = float(m_deltaMS)/ (1000.0f/50.0f); //plan on 50 FPS being about average, which will return 1.0
	
	//LogMsg("Delta: %.5f", m_deltaFloat);
	//if (m_deltaFloat == 0) m_deltaFloat = 0.0000001; //avoid divide by 0 errors later

	if (m_fpsTimer < m_timeMS)
	{
		m_fpsTimer = m_timeMS+1000;
		m_fps = m_fpsTemp;
		m_fpsTemp = 0;
	}

	//assert(m_timeMS == m_gameTimer);
	m_fpsTemp++;
}

void GameTimer::Reset()
{
	m_lastTimeMS = m_timeMS = g_lastGameTime = uint32(GetSystemTimeAccurateRangeChecked()); //make sure this is valid now
	m_bGameTimerPaused = false;
	m_fps = m_fpsTemp = 0;
	m_fpsTimer = 0;
	m_deltaFloat = 1;
}

bool GameTimer::IsKosher()
{
	return (m_gameTimer == (m_shadowGameTick-m_shadowOffset));
}

void GameTimer::SetGameTickPause( bool bNew )
{
 m_bGameTimerPaused = bNew;
}