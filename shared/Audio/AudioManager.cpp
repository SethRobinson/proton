#include "PlatformPrecomp.h"
#include "AudioManager.h"

AudioManager::AudioManager()
{
	m_bMusicEnabled = true;
	m_bSoundEnabled = true;
	m_bLastMusicLooping = true;
	m_lastMusicID = AUDIO_HANDLE_BLANK;
	m_bVibrationDisabled = false;
	m_musicVol = m_defaultVol = m_midiVolumeMod = 1.0f;
	m_bStreamMusic = true;
	m_bPreferOGG = false;
	
}

AudioManager::~AudioManager()
{
}

void AudioManager::StopMusic()
{
	if (m_lastMusicID != AUDIO_HANDLE_BLANK)
	{
		Stop(m_lastMusicID);
		m_lastMusicID = AUDIO_HANDLE_BLANK;
		m_lastMusicFileName = "";
	}
}


void AudioManager::SetDLS(string fName)
{
	m_midiSoundBankFile = fName;
}


void AudioManager::SetRequestedDriverByName(string requestedPartialDriverName)
{
	m_requestedPartialDriverName = requestedPartialDriverName;
}

AudioHandle AudioManager::Play( string fName, bool bLooping , bool bIsMusic , bool bAddBasePath , bool bForceStreaming )
{
//	LogMsg("AudioManager::Play activated, not handled");
	return AUDIO_HANDLE_BLANK;
}

void AudioManager::SetMusicStreaming( bool bStreaming )
{
	m_bStreamMusic = bStreaming;
}

void AudioManager::ForceAudioExtension( const string extension )
{
	m_forcedAudioExtension = extension;
}


string AudioManager::ModifiedFileName(string fName)
{
	if (m_forcedAudioExtension.empty()) return fName;

	return ModifyFileExtension(fName, m_forcedAudioExtension);
}

std::string AudioManager::GetAudioSystemName()
{
	return "unknown";
}

bool AudioManager::IsPlayingMusic()
{
		if (m_lastMusicID == AUDIO_HANDLE_BLANK) return false;
		return IsPlaying(m_lastMusicID);
}

void PlaySound(VariantList *pVList)
{
	string fName = pVList->Get(0).GetString();
	GetAudioManager()->Play(fName);
}

void PlayMusic(VariantList* pVList)
{
	string fName = pVList->Get(0).GetString();
	GetAudioManager()->Play(fName, true, true);
}

void StopMusic(VariantList* pVList)
{
	GetAudioManager()->StopMusic();
}

