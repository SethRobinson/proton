//  ***************************************************************
//  GameTimer - Creation date:  03/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GameTimer_h__
#define GameTimer_h__

class GameTimer
{
public:
	
	void Update();

	unsigned int GetTick() {return m_timeMS;}
	unsigned int GetGameTick() {return m_gameTimer;} //this timer can be paused with SetGameTimerPause(), use for all entity/object timing in the game
	void SetGameTick(unsigned int tick);
	int GetHistorySize();
	int GetFPS() {return m_fps;}
	float GetDeltaHistory();
	GameTimer();
	virtual ~GameTimer();
	float GetDelta() {return m_deltaFloat;}
	float GetGameDelta() {if (m_bGameTimerPaused) return 0; else return m_deltaFloat;}
	int GetDeltaTick() {return m_deltaMS;}
	int GetDeltaGameTick() {if (m_bGameTimerPaused) return 0; else return m_deltaMS;}
	void SetGameTickPause(bool bNew);
	bool GetGameTickPause() {return m_bGameTimerPaused;}
	void Reset();
	bool IsKosher(); //false if timer issues noticed

private:

	unsigned int m_lastTimeMS;
	unsigned int m_timeMS;
	unsigned int m_fpsTimer;
	unsigned int m_gameTimer;
	unsigned int m_shadowOffset;
	int m_fps;
	int m_fpsTemp;
	bool m_bGameTimerPaused;
	int m_deltaMS;
	float m_deltaFloat;
	std::deque <float> m_tickHistory;
	unsigned int m_shadowGameTick;


};

#endif // GameTimer_h__