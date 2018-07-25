//  ***************************************************************
//  AudioManagerBBX - Creation date: 11/7/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved


#ifndef AudioManagerBBX_h__
#define AudioManagerBBX_h__

#include "AudioManager.h"

#include <stdio.h>
#include <unistd.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <mm/renderer.h>
#include <sys/stat.h>

class SoundObject
{
public:

	SoundObject()
	{
		m_bIsLooping	= false;
		m_bIsMusic		= false;
		buffer = NULL;
		source = NULL;
		m_volume = 1.0f;
	}

	~SoundObject()
	{

		if (buffer)
		{
			alDeleteBuffers(1, &buffer);
			buffer = NULL;
		}

		if (source)
		{
			alDeleteSources(1, &source);
			source = NULL;
		}
	}

	string m_fileName;
	bool   m_bIsLooping;
	bool   m_bIsMusic;

	ALuint buffer;
	ALuint source;
	float m_volume;
};

class AudioManagerBBX: public AudioManager
{
public:

	AudioManagerBBX();
	virtual ~AudioManagerBBX();

	virtual bool Init();
	virtual void Kill();

	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

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
	virtual void SetVol(AudioHandle soundID, float vol); //send -1 as the soundID to adjust the global volume
	virtual void SetPriority(AudioHandle soundID, int priority);
	virtual uint32 GetPos( AudioHandle soundID );
	virtual void SetPos( AudioHandle soundID, uint32 posMS );
	virtual void SetMusicVol(float vol);
	virtual void Suspend(); //stop all audio, app when into background or something
	virtual void Resume(); //restore audio that was stopped

	SoundObject * GetSoundObjectByPointer(void *p);

private:

	void mmrerror(mmr_context_t *ctxt, const char *msg);

	typedef enum
	{
		PLAYING,
		STOPPED,
		PAUSED,
	} playStatus;

	void *	   m_pDevice;

	list<SoundObject*>	m_soundList;
	float m_globalVol;
	int s_audioOid;

	playStatus s_playStatus;
	mmr_connection_t *s_mmrConnection;
	mmr_context_t *s_mmrContext;
	strm_dict_t *s_repeatDictionary;
	strm_dict_t *s_volumeDictionary;

};


#endif // AudioManagerBBX_h__

