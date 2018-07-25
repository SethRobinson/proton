#include "PlatformPrecomp.h"

#ifdef PLATFORM_BBX
#include "AudioManagerBBX.h"
#include "util/MiscUtils.h"
#define MUSIC_HANDLE 2000000000


AudioManagerBBX::AudioManagerBBX()
{
	alutInit(0, 0);
	m_pDevice = NULL;
	m_globalVol = 1.0f;

	s_playStatus = STOPPED;
	s_mmrConnection = 0;
	s_mmrContext = 0;
	s_repeatDictionary = 0;
	s_volumeDictionary = 0;
	s_audioOid = 0;
}

AudioManagerBBX::~AudioManagerBBX()
{
	Kill();
	alutExit();


		if (s_mmrConnection)
		mmr_disconnect(s_mmrConnection);

		if (s_repeatDictionary)
		strm_dict_destroy(s_repeatDictionary);

		if (s_volumeDictionary)
		strm_dict_destroy(s_volumeDictionary);

		s_mmrConnection = 0;
		s_repeatDictionary = 0;
		s_volumeDictionary = 0;

}

bool AudioManagerBBX::Init()
{
	LogMsg("initting AudioManagerBBX");
	return true;
}

void AudioManagerBBX::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
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
		
		if (!bKillSoundsPlaying)
		{
			//are any channels currently using this sound?
			
			if ( IsPlaying((*itor)->source))
			{
				itor++;
				continue;
			}
		}
		
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
	
}

void AudioManagerBBX::Kill()
{
	if (m_pDevice)
	{
		KillCachedSounds(true, true, 0, 100, true);
		m_pDevice = NULL;
	}
}

bool AudioManagerBBX::DeleteSoundObjectByFileName(string fName)
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

	return false; 
}

SoundObject * AudioManagerBBX::GetSoundObjectByFileName(string fName)
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

SoundObject * AudioManagerBBX::GetSoundObjectByPointer(void *p)
{
	list<SoundObject*>::iterator itor = m_soundList.begin();

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

void AudioManagerBBX::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{

	if (bIsMusic) return; //can't preload music on BBX, right?

	if (bIsMusic && !GetMusicEnabled()) return; //ignoring because music is off right now

	if (bIsMusic) 
	{
		m_lastMusicFileName = fName;
		bForceStreaming = true;
	}

	string basePath;

	if (bAddBasePath)
	{
		basePath = GetBaseAppPath();
	}
	SoundObject *pObject = GetSoundObjectByFileName((basePath+fName).c_str());

	if (!pObject)
	{
		//create it
		pObject = new SoundObject;
		pObject->m_fileName = fName;

		//load with openal
		pObject->buffer = alutCreateBufferFromFile((basePath+fName).c_str());

		if (pObject->buffer == AL_NONE)
		{
			LogMsg("Error loading audio file: '%s'", (basePath+fName).c_str());
			alDeleteBuffers(1, &pObject->buffer);
			pObject->buffer = 0;
			SAFE_DELETE(pObject);
			return;
		}

		alGenSources(1, &pObject->source);
		alSourcei(pObject->source, AL_BUFFER, pObject->buffer);
																		    
		pObject->m_bIsLooping = bLooping;
		pObject->m_bIsMusic   = bIsMusic;

		m_soundList.push_back(pObject);
	}	
}

void AudioManagerBBX::mmrerror(mmr_context_t *ctxt, const char *msg)
    {
     const mmr_error_info_t *err = mmr_error_info( ctxt );
     unsigned errcode = (err) ? err->error_code : -1;
     const char *name;

     LogMsg("%s: error %d", msg, errcode);
    }

AudioHandle AudioManagerBBX::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{

	if( !GetMusicEnabled() && bIsMusic )
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;
		return 0;
	}

	if (bIsMusic && m_lastMusicFileName == fName && m_bLastMusicLooping && m_lastMusicID == MUSIC_HANDLE)
	{
		return (AudioHandle) m_lastMusicID;
	}

	if(bIsMusic)
	{
		StopMusic();
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;

			const char *mmrname = NULL;
		     const char *ctxtname = "mmrplayer";

		     mode_t mode = S_IRUSR | S_IXUSR;


		     string basePath;

		     	if (bAddBasePath)
		     	{
		     		basePath = GetBaseAppPath();
		     	}

		//music is handled differently


		//setup music context and preload it (?)
		   if (s_mmrConnection == NULL)
		   {
		     	s_mmrConnection = mmr_connect(mmrname);
		     if (!s_mmrConnection)
		     {
				perror("mmr_connect when starting - ");

				 return  AUDIO_HANDLE_BLANK;
		     }
		   }

		   	 s_mmrContext = mmr_context_create(s_mmrConnection, ctxtname, 0, mode);

		     	 if (!s_mmrContext)
		          {
		          LogError(ctxtname);
		          return  AUDIO_HANDLE_BLANK;
		          }

		          if ((s_audioOid = mmr_output_attach(s_mmrContext, "audio:default", "audio")) < 0)
		          {
		          mmrerror(s_mmrContext, "audio:default");
		          return  AUDIO_HANDLE_BLANK;
		          }

		         // setBackgroundVolume(s_volume);
		      	char cwd[PATH_MAX];
		      	getcwd(cwd, PATH_MAX);

		      	string finalName = "file://"+string(cwd)+"/"+(basePath+fName);

		          if (mmr_input_attach(s_mmrContext, finalName.c_str(), "autolist") < 0)
		          {
		          LogMsg("unable to load %s",finalName.c_str());
		          mmrerror(s_mmrContext,finalName.c_str());
		          return  AUDIO_HANDLE_BLANK;
		       }


		//*** play the music

		if (bLooping)
		{
		// set it up to loop
		strm_dict_t *dictionary = strm_dict_new();
		s_repeatDictionary = strm_dict_set(dictionary, "repeat", "all");

		//LogMsg("Setting up to loop");
		     if (mmr_input_parameters(s_mmrContext, s_repeatDictionary) != 0)
		     {
				 mmrerror(s_mmrContext, "input parameters (loop)");
		     }
		}

		if (mmr_play(s_mmrContext) < 0)
		{
			mmrerror(s_mmrContext, "mmr_play");
		}

		SetMusicVol(m_musicVol);
		m_lastMusicID = MUSIC_HANDLE;
		return MUSIC_HANDLE;
	}


	SoundObject *pObject = GetSoundObjectByFileName(fName);

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
	}


	
	if (pObject->m_bIsLooping)
	{
		LogMsg("Playing looping sound: %s", fName.c_str());
	}
	alSourcei(pObject->source, AL_LOOPING, pObject->m_bIsLooping ? AL_TRUE : AL_FALSE);
	if (GetSoundEnabled()) alSourcePlay(pObject->source);
	alSourcef(pObject->source, AL_GAIN,pObject->m_volume* m_globalVol); //default volume
	//LogMsg("Playing sound at %.2f on ID %d", m_globalVol, (AudioHandle)pObject);
	return (AudioHandle)pObject;
}


void AudioManagerBBX::Update()
{
	//no need to update
}

void AudioManagerBBX::Stop( AudioHandle soundID )
{

	if (soundID == MUSIC_HANDLE)
	{
		StopMusic();
		return;
	}

	SoundObject *pObject = GetSoundObjectByPointer((void*)soundID);

	if (pObject && pObject->source)
	{
		alSourceStop(pObject->source);
	}
	
}

AudioHandle AudioManagerBBX::GetMusicChannel()
{
	//skipped
	return (AudioHandle)0;
}

bool AudioManagerBBX::IsPlaying( AudioHandle soundID )
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

	SoundObject *pObject = GetSoundObjectByPointer((void*)soundID);

		if (pObject && pObject->source)
		{
			ALint val;
			alGetSourcei(pObject->source, AL_SOURCE_STATE, &val);
            if(val == AL_PLAYING) return true;
		}

		return false;
}

void AudioManagerBBX::SetMusicEnabled( bool bNew )
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

void AudioManagerBBX::StopMusic()
{
	bool bReleaseData = true;
	if (s_mmrContext)
	{
		LogMsg("Trying to stop music..");
		int er = mmr_stop(s_mmrContext);
		if (er != 0)
		{
			 mmrerror(s_mmrContext, "mmr_stop");
			LogMsg("mm_r_stop failed");

		}
	}

	if (bReleaseData)
	{

		if (s_mmrContext)
			{
				mmr_input_detach(s_mmrContext);
				mmr_context_destroy(s_mmrContext);
				s_mmrContext = 0;

			}
	}

	m_lastMusicID = AUDIO_HANDLE_BLANK;
}

int AudioManagerBBX::GetMemoryUsed()
{
	//no clue how to get BBX memory usage :)
	return 0;
}

void AudioManagerBBX::SetFrequency( AudioHandle soundID, int freq )
{
	//skipped
}

void AudioManagerBBX::SetPan( AudioHandle soundID, float pan )
{
	//skipped
}

uint32 AudioManagerBBX::GetPos( AudioHandle soundID )
{
	return (AudioHandle)0;
}

void AudioManagerBBX::SetPos( AudioHandle soundID, uint32 posMS )
{
}

void AudioManagerBBX::SetVol( AudioHandle soundID, float vol )
{
	if (soundID == -1)
	{
		m_globalVol = vol;
		SetMusicVol(m_musicVol);
		return;
	}

	SoundObject *pObject = GetSoundObjectByPointer((void*)soundID);

	if (pObject && pObject->source)
	{
		pObject->m_volume = vol;
		//LogMsg("Setting vol of id %d to %.2f", soundID, pObject->m_volume*m_globalVol);
		alSourcef(pObject->source, AL_GAIN, pObject->m_volume*m_globalVol);
	}

}

void AudioManagerBBX::SetMusicVol(float vol )
{

	m_musicVol = vol;

	char volume_str[128];

	// set it up the background volume
	strm_dict_t *dictionary = strm_dict_new();

	sprintf(volume_str, "%d", (int)(vol * m_globalVol * 100) );
	LogMsg("Setting music vol to %s", volume_str);

	s_volumeDictionary = strm_dict_set(dictionary, "volume", volume_str);

	if (s_mmrContext)
	{
		if (mmr_output_parameters(s_mmrContext, s_audioOid, s_volumeDictionary) != 0)
		{
		mmrerror(s_mmrContext, "output parameters");
		return;
		}
	}
	
}


void AudioManagerBBX::SetPriority( AudioHandle soundID, int priority )
{

}

void AudioManagerBBX::Suspend()
{
	if (s_mmrContext && mmr_speed_set(s_mmrContext, 0) < 0)
	{
		mmrerror(s_mmrContext, "pause");
	}

	//stop all looping sfx

	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor)->source) alSourceStop((*itor)->source);
		
		itor++;
	}

}


void AudioManagerBBX::Resume()
{
	if (s_mmrContext && mmr_speed_set(s_mmrContext, 1000) < 0)
	{
		mmrerror(s_mmrContext, "resume");
	}
}

#endif