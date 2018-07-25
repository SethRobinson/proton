//  ***************************************************************
//  AudioManagerAudiere - Creation date: 12/14/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson
//  ***************************************************************

/*
Audiere is an open source (LPGL) audio system

It uses DirectSound or WinMM in windows, OSS on Linux

Supports .wav, ogg, flac, mp3, mod/xm/s3m, midi (through MCI)

http://audiere.sourceforge.net/home.php
*/

#ifndef AudioManagerAudiere_h__
#define AudioManagerAudiere_h__

#include "AudioManager.h"

#if !defined RT_WEBOS && !defined (ANDROID_NDK)


#include "audiere/include/audiere.h"

using namespace audiere;

class SoundObject
{
public:

	SoundObject()
	{
		m_pSound		= NULL;
		m_bIsLooping	= false;
		m_bIsMusic		= false;

	}

	~SoundObject()
	{
		if (m_pSound)
		{
			m_pSound = 0;
		}
	}

	OutputStreamPtr m_pSound;
	//FMOD::Sound *m_pSound;
	std::string m_fileName;
	bool   m_bIsLooping;
	bool   m_bIsMusic;
	//FMOD::Channel *m_pLastChannelToUse;
};

class AudioManagerAudiere: public AudioManager
{
public:
	AudioManagerAudiere();
	virtual ~AudioManagerAudiere();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(std::string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	
	virtual void Preload(std::string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

	SoundObject * GetSoundObjectByFileName(std::string fName);
	virtual void KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying);
	virtual void Update();
	virtual void Stop(AudioHandle soundID);
	virtual AudioHandle GetMusicChannel();
	virtual bool IsPlaying(AudioHandle soundID);
	virtual void SetMusicEnabled(bool bNew);
	virtual void StopMusic();
	virtual int GetMemoryUsed();
	bool DeleteSoundObjectByFileName(std::string fName);
	virtual void SetFrequency(AudioHandle soundID, int freq);
	virtual void SetPan(AudioHandle soundID, float pan); //0 is normal stereo, -1 is all left, +1 is all right
	virtual void SetVol(AudioHandle soundID, float vol); //-1 for global vol
	virtual void SetPriority(AudioHandle soundID, int priority);
	virtual uint32 GetPos( AudioHandle soundID );
	virtual void SetPos( AudioHandle soundID, uint32 posMS );
	virtual void SetMusicVol(float vol);
	SoundObject * GetSoundObjectByPointer(void *p);

private:
	
	AudioDevicePtr	   m_pDevice;

	std::list<SoundObject*>	m_soundList;
	float m_globalVol;

protected:


private:
};


#endif // AudioManagerAudiere_h__
#endif