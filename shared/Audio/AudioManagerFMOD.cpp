#include "PlatformPrecomp.h"

#ifndef RT_WEBOS

#include "AudioManagerFMOD.h"
#include "util/MiscUtils.h"

void FMOD_ERROR_CHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		LogMsg("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		//exit(-1);
	}
}

AudioManagerFMOD::AudioManagerFMOD()
{
	system = NULL;
	m_pMusicChannel = NULL;
}

AudioManagerFMOD::~AudioManagerFMOD()
{
	Kill();
}


bool AudioManagerFMOD::Init()
{
	FMOD_RESULT       result;
	unsigned int      version;
	int              numdrivers;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS        caps;
	char             name[256];


	result = FMOD::System_Create(&system);
	FMOD_ERROR_CHECK(result);
	result = system->getVersion(&version);
	FMOD_ERROR_CHECK(result);

	if (version < FMOD_VERSION)
	{
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		assert(0);
		return 0;
	}
	result = system->getNumDrivers(&numdrivers);
	FMOD_ERROR_CHECK(result);

	if (numdrivers == 0)
	{
		result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		FMOD_ERROR_CHECK(result);
	}
	else
	{

		
        //if you get an error about incorrect number of args, go download the latest fmod version! -Seth
        result = system->getDriverCaps(0, &caps, 0, &speakermode);

		FMOD_ERROR_CHECK(result);

		result = system->setSpeakerMode(speakermode);       /* Set the user selected speaker mode. */
		FMOD_ERROR_CHECK(result);

		if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
		{                                                   /* You might want to warn the user about this. */
			result = system->setDSPBufferSize(1024, 10);
			FMOD_ERROR_CHECK(result);
		}

		result = system->getDriverInfo(0, name, 256, 0);
		FMOD_ERROR_CHECK(result);

		if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
		{
			result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
			FMOD_ERROR_CHECK(result);
		}
        
#ifdef RT_IOS_60BEAT_GAMEPAD_SUPPORT
        //to work with the 60beat gampad, we must be at 44k
        result = system->setSoftwareFormat(44100, FMOD_SOUND_FORMAT_PCM16, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
        FMOD_ERROR_CHECK(result);
#endif

	}

	result = system->init(32, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
	{
		result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		FMOD_ERROR_CHECK(result);

		result = system->init(32, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
		FMOD_ERROR_CHECK(result);
	}

	return true;
}

void AudioManagerFMOD::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
{
	LogMsg("Killing sound cache");
	list<SoundObject*>::iterator itor = m_soundList.begin();

	//assert(ignoreSoundsUsedInLastMS == 0 && "We don't actually support this yet");

	//StopMusic();

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
			
			if ((*itor)->m_pLastChannelToUse && IsPlaying( (AudioHandle)(*itor)->m_pLastChannelToUse) )
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

	if (bKillMusic)
	{
		StopMusic();
	}
}

void AudioManagerFMOD::Kill()
{
	if (system)
	{
		KillCachedSounds(true, true, 0, 100, true);
		FMOD_RESULT  result;
		result = system->close();
		FMOD_ERROR_CHECK(result);
		result = system->release();
		FMOD_ERROR_CHECK(result);
		system = NULL;
	}
}

bool AudioManagerFMOD::DeleteSoundObjectByFileName(string fName)
{
	fName = ModifiedFileName(fName);

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

SoundObject * AudioManagerFMOD::GetSoundObjectByFileName(string fName)
{
	fName = ModifiedFileName(fName);

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

void AudioManagerFMOD::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{

	if (!system) return;

	if (bIsMusic && !GetMusicEnabled()) return; //ignoring because music is off right now

	fName = ModifiedFileName(fName);
	
	if (bIsMusic) 
	{
	
		m_lastMusicFileName = fName;
		
		if (m_bStreamMusic)
			bForceStreaming = true;
	}

	FMOD_RESULT  result;
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

#ifdef _DEBUG
		LogMsg("Caching out %s", fName.c_str());
#endif
		int fModParms = FMOD_SOFTWARE|FMOD_LOWMEM|FMOD_LOOP_NORMAL; //tell it looking anyway, avoid a click later
		if (bLooping)
		{
			fModParms = FMOD_SOFTWARE|FMOD_LOOP_NORMAL|FMOD_LOWMEM;
		}

			if (ToLowerCaseString(GetFileExtension(fName)) == "mid")
			{
				//special handling for midi files
				FMOD_CREATESOUNDEXINFO ex;
				memset(&ex, 0, sizeof(FMOD_CREATESOUNDEXINFO));
				ex.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
				string midiSoundBank = GetBaseAppPath()+m_midiSoundBankFile;
				if (!m_midiSoundBankFile.empty())
				{
					ex.dlsname = midiSoundBank.c_str();
				}
				ex.suggestedsoundtype = FMOD_SOUND_TYPE_MIDI;
				LogMsg("Loading %s", midiSoundBank.c_str());
				result = system->createSound( (basePath+fName).c_str(), fModParms, &ex, &pObject->m_pSound);
						
			} else
			{
				
				if (bForceStreaming)
				{
					result = system->createStream( (basePath+fName).c_str(), fModParms, 0, &pObject->m_pSound);
				} else
				{
					result = system->createSound( (basePath+fName).c_str(), fModParms, 0, &pObject->m_pSound);
				}
			}

			pObject->m_bIsLooping = bLooping;

		FMOD_ERROR_CHECK(result);
		m_soundList.push_back(pObject);
	}
}


AudioHandle AudioManagerFMOD::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{
	if (!GetSoundEnabled() || !system) return 0;

	if ( !GetMusicEnabled() && bIsMusic )
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;

		return 0;
	}

	fName = ModifiedFileName(fName);

	if (bIsMusic && m_bLastMusicLooping == bLooping && m_lastMusicFileName == fName && m_bLastMusicLooping && m_pMusicChannel)
	{
		return (AudioHandle) m_pMusicChannel;
	}

	if (bIsMusic)
	{
		StopMusic();
	}
	FMOD_RESULT  result;

	SoundObject *pObject = GetSoundObjectByFileName(fName);

	if (!pObject)
	{
		//create it
		Preload(fName, bLooping, bIsMusic, bAddBasePath, bForceStreaming);
		pObject = GetSoundObjectByFileName(fName);
		if (!pObject)
		{
			LogError("Unable to cache sound %s", fName.c_str());
			return false;
			
		}
	}

	//play it
	FMOD::Channel    *pChannel = NULL;

	if (bIsMusic)
	{

		result = system->playSound(FMOD_CHANNEL_REUSE, pObject->m_pSound, false, &m_pMusicChannel);
		FMOD_ERROR_CHECK(result);
		if (m_pMusicChannel && bLooping)
		{
			m_pMusicChannel->setLoopCount(-1);
			m_pMusicChannel->setPosition(0, FMOD_TIMEUNIT_MS);
		} else
		{
			m_pMusicChannel->setLoopCount(0);
		}
		
		m_lastMusicID = (AudioHandle) m_pMusicChannel;
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;
		pObject->m_pLastChannelToUse = m_pMusicChannel;
		SetMusicVol(m_musicVol);
		return (AudioHandle) m_pMusicChannel;
		
	} else
	{
		result = system->playSound(FMOD_CHANNEL_FREE, pObject->m_pSound, false, &pChannel);
		FMOD_ERROR_CHECK(result);
		if (pChannel && bLooping)
		{
			pChannel->setLoopCount(-1);
		} else
		{
			pChannel->setLoopCount(0);

		}

		if (m_defaultVol != 1.0f)
		{
			SetVol((AudioHandle)pChannel, m_defaultVol);
		}
	}
	pObject->m_pLastChannelToUse = pChannel;
	return (AudioHandle)pChannel;
}

AudioHandle AudioManagerFMOD::Play( string fName, int vol, int pan /*= 0*/ )
{	
	if (!GetSoundEnabled()) return 0;

	fName = ModifiedFileName(fName);

	assert(system);
	FMOD::Channel *pChannel = (FMOD::Channel*)Play(fName, false);

	if (pChannel)
	{
		float fvol = float(vol)/100.0f;
#ifdef _DEBUG
		LogMsg("Playing %s at vol %.2f (from %d)", fName.c_str(), fvol, vol);
#endif
		pChannel->setVolume(fvol);
		pChannel->setPan(float(pan)/100.0f);
	}
	return (AudioHandle) pChannel;
}

void AudioManagerFMOD::Update()
{
	if (system)
	{
		system->update();
	}
}

void AudioManagerFMOD::Stop( AudioHandle soundID )
{
	if (!system) return;

	if (!soundID) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->stop();
	if (pChannel == m_pMusicChannel)
	{
		//m_lastMusicFileName = "";
		m_pMusicChannel = NULL;
	}
	
}

AudioHandle AudioManagerFMOD::GetMusicChannel()
{
	return (AudioHandle)m_pMusicChannel;

}

bool AudioManagerFMOD::IsPlaying( AudioHandle soundID )
{
	if (soundID == 0) return false;

	assert(system && soundID);
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	bool bPlaying = false;
	pChannel->isPlaying(&bPlaying);
	return bPlaying;
}


void AudioManagerFMOD::SetMusicEnabled( bool bNew )
{
	if (bNew != m_bMusicEnabled)
	{
		AudioManager::SetMusicEnabled(bNew);
		if (bNew)
		{
			if (!m_lastMusicFileName.empty() && system)
			{
				Play(m_lastMusicFileName, GetLastMusicLooping(), true);
				
			}
		} else
		{
			//kill the music
			if (system)
			Stop(GetMusicChannel());
		}
	}
	
}

void AudioManagerFMOD::StopMusic()
{
	Stop(GetMusicChannel());

	if (!m_bStreamMusic)
        DeleteSoundObjectByFileName(m_lastMusicFileName);
}

int AudioManagerFMOD::GetMemoryUsed()
{
	if (system)
	{
		/*
		FMOD_RESULT  result;
		unsigned int usedValue;

		result = system->getMemoryInfo(FMOD_MEMBITS_ALL, 0, &usedValue, 0);
		ERRCHECK(result);

		LogMsg("This FMOD::ChannelGroup is currently using %d bytes for DSP units and the ChannelGroup object itself", usedValue);
		*/
/*

			FMOD::Channel *pMusic = (FMOD::Channel *)GetMusicChannel();

			FMOD_RESULT  result;
			unsigned int usedValue;

			list<SoundObject*>::iterator itor = m_soundList.begin();

			int memUsed = 0;

			while (itor != m_soundList.end())
			{
				
					 result = (*itor)->m_pSound->getMemoryInfo(FMOD_MEMBITS_ALL, 0, &usedValue, 0);
					 memUsed += usedValue;
					 ERRCHECK(result);

				
				itor++;
			}	
			LogMsg("This FMOD::ChannelGroup is currently using %d bytes for DSP units and the ChannelGroup object itself", memUsed);
	
			*/
		
		
	

		int memUsed, maxAlloced;
		FMOD_Memory_GetStats(&memUsed, &maxAlloced, false);
		return memUsed;
	}

	return 0;
}

void AudioManagerFMOD::SetFrequency( AudioHandle soundID, int freq )
{
	assert(system);
	if (!soundID) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setFrequency(float(freq));

}

void AudioManagerFMOD::SetPan( AudioHandle soundID, float pan )
{
	assert(system && soundID);
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setPan(pan);
}

uint32 AudioManagerFMOD::GetPos( AudioHandle soundID )
{
	assert(system && soundID);
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	unsigned int pos;
	pChannel->getPosition(&pos, FMOD_TIMEUNIT_MS);
	return pos;
}

void AudioManagerFMOD::SetPos( AudioHandle soundID, uint32 posMS )
{
	assert(system);
	if (!soundID) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	
	pChannel->setPosition(posMS, FMOD_TIMEUNIT_MS);
}


void AudioManagerFMOD::SetVol( AudioHandle soundID, float vol )
{
	assert(system);
	if (!soundID) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setVolume(vol);
}

void AudioManagerFMOD::SetMusicVol(float vol )
{
	assert(system);
	if (m_pMusicChannel)
	{
		m_pMusicChannel->setVolume(vol);

	}
	m_musicVol = vol;
}


void AudioManagerFMOD::SetPriority( AudioHandle soundID, int priority )
{

}

void AudioManagerFMOD::Suspend()
{
	if (m_pMusicChannel)
	{
		m_pMusicChannel->setPaused(true);
	}
}

void AudioManagerFMOD::Resume()
{
	if (m_pMusicChannel)
	{
		m_pMusicChannel->setPaused(false);
	}

}
#endif