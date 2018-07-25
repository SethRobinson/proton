#include "PlatformPrecomp.h"

#ifndef RT_WEBOS

#include "AudioManagerFMODStudio.h"
#include "util/MiscUtils.h"

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

void FMOD_ERROR_CHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		LogMsg("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		//exit(-1);
	}
}

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line)
{
	if (result != FMOD_OK)
	{
		LogMsg("%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString(result));
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
	void             *extradriverdata = 0;


	/*
	Create a System object and initialize
	*/

	LogMsg("Creating FMOD system...");
	result = FMOD::System_Create(&system);
	ERRCHECK(result);
	LogMsg("Getting version...");

	result = system->getVersion(&version);
	ERRCHECK(result);

	
	if (version < FMOD_VERSION)
	{
		LogMsg("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
		return false;
	}
	
#ifdef PLATFORM_HTML5
	//helps with stuttering
	system->setDSPBufferSize(1024*4, 2);
#endif
	
	LogMsg("Initting FMOD...");
	//extradriverdata
	result = system->init(1024, FMOD_INIT_NORMAL, extradriverdata);
	ERRCHECK(result);
	LogMsg("FMOD initted");
	return true;
}

void AudioManagerFMOD::ReinitForHTML5()
{
	//this gets around the Safari ios audio block, if called on a touch-down.  Doing a Kill() and Init() also works, but this way
	//we can try to continue the song that was playing

	uint32 musicPosition = 0;
	bool bResumeMusic = false;
	 
	if ((!m_lastMusicFileName.empty()) && GetMusicEnabled() && IsPlaying(GetMusicChannel()))
	{
		bResumeMusic = true;
		musicPosition = GetPos(GetMusicChannel());
	}

	KillCachedSounds(true, true, 0, 0, true);
	system->close();
	FMOD_RESULT       result;
	void             *extradriverdata = 0;
	
	result = system->init(1024, FMOD_INIT_NORMAL, extradriverdata);
	ERRCHECK(result);
	LogMsg("FMOD initted again to enable audio on Safari");

	//start back up the music?
	if (bResumeMusic)
	{
		Play(m_lastMusicFileName, true, true, true);
		SetPos(GetMusicChannel(), musicPosition);
	}
	
	
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
		KillCachedSounds(true, true, 0, 0, true);
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
		int fModParms = FMOD_LOWMEM|FMOD_LOOP_NORMAL; //tell it looking anyway, avoid a click later
		if (bLooping)
		{
			fModParms = FMOD_LOOP_NORMAL|FMOD_LOWMEM;
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

//		result = system->playSound(FMOD_CHANNEL_REUSE, pObject->m_pSound, false, &m_pMusicChannel);
		result = system->playSound(pObject->m_pSound, 0, false, &m_pMusicChannel);
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
//		result = system->playSound(FMOD_CHANNEL_FREE, pObject->m_pSound, false, &pChannel);
		
		result = system->playSound(pObject->m_pSound, 0, false, &pChannel);

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

	if (soundID == AUDIO_HANDLE_BLANK) return;

	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->stop();
	if (pChannel == m_pMusicChannel)
	{
		//m_lastMusicFileName = "";
		m_pMusicChannel = (FMOD::Channel*)AUDIO_HANDLE_BLANK;
	}
	
}

AudioHandle AudioManagerFMOD::GetMusicChannel()
{
	return (AudioHandle)m_pMusicChannel;

}

bool AudioManagerFMOD::IsPlaying( AudioHandle soundID )
{
	if (soundID == AUDIO_HANDLE_BLANK) return false;

	assert(system);
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
	if (soundID == AUDIO_HANDLE_BLANK) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setFrequency(float(freq));

}

void AudioManagerFMOD::SetPan( AudioHandle soundID, float pan )
{
	assert(system);
	if (soundID == AUDIO_HANDLE_BLANK) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setPan(pan);
}

uint32 AudioManagerFMOD::GetPos( AudioHandle soundID )
{
	assert(system);
	if (soundID == AUDIO_HANDLE_BLANK) return 0;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	unsigned int pos;
	pChannel->getPosition(&pos, FMOD_TIMEUNIT_MS);
	return pos;
}

void AudioManagerFMOD::SetPos( AudioHandle soundID, uint32 posMS )
{
	assert(system);
	if (soundID == AUDIO_HANDLE_BLANK) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	
	pChannel->setPosition(posMS, FMOD_TIMEUNIT_MS);
}


void AudioManagerFMOD::SetVol( AudioHandle soundID, float vol )
{
	assert(system);
	if (soundID == AUDIO_HANDLE_BLANK) return;
	FMOD::Channel *pChannel = (FMOD::Channel*) soundID;
	pChannel->setVolume(vol);
}

void AudioManagerFMOD::SetMusicVol(float vol )
{
	assert(system);
	if (m_pMusicChannel != AUDIO_HANDLE_BLANK)
	{
		m_pMusicChannel->setVolume(vol);
	}
	m_musicVol = vol;
}


void AudioManagerFMOD::SetPriority( AudioHandle soundID, int priority )
{

}

void AudioManagerFMOD::SetGlobalPause(bool bPaused)
{
	FMOD::ChannelGroup *pChannelGroup = NULL;
	system->getMasterChannelGroup(&pChannelGroup);
	pChannelGroup->setPaused(bPaused);
}

void AudioManagerFMOD::Suspend()
{
	if (!system) return;
	SetGlobalPause(true);
}

void AudioManagerFMOD::Resume()
{
	if (!system) return;
	SetGlobalPause(false);
}
#endif