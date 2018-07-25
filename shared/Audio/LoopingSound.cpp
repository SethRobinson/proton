#include "PlatformPrecomp.h"
#include "LoopingSound.h"

LoopingSound::LoopingSound()
{
	m_loopingSoundHandle = AUDIO_HANDLE_BLANK;
	m_transitionSoundHandle = AUDIO_HANDLE_BLANK;
	m_moveStartTimeMS = 1400;
	m_moveEndTimeMS = 4300;
	m_bMoving = false;
	m_state = STATE_IDLE;
	m_bDisabled = false;
	m_volume = 1.0f;
}

LoopingSound::~LoopingSound()
{
	KillAudio();
}

void LoopingSound::KillAudio()
{
	//kill any currently playing looping sound
	
#ifdef _DEBUG
	//LogMsg("** Looping sound %s stopped", m_loopingIdle.c_str());
#endif

	if (m_loopingSoundHandle)
	{
		GetAudioManager()->Stop(m_loopingSoundHandle);
		m_loopingSoundHandle = AUDIO_HANDLE_BLANK;
	}

	if (m_transitionSoundHandle != AUDIO_HANDLE_BLANK)
	{
		GetAudioManager()->Stop(m_transitionSoundHandle);
		m_transitionSoundHandle = AUDIO_HANDLE_BLANK;
	}
}

void LoopingSound::PlayIdleSound()
{
	if (!m_loopingIdle.empty())
	{
		m_loopingSoundHandle = GetAudioManager()->Play(m_loopingIdle, true, false, m_bAddBasePath);
		GetAudioManager()->SetVol(m_loopingSoundHandle, m_volume);
	} else
	{
		m_loopingSoundHandle = NULL;
	}

	
	m_state = STATE_IDLE;
}
void LoopingSound::Init( string loopingMove, string moveStart /*= ""*/, string moveEnd /*= ""*/, string loopingIdle /*= ""*/, bool bAddBasePath )
{

	m_loopingIdle = loopingIdle;
	m_loopingMove = loopingMove;
	m_moveStart = moveStart;
	m_moveEnd = moveEnd;

	m_bAddBasePath = bAddBasePath;
	//preload as needed
	
	if (!m_moveStart.empty()) GetAudioManager()->Preload(m_moveStart, false, false, bAddBasePath);
	if (!m_moveEnd.empty()) GetAudioManager()->Preload(m_moveEnd, false, false, bAddBasePath);

	PlayIdleSound();
	
}

void LoopingSound::PlayMoveSound()
{
	if (!m_loopingMove.empty())
	{
		m_loopingSoundHandle = GetAudioManager()->Play(m_loopingMove, true, false, m_bAddBasePath);
		GetAudioManager()->SetVol(m_loopingSoundHandle, m_volume);

	} else
	{
		m_loopingSoundHandle = NULL;
	}
	m_state = STATE_MOVING;

}

void LoopingSound::SetMoving( bool bNew )
{
	if (m_bDisabled) return;

	if (bNew == m_bMoving) return; //nothing really changed

	m_bMoving = bNew;
  
	KillAudio();

	if (m_bMoving)
	{
	
		if (m_moveStart.empty())
		{
			//there is no move start sound, so just play the looping moving sound right now
			PlayMoveSound();
		} else
		{
			//play a starting sound first
			m_transitionSoundHandle =  GetAudioManager()->Play(m_moveStart, false, false, m_bAddBasePath);
			GetAudioManager()->SetVol(m_transitionSoundHandle, m_volume);

			m_waitTimer = GetTick(TIMER_SYSTEM)+m_moveStartTimeMS;
			m_state = STATE_MOVE_START;
		}
	} else
	{
		//stop moving and play the idle sound

		if (m_moveEnd.empty())
		{
			PlayIdleSound();
		} else
		{
			//play a stopping sound first
			m_transitionSoundHandle =  GetAudioManager()->Play(m_moveEnd, false, false, m_bAddBasePath);
			GetAudioManager()->SetVol(m_transitionSoundHandle, m_volume);

			m_waitTimer = GetTick(TIMER_SYSTEM)+m_moveEndTimeMS;
			m_state = STATE_MOVE_END;

		}

	}
}

void LoopingSound::Update()
{
	if (m_bDisabled) return;

	switch (m_state)
	{
	case STATE_MOVE_START:

		if (m_waitTimer < GetTick(TIMER_SYSTEM))
		{
			m_waitTimer = 0;
			//time to kick into the move loop sound
			GetAudioManager()->Stop(m_transitionSoundHandle);
			m_transitionSoundHandle = AUDIO_HANDLE_BLANK;

			PlayMoveSound();
		}
		break;

	case STATE_MOVE_END:

		if (m_waitTimer < GetTick(TIMER_SYSTEM))
		{
			m_waitTimer = 0;
			//time to kick into the move loop sound
			GetAudioManager()->Stop(m_transitionSoundHandle);
			m_transitionSoundHandle = AUDIO_HANDLE_BLANK;

			PlayIdleSound();
		}
		break;

	case STATE_IDLE:
		if (GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
		{
			//special hack to fix crappy SDL audio issues with looping cutting out
			if (m_loopingSoundHandle > 0 && !GetAudioManager()->IsPlaying(m_loopingSoundHandle))
			{
				//restart it
				#ifdef _DEBUG
				//LogMsg("Kickstarting idle loop again");
				#endif
				PlayIdleSound();
			}
		}


		break;
	}

}

void LoopingSound::SetTransitionTimings( int transitionStartMS, int transitionStopMS )
{
	m_moveStartTimeMS = transitionStartMS;
	m_moveEndTimeMS = transitionStopMS;

}

void LoopingSound::SetDisabled( bool bDisabled )
{
	if (bDisabled == m_bDisabled) return; //no change

	if (bDisabled)
	{
		KillAudio();
		m_bMoving = false;
	} else
	{
	  m_state = STATE_IDLE;
	  PlayIdleSound();
	}

	m_bDisabled = bDisabled;


}

void LoopingSound::SetVolume( float vol )
{
	if (vol == m_volume) return;

	m_volume = vol;

	if (m_bDisabled) return;

	switch (m_state)
	{
	case STATE_IDLE:
	case STATE_MOVING:

		if (m_loopingSoundHandle != AUDIO_HANDLE_BLANK)
		{
			GetAudioManager()->SetVol(m_loopingSoundHandle, vol);
		}
		break;
	}
}