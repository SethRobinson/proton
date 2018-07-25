//  ***************************************************************
//  AudioManagerOS - Creation date: 06/12/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AudioManagerOS_h__
#define AudioManagerOS_h__

#include "AudioManager.h"



class AudioObjectOS
{
public:
	AudioObjectOS()
	{
		m_id = AUDIO_HANDLE_BLANK;
	}

	void Unload();
	string fName;
	uint32 m_id;
	;
};

class AudioManagerOS: public AudioManager
{
public:
	AudioManagerOS();
	virtual ~AudioManagerOS();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	virtual AudioHandle PlayWithAVPlayer(string fName); //also doesn't cache and uses AV player, useful for the TTS system or oneshots
	virtual void Preload(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	virtual void Stop(AudioHandle id);
	virtual void SetMusicEnabled( bool bNew );
	virtual void Vibrate(int duration = 300);
	virtual void StopMusic();

protected:
	AudioObjectOS * GetAudioObjectByFileName(const string &fName, bool bLooping);
	void KillAudioObjectByFileName(const string &fName);
	void DestroyAudioCache();
	AudioObjectOS * GetAudioObjectByID(uint32 id);
	bool m_bDisabledMusicRecently;
	vector<AudioObjectOS> m_audioList;
};

#endif // AudioManagerOS_h__