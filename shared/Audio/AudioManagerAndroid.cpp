#include "PlatformPrecomp.h"

#include "AudioManagerAndroid.h"

#ifdef ANDROID_NDK

#include "util/MiscUtils.h"
#include "android/AndroidUtils.h"

AudioManagerAndroid::AudioManagerAndroid()
{
	m_lastMusicID = 200000000; //doesn't matter, internal usage
	m_globalVol = 1.0f;
}

AudioManagerAndroid::~AudioManagerAndroid()
{
	Kill();
}

bool AudioManagerAndroid::Init()
{
	//initted on the Android side to reduce JNI crap
	return true;
}

void AudioManagerAndroid::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
{
	LogMsg("Killing sound cache");
	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{

		if (!bKillLooping && (*itor)->m_bIsLooping) 
		{
			itor++;
			continue;
		}

		/*
		if (!bKillSoundsPlaying)
		{
			//are any channels currently using this sound?

			if ((*itor)->m_pLastChannelToUse && IsPlaying( (AudioHandle)(*itor)->m_pLastChannelToUse) )
			{
				itor++;
				continue;
			}
		}
		*/

		if (!bKillMusic && (*itor)->m_fileName == m_lastMusicFileName)
		{
			itor++;
			continue; //skip this one
		}

		delete (*itor);
		list<SoundObject*>::iterator itorTemp = itor;
		itor++;
		m_soundList.erase(itorTemp);
	}

	if (bKillMusic)
	{
		//m_pMusicChannel = NULL;
		StopMusic();
	}
}

void AudioManagerAndroid::Kill()
{
	
	StopMusic();
	KillCachedSounds(true, true, 0, 0, false);
	
	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
			delete (*itor);
			itor++;
	}
	m_soundList.clear();

	
	LogMsg("Shutting down audio system");
}

bool AudioManagerAndroid::DeleteSoundObjectByFileName(string fName)
{
	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor)->m_fileName == fName)
		{
			delete (*itor);
			m_soundList.erase(itor);
			return true; //deleted
		}
		itor++;
	}

	return false; //failed
}

SoundObject * AudioManagerAndroid::GetSoundObjectByFileName(string fName)
{
	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor)->m_fileName == fName)
		{
			return (*itor); //found a match
		}
		itor++;
	}

	return 0; //failed
}

void AudioManagerAndroid::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{

	if (bIsMusic) return;//we don't preload music that way

	string basePath;

	if (bAddBasePath)
	{
		basePath = GetBaseAppPath();
	}
	SoundObject *pObject = GetSoundObjectByFileName((GetBaseAppPath()+fName).c_str());

	if (!pObject)
	{
		//create it
		pObject = new SoundObject;
		pObject->m_fileName = fName;

		if (GetFileExtension(fName) == "mp3")
		{
			fName = ModifyFileExtension(fName, "ogg");
			StringReplace("/mp3", "/ogg", fName);
		} else
		if (GetFileExtension(fName) == "wav")
		{
			fName = ModifyFileExtension(fName, "ogg");
			if (!FileExists(fName))
			{
				//well, fooey.  change it back
				fName = ModifyFileExtension(fName, "wav");
			}
		}


		//tell android to load sound

		JNIEnv *env = GetJavaEnv();
		if (env)
		{

		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_load",
			"(Ljava/lang/String;)I");
		
		jstring mystr = env->NewStringUTF((basePath+fName).c_str());

		jint ret = env->CallStaticIntMethod(cls, mid, mystr);
		pObject->m_soundID = ret;

		//LogMsg("Got back audio handle %d", pObject->m_soundID);
		}

	
		if (!pObject->m_soundID)
		{
			LogMsg("Error loading %s ", (basePath+fName).c_str());
			delete pObject;
			return;
		}
		

//		LogMsg("Caching out %s", fName.c_str());

		pObject->m_bIsLooping = bLooping;
		m_soundList.push_back(pObject);
	}
}


AudioHandle AudioManagerAndroid::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{
	if (!GetSoundEnabled() && !bIsMusic) return AUDIO_HANDLE_BLANK;

	if (!GetMusicEnabled() && bIsMusic)
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;

		return AUDIO_HANDLE_BLANK;
	}

	if (bIsMusic && m_bLastMusicLooping == bLooping && m_lastMusicFileName == fName && m_bLastMusicLooping && IsPlaying((AudioHandle) m_lastMusicID))
	{
		return (AudioHandle) m_lastMusicID;
	}

	if (bIsMusic)
	{
		StopMusic();
	}

	if (fName.empty()) return AUDIO_HANDLE_BLANK; //can't play a blank, now can we

	int loops = 0;
	if (bLooping) loops = -1;


	if (bIsMusic)
	{
		string basePath;

		if (bAddBasePath)
		{
			basePath = GetBaseAppPath();
		}

		m_lastMusicFileName = fName;
		m_bLastMusicLooping = bLooping;
		
		if (GetFileExtension(fName) == "mp3")
		{
			fName = ModifyFileExtension(fName, "ogg");
			StringReplace("/mp3", "/ogg", fName);

			fName = ModifyFileExtension(fName, "ogg");
		} else
		if (GetFileExtension(fName) == "wav")
		{
			fName = ModifyFileExtension(fName, "ogg");
			if (!FileExists(fName))
			{
				//well, fooey.  change it back
				fName = ModifyFileExtension(fName, "wav");
			}
		}

		JNIEnv *env = GetJavaEnv();
		if (env)
		{

			jclass cls = env->FindClass(GetAndroidMainClassName());
			jmethodID mid = env->GetStaticMethodID(cls,
				"music_play",
				"(Ljava/lang/String;Z)V");
			jstring mystr = env->NewStringUTF((basePath+fName).c_str());

			env->CallStaticVoidMethod(cls, mid, mystr, jboolean(bLooping));
		}

	
		SetMusicVol(m_musicVol);
		return (AudioHandle) m_lastMusicID;

	}

	//non music
	SoundObject *pObject = GetSoundObjectByFileName(fName);

	if (!pObject)
	{
		//create it
		Preload(fName, bLooping, bIsMusic, bAddBasePath, bForceStreaming);
		pObject = GetSoundObjectByFileName(fName);
		if (!pObject)
		{
			LogError("Unable to cache sound %s", fName.c_str());
			return (AudioHandle) 0 ;
		}
	}

	JNIEnv *env = GetJavaEnv();
	if (env)
	{

		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_play",
			"(IFFIIF)I");

		int streamID = env->CallStaticIntMethod(cls, mid, pObject->m_soundID, jfloat(m_globalVol*m_defaultVol), jfloat(m_globalVol*m_defaultVol), jint(0), jint(loops), jfloat(1.0f));
		if (streamID == 0)
		{
			//something is wrong... probably not loaded.  Reschedule
			GetMessageManager()->SendGame(MESSAGE_TYPE_PLAY_SOUND, fName, 50, TIMER_SYSTEM);
			return (AudioHandle) 0;
		}

		return (AudioHandle) streamID;
	}

	return (AudioHandle) 0 ;
}

void AudioManagerAndroid::Update()
{

}

void AudioManagerAndroid::Stop( AudioHandle soundID )
{

	//LogMsg("About to call soundstop with %d", soundID);

	if (!soundID) return;

	if (soundID == m_lastMusicID)
	{
		StopMusic();
		return;
	}
	//notify engine to kill it, although it likely already did
	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_stop",
			"(I)V");

		env->CallStaticVoidMethod(cls, mid, jint(soundID));
	}

}

AudioHandle AudioManagerAndroid::GetMusicChannel()
{
	return m_lastMusicID; //(AudioHandle)m_pMusicChannel;

}

bool AudioManagerAndroid::IsPlaying( AudioHandle soundID )
{
	if (soundID == AUDIO_HANDLE_BLANK) return false;

	if (soundID == (AudioHandle) m_lastMusicID)
	{
		JNIEnv *env = GetJavaEnv();
		if (env)
		{
			jclass cls = env->FindClass(GetAndroidMainClassName());
			jmethodID mid = env->GetStaticMethodID(cls,
				"music_is_playing",
				"()Z");
			return env->CallStaticBooleanMethod(cls, mid);
		}
		
	}
	
	return false;
	//return Mix_Playing(soundID) != 0;
}


void AudioManagerAndroid::SetMusicEnabled( bool bNew )
{
	if (bNew != m_bMusicEnabled)
	{
		AudioManager::SetMusicEnabled(bNew);
		if (bNew)
		{
			if (!m_lastMusicFileName.empty())
			{
				Play(m_lastMusicFileName, GetLastMusicLooping(), true);

			}
		} else
		{
			//kill the music

			StopMusic();
		}
	}

}

void AudioManagerAndroid::StopMusic()
{
	//LogMsg("Stop music");
	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
		"music_stop",
		"()V");
		env->CallStaticVoidMethod(cls, mid);
	}
	
}

void AudioManagerAndroid::FadeOutMusic(unsigned int duration)
{
	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls, "music_fadeout", "(I)V");
		env->CallStaticVoidMethod(cls, mid, jint(duration));
	}
}

int AudioManagerAndroid::GetMemoryUsed()
{

	return 0;
}

void AudioManagerAndroid::SetFrequency( AudioHandle soundID, int freq )
{
	
	assert(soundID);
	//Android::Channel *pChannel = (Android::Channel*) soundID;
	
	float rate = float(freq)/float(22050);

	//force value to be within range
	rate = rt_min(2.0, rate);
	rate = rt_max(0.5, rate);

#ifdef _DEBUG
	//LogMsg("Setting freq to %0.2f", rate);
#endif

	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_set_rate",
			"(IF)V");
		env->CallStaticVoidMethod(cls, mid, jint(soundID), jfloat(rate));
	}

}

void AudioManagerAndroid::SetPan( AudioHandle soundID, float pan )
{
	if (soundID == 0) return;
	
	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_set_vol",
			"(IFF)V");
		
		float volLeft = rt_max(1, 1- pan);
		float volRight = rt_max(1, 1- (-pan));

		//LogMsg("%.2f converted to l %.2f, r %.2f");
		env->CallStaticVoidMethod(cls, mid, jint(soundID), jfloat(volLeft*m_globalVol), jfloat(volRight*m_globalVol));
	}
}


void AudioManagerAndroid::SetPos( AudioHandle soundID, uint32 posMS )
{
	assert(soundID);
	if (soundID == m_lastMusicID) 
	{
			JNIEnv *env = GetJavaEnv();
			if (env)
			{
				jclass cls = env->FindClass(GetAndroidMainClassName());
				jmethodID mid = env->GetStaticMethodID(cls,
					"music_set_pos",
					"(I)V");
				env->CallStaticVoidMethod(cls, mid, jint(int(posMS)));
			}
	
		return;
	}
	LogMsg("SetPosition is unsupported for sounds");
}

uint32 AudioManagerAndroid::GetPos( AudioHandle soundID)
{
	if (soundID == m_lastMusicID) 
	{
		JNIEnv *env = GetJavaEnv();
		if (env)
		{
			jclass cls = env->FindClass(GetAndroidMainClassName());
			jmethodID mid = env->GetStaticMethodID(cls,
				"music_get_pos",
				"()I");
			return env->CallStaticIntMethod(cls, mid);
		}
		return 0;
	}
	LogMsg("GetPosition is unsupported for sounds");
	return 0;
}

void AudioManagerAndroid::SetVol( AudioHandle soundID, float vol )
{
	if (soundID == m_lastMusicID) 
	{
		SetMusicVol(vol);
		return;
	}
	
	if (soundID == -1)
	{
		m_globalVol = vol;
		//LogMsg("AudioManagerAndroid: Set globalvol to %.2f", m_globalVol);
		SetMusicVol(m_musicVol);
		return;
	}
	if (soundID == 0) return;

#ifdef _DEBUG
	//LogMsg("Setting vol to %.2f", vol);
#endif


	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"sound_set_vol",
			"(IFF)V");
		env->CallStaticVoidMethod(cls, mid, jint(soundID), jfloat(vol*m_globalVol), jfloat(vol*m_globalVol));
	}

}

void AudioManagerAndroid::SetMusicVol(float vol )
{
	
	m_musicVol = vol;
	JNIEnv *env = GetJavaEnv();
	if (env)
	{
		jclass cls = env->FindClass(GetAndroidMainClassName());
		jmethodID mid = env->GetStaticMethodID(cls,
			"music_set_volume",
			"(F)V");
		env->CallStaticVoidMethod(cls, mid, jfloat(vol*m_globalVol));
	}
	
}



void AudioManagerAndroid::SetPriority( AudioHandle soundID, int priority )
{

}

void AudioManagerAndroid::Vibrate(int duration)
{
	if (!m_bVibrationDisabled)
	{
		JNIEnv *env = GetJavaEnv();
		if (env)
		{
			jclass cls = env->FindClass(GetAndroidMainClassName());
			jmethodID mid = env->GetStaticMethodID(cls,
				"vibrate",
				"(I)V");
			env->CallStaticVoidMethod(cls, mid, jint(duration));
		}
	}
}


#endif