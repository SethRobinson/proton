//  ***************************************************************
//  LoopingSound - Creation date: 02/15/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef LoopingSound_h__
#define LoopingSound_h__

#include "BaseApp.h"
#include "AudioManager.h"

class LoopingSound
{
public:
	
	enum eState
	{
		STATE_IDLE,
		STATE_MOVE_START,
		STATE_MOVING,
		STATE_MOVE_END

	};
	
	LoopingSound();
	virtual ~LoopingSound();

	void Init(string loopingMove, string moveStart = "", string moveEnd = "", string loopingIdle = "", bool bAddBasePath = true);
	void SetMoving(bool bNew);
	void SetTransitionTimings(int transitionStartMS, int transitionStopMS);
	void Update(); //needs to be called each frame
	void SetDisabled(bool bDisabled);
	void SetVolume(float vol); //0 to 1

private:

	void PlayMoveSound();
	void PlayIdleSound();
	void KillAudio();

	string m_loopingMove;
	string m_moveStart;
	string m_moveEnd;
	string m_loopingIdle;

	int m_moveStartTimeMS;
	int m_moveEndTimeMS;

	AudioHandle m_loopingSoundHandle;
	AudioHandle m_transitionSoundHandle;
	bool m_bMoving;

	uint32 m_waitTimer;
	eState m_state;
	bool m_bDisabled;
	bool m_bAddBasePath;
	float m_volume;

};

#endif // LoopingSound_h__