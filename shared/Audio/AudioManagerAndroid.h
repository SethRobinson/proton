//  ***************************************************************
//  AudioManagerAndroid - Creation date: 09/28/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AudioManagerAndroid_h__
#define AudioManagerAndroid_h__

#ifdef ANDROID_NDK
#include "AudioManager.h"

class SoundObject
{
public:

	SoundObject()
	{
		m_soundID = 0;
		m_bIsLooping = false;
	}

	~SoundObject()
	{
		if (m_soundID)
		{
	
			//notify engine to kill it, although it likely already did
			JNIEnv *env = GetJavaEnv();
			if (env)
			{
				jclass cls = env->FindClass(GetAndroidMainClassName());
				jmethodID mid = env->GetStaticMethodID(cls,
					"sound_kill",
					"(I)V");

				env->CallStaticVoidMethod(cls, mid, jint(m_soundID));
			}


			 m_soundID = 0;
			 m_lastStreamIDToUse = 0;
		}
	}

	int m_soundID;
	string m_fileName;
	bool m_bIsLooping;
	int m_lastStreamIDToUse;
};

class AudioManagerAndroid: public AudioManager
{
public:
	AudioManagerAndroid();
	virtual ~AudioManagerAndroid();

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
	virtual void FadeOutMusic(unsigned int duration = 1000);
	virtual int GetMemoryUsed();
	bool DeleteSoundObjectByFileName(string fName);
	virtual void SetFrequency(AudioHandle soundID, int freq);
	virtual void SetPan(AudioHandle soundID, float pan); //0 is normal stereo, -1 is all left, +1 is all right
	virtual void SetVol(AudioHandle soundID, float vol);
	virtual void SetPriority(AudioHandle soundID, int priority);
	virtual uint32 GetPos( AudioHandle soundID );
	virtual void SetPos( AudioHandle soundID, uint32 posMS );
	virtual void SetMusicVol(float vol);
	virtual void Vibrate(int duration = 300);
private:

	list<SoundObject*> m_soundList;

protected:

float m_globalVol;
private:
};

#endif
#endif