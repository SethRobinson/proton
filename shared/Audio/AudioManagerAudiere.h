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

Releasing a sound (Sep 2026, found through RTGameBot's "R6025 pure virtual
function call" box, its docs/deploy.md "The R6025 box" has the dumps and
the arithmetic): audiere's DirectSound device polls every open buffer's
stop notification on a thread of its own, every 50 ms under its device
lock, and fires a stop event that takes a NEW reference on the buffer,
which its event thread releases a moment later. A buffer whose last
reference goes while that notification is signaled and not yet consumed
(within a poll period of its end or of its stop) can be resurrected by
the poll while it is being destroyed, and is then destroyed a second time
under the event thread: a pure virtual call on the corpse (the box) or a
double free (a heap corruption report that goes straight to WER). So a
sound object the app deletes is only RETIRED here: it is stopped and kept
in m_retired with its stream, and the stream is released from Update once
C_AUDIERE_RELEASE_DELAY_MS have passed, by which time the poll has
consumed the notification and the event thread has let go. Kill stops
everything, waits C_AUDIERE_KILL_SETTLE_MS for the same reason, then frees
it all on the calling thread before the device goes (the device's
destructor waits for audiere's threads, so nothing may still be dying on
them). RTGameBot's -audiostresstest hammers this path.
*/

#ifndef AudioManagerAudiere_h__
#define AudioManagerAudiere_h__

#include "AudioManager.h"

#if !defined RT_WEBOS && !defined (ANDROID_NDK)


#include "audiere/include/audiere.h"

using namespace audiere;

const int C_AUDIERE_RELEASE_DELAY_MS = 1000; //a retired sound's stream is released this long after its stop (20 poll periods, see above)
const int C_AUDIERE_KILL_SETTLE_MS = 250;    //Kill's wait between stopping everything and freeing it

class AudiereSoundObject
{
public:

	AudiereSoundObject()
	{
		m_pSound		= NULL;
		m_bIsLooping	= false;
		m_bIsMusic		= false;
		m_retiredTick	= 0;
	}

	~AudiereSoundObject()
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
	unsigned int m_retiredTick; //GetSystemTimeTick() when the app deleted it (0 = live); its stream is released C_AUDIERE_RELEASE_DELAY_MS later
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

	AudiereSoundObject * GetSoundObjectByFileName(std::string fName);
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
	AudiereSoundObject * GetSoundObjectByPointer(void *p);

private:

	void Retire(AudiereSoundObject *pObject); //stop it and move it to m_retired; SweepRetired releases its stream later (the header comment)
	void SweepRetired(bool bForce);           //release the retired streams past the delay (bForce: every one, from Kill)

	AudioDevicePtr	   m_pDevice;

	std::list<AudiereSoundObject*>	m_soundList;
	std::list<AudiereSoundObject*>	m_retired; //deleted by the app, their streams still held for the delay: invisible to the lookups
	float m_globalVol;

protected:


private:
};


#endif // AudioManagerAudiere_h__
#endif