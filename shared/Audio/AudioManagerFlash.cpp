#include "PlatformPrecomp.h"
 
#if defined(RT_FLASH_TEST) || !defined(WIN32)

//#define RT_DEBUG_AUDIO

#ifndef RT_FLASH_TEST
#include <AS3/AS3.h>
#endif

#include "AudioManagerFlash.h"
#include "util/MiscUtils.h"


void SoundObjectFlash::DeleteSample()
{
	//this is a sample id, not an audio handle instance.  Sort of confusing, but 1 sample is loaded and shared, and an audio id is
	//for each sfx played

	if (m_sampleID != AUDIO_HANDLE_BLANK)
	{
		inline_as3(
			"import com.rtsoft.AudioManager;"
			"AudioManager.current.DeleteAudio(%0);"
			:  : "r"(m_sampleID) );

	}

}

AudioManagerFlash::AudioManagerFlash()
{
	m_globalVol = 1.0f;

}

AudioManagerFlash::~AudioManagerFlash()
{
	Kill();

}

bool AudioManagerFlash::Init()
{
	LogMsg("Initting AudioManagerFlash");
	return true;
}

void AudioManagerFlash::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
{
	LogMsg("Killing sound cache");
	list<SoundObjectFlash*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{

		if (!bKillLooping && (*itor)->m_bIsLooping) 
		{
			itor++;
			continue;
		}
		
		if (!bKillSoundsPlaying)
		{
			//are any channels currently using this sound?
			/*
			if ( IsPlaying((*itor)->source))
			{
				itor++;
				continue;
			}
			*/
		}
		
		if (!bKillMusic && (*itor)->m_fileName == m_lastMusicFileName)
		{
			itor++;
			continue; //skip this one
		}
		
		delete (*itor);
		list<SoundObjectFlash*>::iterator itorTemp = itor;
		itor++;
		m_soundList.erase(itorTemp);
	}
	
}

void AudioManagerFlash::Kill()
{
	
}

bool AudioManagerFlash::DeleteSoundObjectByFileName(string fName)
{
	list<SoundObjectFlash*>::iterator itor = m_soundList.begin();

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

	return false; 
}

SoundObjectFlash * AudioManagerFlash::GetSoundObjectByFileName(string fName)
{
	list<SoundObjectFlash*>::iterator itor = m_soundList.begin();

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

SoundObjectFlash * AudioManagerFlash::GetSoundObjectByPointer(void *p)
{
	list<SoundObjectFlash*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor) == p)
		{
			return (*itor); //found a match
		}
		itor++;
	}

	return 0; //failed
}

void AudioManagerFlash::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{
	if (bIsMusic && !GetMusicEnabled()) return; //ignoring because music is off right now

	string basePath;

	StringReplace(".wav", ".mp3", fName);
	StringReplace(".ogg", ".mp3", fName);

	LogMsg("Preloading %s", fName.c_str());
	if (bAddBasePath)
	{
		basePath = GetBaseAppPath();
	}
	SoundObjectFlash *pObject = GetSoundObjectByFileName((basePath+fName).c_str());

	if (!pObject)
	{
		//create it
		pObject = new SoundObjectFlash;
		pObject->m_fileName = fName;
		pObject->m_bIsLooping = bLooping;
		pObject->m_bIsMusic   = bIsMusic;

	//	LogMsg("Created audio ID %d", pObject->m_sampleID);

		//Current flash target only handles mp3 files
		int fileSize;
	
		byte *pData = GetFileManager()->Get(fName, &fileSize);

		if (!pData)
		{
			LogMsg("Can't instantiate %s if the file is missing!", fName.c_str());
			SAFE_DELETE(pObject);
			return;
		} else
		{
#ifdef RT_DEBUG_AUDIO
			LogMsg("Audio data %s cached, %d bytes", fName.c_str(), fileSize);
#endif
		}


		inline_as3(
			"import com.rtsoft.AudioManager;"
			"%0 = AudioManager.current.GenAudio();"
			:"=r"(pObject->m_sampleID) : );


		int loadReturn = 1;

		//the 0 I sent as the last parm below means ".mp3", which is the only format currently supported
		//on the flash side

		inline_as3(
			"import com.rtsoft.AudioManager;"
			"%0 = AudioManager.current.LoadAudioWav(%1, %2, %3, 0);"
			: "=r"(loadReturn): "r"(pObject->m_sampleID), "r"(pData), "r" (fileSize) );

		SAFE_DELETE_ARRAY(pData);

		if (loadReturn)
		{
			m_soundList.push_back(pObject);
		} else
		{
			LogMsg("AudioManagerFlash: Couldn't load %s", (basePath+fName).c_str());
			SAFE_DELETE(pObject);
		}
		
	}	
}

AudioHandle AudioManagerFlash::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{
#ifdef RT_DEBUG_AUDIO
	LogMsg("Gonna play %s", fName.c_str());
#endif

	if (!GetSoundEnabled()) return 0;

	if( !GetMusicEnabled() && bIsMusic )
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;
		LogMsg("Music disabled, not playing");
		return AUDIO_HANDLE_BLANK;
	}

	
	StringReplace(".wav", ".mp3", fName);
	StringReplace(".ogg", ".mp3", fName);

	if (bIsMusic && m_lastMusicFileName == fName && m_bLastMusicLooping && m_lastMusicID != AUDIO_HANDLE_BLANK)
	{
		LogMsg("Already playing this song, ignoring command");
		return (AudioHandle) m_lastMusicID;
	}
	int loopCount = 0;

	if(bIsMusic)
	{
		StopMusic();
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;
	}

	SoundObjectFlash *pObject = GetSoundObjectByFileName(fName);

	if (!pObject)
	{
		Preload(fName, bLooping, bIsMusic, bAddBasePath, bForceStreaming);
		pObject = GetSoundObjectByFileName(fName);
		if (!pObject)
		{
			LogError("Unable to cache sound %s", fName.c_str());
			return false;
		}
	}

	if (bLooping)
	{
		pObject->m_bIsLooping = bLooping;
#ifdef RT_DEBUG_AUDIO
		LogMsg("Playing looping sound: %s", fName.c_str());
#endif
		loopCount = 200000000;
	}

	int soundInstanceID = 0;

#ifdef RT_DEBUG_AUDIO
	LogMsg("Sending flash command to play %s", fName.c_str());
#endif

	inline_as3(
		"import com.rtsoft.AudioManager;"
		"%0 = AudioManager.current.PlayAudio(%1, %2, %3);"
		: "=r"(soundInstanceID ) : "r"(pObject->m_sampleID), "r"(0), "r" (loopCount) );

	if (bIsMusic)
	{
		//LogMsg("(Set as music)");
		SetMusicVol(m_musicVol);
		m_lastMusicID = soundInstanceID;
	}


	//LogMsg("Played, channel is %d",soundInstanceID);
	return (AudioHandle)soundInstanceID;
}


void AudioManagerFlash::Update()
{
	//no need to update
}

void AudioManagerFlash::Stop( AudioHandle soundID )
{

	//LogMsg("Stopping soundID %d", soundID);
//	SoundObjectFlash *pObject = GetSoundObjectByPointer((void*)soundID);

	inline_as3(
		"import com.rtsoft.AudioManager;"
		"AudioManager.current.DeleteSoundInstance(%0);"
		:  : "r"(soundID) );

	if (soundID == m_lastMusicID)
	{
		//LogMsg("Stoppnig music");
		m_lastMusicID = AUDIO_HANDLE_BLANK;
	}

}

AudioHandle AudioManagerFlash::GetMusicChannel()
{
	//skipped
	return (AudioHandle)m_lastMusicID;
}

bool AudioManagerFlash::IsPlaying( AudioHandle soundID )
{

	if ( m_lastMusicID != AUDIO_HANDLE_BLANK && soundID == m_lastMusicID)
	{
		if (m_bLastMusicLooping && m_bMusicEnabled)
		{
			return true; //well, it must be playing, it's looping, right?
		}
		//this is wrong, we don't know if the music is playing.. fix me?
		return false;
	}

	SoundObjectFlash *pObject = GetSoundObjectByPointer((void*)soundID);

		return false;
}

void AudioManagerFlash::SetMusicEnabled( bool bNew )
{
	if (bNew != m_bMusicEnabled)
	{
		AudioManager::SetMusicEnabled(bNew);
	
		if (bNew)
		{
			if (!m_lastMusicFileName.empty())
			{
				Play(m_lastMusicFileName, m_bLastMusicLooping, true);
			}
		}else
		{
			StopMusic();
		}
	}
}

void AudioManagerFlash::StopMusic()
{
	Stop(m_lastMusicID);
}

int AudioManagerFlash::GetMemoryUsed()
{
	//no clue how to get Flash memory usage :)
	return 0;
}

void AudioManagerFlash::SetFrequency( AudioHandle soundID, int freq )
{
	//skipped
}

void AudioManagerFlash::SetPan( AudioHandle soundID, float pan )
{
	//skipped
}

uint32 AudioManagerFlash::GetPos( AudioHandle soundID )
{
	return (AudioHandle)0;
}

void AudioManagerFlash::SetPos( AudioHandle soundID, uint32 posMS )
{
}

void AudioManagerFlash::SetVol( AudioHandle soundID, float vol )
{
	if (soundID == -1)
	{
		m_globalVol = vol;
		SetMusicVol(m_musicVol);
		return;
	}

	SoundObjectFlash *pObject = GetSoundObjectByPointer((void*)soundID);
}

void AudioManagerFlash::SetMusicVol(float vol )
{

	m_musicVol = vol;
	char volume_str[128];

	sprintf(volume_str, "%d", (int)(vol * m_globalVol * 100) );
	LogMsg("Setting music vol to %s", volume_str);
	
}


void AudioManagerFlash::SetPriority( AudioHandle soundID, int priority )
{

}

void AudioManagerFlash::Suspend()
{
	

	//stop all looping sfx

	list<SoundObjectFlash*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		//if ( (*itor)->source) alSourceStop((*itor)->source);
		
		itor++;
	}

}


void AudioManagerFlash::Resume()
{
	
}

void AudioManagerFlash::Testy()
{
  LogMsg("TOOOOASTTTTY!!!!");
}
#endif