//  ***************************************************************
//  AudioManagerDenshion - Creation date: 02/16/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AudioManagerDenshion_h__
#define AudioManagerDenshion_h__

#include "AudioManager.h"



class AudioManagerDenshion: public AudioManager
{
public:
	AudioManagerDenshion();
	virtual ~AudioManagerDenshion();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	virtual AudioHandle PlayWithAVPlayer(string fName); //also doesn't cache and uses AV player, useful for the TTS system or oneshots
	virtual void Preload(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	virtual void Stop(AudioHandle id);
	virtual void SetMusicEnabled( bool bNew );
	virtual void Vibrate(int duration = 300);
	virtual void StopMusic();
	virtual void Update();
	virtual void FadeOutMusic(unsigned int duration = 1000);
	virtual bool IsPlaying(AudioHandle soundID);
	virtual void SetVol( AudioHandle soundID, float vol );
	virtual void SetMusicVol(float vol );

protected:
	void DestroyAudioCache();
	bool m_bDisabledMusicRecently;
	
	bool m_musicFadeOutInProgress;
	unsigned int m_musicFadeOutStartTime;
	unsigned int m_musicFadeOutDuration;
	unsigned int m_musicFadeOutPreviouslySetTime;
	
};

#endif // AudioManagerDenshion_h__