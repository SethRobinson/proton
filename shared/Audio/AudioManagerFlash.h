//  ***************************************************************
//  AudioManagerFlash - Creation date: 06/08/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************


#ifndef AudioManagerFlash_h__
#define AudioManagerFlash_h__

#include "AudioManager.h"

class SoundObjectFlash
{
public:

	SoundObjectFlash()
	{
		m_bIsLooping	= false;
		m_bIsMusic		= false;
		m_volume = 1.0f;
		m_sampleID = AUDIO_HANDLE_BLANK;
	}

	void DeleteSample();
	~SoundObjectFlash()
	{
		DeleteSample();
	}

	std::string m_fileName;
	bool   m_bIsLooping;
	bool   m_bIsMusic;
	AudioHandle m_sampleID;
	float m_volume;
};

class AudioManagerFlash: public AudioManager
{
public:

	AudioManagerFlash();
	virtual ~AudioManagerFlash();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

	virtual void Preload(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

	SoundObjectFlash * GetSoundObjectByFileName(string fName);
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
	virtual void SetVol(AudioHandle soundID, float vol); //send -1 as the soundID to adjust the global volume
	virtual void SetPriority(AudioHandle soundID, int priority);
	virtual uint32 GetPos( AudioHandle soundID );
	virtual void SetPos( AudioHandle soundID, uint32 posMS );
	virtual void SetMusicVol(float vol);
	virtual void Suspend(); //stop all audio, app when into background or something
	virtual void Resume(); //restore audio that was stopped

	SoundObjectFlash * GetSoundObjectByPointer(void *p);

	void Testy();

private:
	
	list<SoundObjectFlash*>	m_soundList;
	float m_globalVol;

};


#endif // AudioManagerFlash_h__

