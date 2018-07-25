//  ***************************************************************
//  AudioManagerFMODStudio - Creation date: 08/31/2017
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2017 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this works with FMOD Studio 1.09 - to keep compatibility with projects that use FmodEx, I've left AudioManagerFMOD intact for that, and this file should be used instead for FMODStudio

#ifndef AudioManagerFMOD_h__
#define AudioManagerFMOD_h__

#include "AudioManager.h"

#if !defined RT_WEBOS && !defined (ANDROID_NDK)

#include "fmodstudio/api/lowlevel/inc/fmod.hpp"
#include "fmodstudio/api/lowlevel/inc/fmod_errors.h"

class SoundObject
{
public:

	SoundObject()
	{
		m_pSound = NULL;
		m_bIsLooping = false;
		m_pLastChannelToUse = NULL;
	}

	~SoundObject()
	{
		if (m_pSound)
		{
			m_pSound->release();
			m_pSound = NULL;
			m_pLastChannelToUse = NULL;
		}
	}
	FMOD::Sound *m_pSound;
	string m_fileName;
	bool m_bIsLooping;
	FMOD::Channel *m_pLastChannelToUse;
};

class AudioManagerFMOD: public AudioManager
{
public:
	AudioManagerFMOD();
	virtual ~AudioManagerFMOD();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);
	virtual AudioHandle Play(string fName, int vol, int pan = 0);
	
	virtual void Preload(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

	SoundObject * GetSoundObjectByFileName(string fName);
	virtual void KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying);
	virtual void Update();
	virtual void Stop(AudioHandle soundID);
	virtual AudioHandle GetMusicChannel();
	virtual bool IsPlaying(AudioHandle soundID);
	virtual void SetMusicEnabled(bool bNew);
	virtual void StopMusic();
	virtual int GetMemoryUsed();
	bool DeleteSoundObjectByFileName(string fName);
	virtual void SetFrequency(AudioHandle soundID, int freq);
	virtual void SetPan(AudioHandle soundID, float pan); //0 is normal stereo, -1 is all left, +1 is all right
	virtual void SetVol(AudioHandle soundID, float vol);
	virtual void SetPriority(AudioHandle soundID, int priority);
	void SetGlobalPause(bool bPaused);
	virtual uint32 GetPos(AudioHandle soundID);
	virtual void SetPos( AudioHandle soundID, uint32 posMS );
	virtual void SetMusicVol(float vol);
	virtual void Suspend(); //stop all audio, app when into background or something
	virtual void Resume(); //restore audio that was stopped
	virtual string GetAudioSystemName(){ return "fmodstudio"; }
	virtual void ReinitForHTML5();

	FMOD::System * GetSystem() {return system;}

private:
	
	FMOD::System     *system;
	list<SoundObject*> m_soundList;
	FMOD::Channel *m_pMusicChannel;
	
protected:


private:
};

void FMOD_ERROR_CHECK(FMOD_RESULT result);

#endif // AudioManagerFMOD_h__
#endif