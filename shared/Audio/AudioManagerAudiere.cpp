#include "PlatformPrecomp.h"

#ifndef RT_WEBOS

#include "AudioManagerAudiere.h"
#include "util/MiscUtils.h"
   
AudioManagerAudiere::AudioManagerAudiere()
{
	m_pDevice = NULL;
	m_globalVol = 1.0f;
}
 
AudioManagerAudiere::~AudioManagerAudiere()
{
	Kill();
}

bool AudioManagerAudiere::Init()
{
	 m_pDevice = OpenDevice();

	return true;
}

void AudioManagerAudiere::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
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
			
			if ( (*itor)->m_pSound->isPlaying() )
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

void AudioManagerAudiere::Kill()
{
	if (m_pDevice)
	{
		KillCachedSounds(true, true, 0, 100, true);
		m_pDevice = NULL;
	}

}

bool AudioManagerAudiere::DeleteSoundObjectByFileName(string fName)
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

SoundObject * AudioManagerAudiere::GetSoundObjectByFileName(string fName)
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

SoundObject * AudioManagerAudiere::GetSoundObjectByPointer(void *p)
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

void AudioManagerAudiere::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{
	if (!m_pDevice) return;

	if (bIsMusic && !GetMusicEnabled()) return; //ignoring because music is off right now

	if (bIsMusic) 
	{
	
		m_lastMusicFileName = fName;
		if (m_bStreamMusic)
			bForceStreaming = true;
	}

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


		if (bForceStreaming){
			pObject->m_pSound  = OpenSound(m_pDevice, (basePath+fName).c_str(), true);
		}else{
			pObject->m_pSound  = OpenSound(m_pDevice, (basePath+fName).c_str());
		}

		if (!pObject->m_pSound)
		{
			LogMsg("Unable to find audio file %s",(basePath+fName).c_str() );
			SAFE_DELETE(pObject);
			return;
		}
		if(bLooping){
			pObject->m_pSound->setRepeat(true);
		}
																		    
		pObject->m_bIsLooping = bLooping;
		pObject->m_bIsMusic   = bIsMusic;

		m_soundList.push_back(pObject);
	}	
}


AudioHandle AudioManagerAudiere::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{
	if (!m_pDevice) return AUDIO_HANDLE_BLANK;

	if( !GetMusicEnabled() && bIsMusic )
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;

		return 0;
	}

	if (bIsMusic && m_bLastMusicLooping == bLooping && m_lastMusicFileName == fName && m_bLastMusicLooping)
	{
		return (AudioHandle) 0;
	}
	
	if(bIsMusic)
	{
		StopMusic();
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;
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

	if(pObject->m_pSound->isPlaying())
	{
		pObject->m_pSound->stop();
	}
	pObject->m_pSound->reset();
	pObject->m_pSound->setRepeat(bLooping);

	if (GetSoundEnabled()) pObject->m_pSound->play();
	
	if (bIsMusic)
	{
		SetMusicVol(m_musicVol);
	} else
	{
		if (m_globalVol != 1.0f || m_defaultVol != 1.0f)
		{
			SetVol( (AudioHandle)pObject, m_defaultVol*m_globalVol);
		}
	}
	return (AudioHandle)pObject;
}

/*
AudioHandle AudioManagerAudiere::Play( string fName, int vol, int pan)
{	
	if (!GetSoundEnabled()) return 0;

	Play(fName, false);


	SoundObject* Sound = GetSoundObjectByFileName(fName);

	float fvol = float(vol)/100.0f;
	Sound->m_pSound->setVolume(fvol);
	Sound->m_pSound->setPan(float(pan)/100.0f);

	return (AudioHandle) 0;
}

*/

void AudioManagerAudiere::Update()
{
	//no need to update
}

void AudioManagerAudiere::Stop( AudioHandle soundID )
{
	
	SoundObject *pObject = GetSoundObjectByPointer((void*)soundID);

	if (pObject && pObject->m_pSound)
	{
		pObject->m_pSound->stop();
	}
	
}

AudioHandle AudioManagerAudiere::GetMusicChannel()
{
	//skipped
	return (AudioHandle)0;

}

bool AudioManagerAudiere::IsPlaying( AudioHandle soundID )
{
	//skipped
	return false;
}


void AudioManagerAudiere::SetMusicEnabled( bool bNew ){
	if (bNew != m_bMusicEnabled){
		AudioManager::SetMusicEnabled(bNew);
		if (bNew){
			if (!m_lastMusicFileName.empty()){
				Play(m_lastMusicFileName, m_bLastMusicLooping, true);
				
			}
		}else{
			StopMusic();
		}
	}
}

void AudioManagerAudiere::StopMusic()
{
	m_lastMusicFileName = "";
	
	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor)->m_bIsMusic)
		{
			(*itor)->m_pSound->setPosition(0);
			(*itor)->m_pSound->stop();
		}
		itor++;
	}

}

int AudioManagerAudiere::GetMemoryUsed()
{
	//no clue how to get audiere memory usage :)
	return 0;
}

void AudioManagerAudiere::SetFrequency( AudioHandle soundID, int freq )
{
	//skipped
}

void AudioManagerAudiere::SetPan( AudioHandle soundID, float pan )
{
	//skipped
}

uint32 AudioManagerAudiere::GetPos( AudioHandle soundID )
{
	return (AudioHandle)0;
	
}

void AudioManagerAudiere::SetPos( AudioHandle soundID, uint32 posMS )
{
	
}


void AudioManagerAudiere::SetVol( AudioHandle soundID, float vol )
{

	if (soundID == -1)
	{
		m_globalVol = vol;
		return;
	}

	SoundObject *pObject = GetSoundObjectByPointer((void*)soundID);

	if (pObject && pObject->m_pSound)
	{
		pObject->m_pSound->setVolume(vol*m_globalVol);
	}
}

void AudioManagerAudiere::SetMusicVol(float vol )
{
	m_musicVol = vol;

	//oh, if it's already playing, mod that

	list<SoundObject*>::iterator itor = m_soundList.begin();

	while (itor != m_soundList.end())
	{
		if ( (*itor)->m_bIsMusic)
		{
			(*itor)->m_pSound->setVolume(vol*m_globalVol);
		}
		itor++;
	}
	
}


void AudioManagerAudiere::SetPriority( AudioHandle soundID, int priority )
{

}
#endif