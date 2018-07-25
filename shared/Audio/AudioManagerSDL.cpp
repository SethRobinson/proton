#include "PlatformPrecomp.h"

//NOTE:  Much of this code is from the tiltodemo sample from the Palm WebBOS PDK


#define NUM_CHANNELS 64

#include "AudioManagerSDL.h"
#include "util/MiscUtils.h"

//0 is a valid channel in SDL audio, but 0 is a bad audio handle the rest of Proton, this hack gets around it
#define C_CHANNEL_OFFSET_SO_ZERO_ISNT_USED 1


class SoundObject
{
public:
	SoundObject() :
	    m_pSound(NULL),
	    m_bIsLooping(false),
	    m_pLastChannelToUse(AUDIO_HANDLE_BLANK)
	{
	}

	~SoundObject()
	{
		Mix_FreeChunk(m_pSound);
	}

	Mix_Chunk *m_pSound;
	string m_fileName;
	bool m_bIsLooping;
	int m_pLastChannelToUse;
};

static bool g_MusicHasFinished = false;

void musicFinishedCallback()
{

#ifdef _DEBUG
	LogMsg("Got musicfinished callback");
#endif
	g_MusicHasFinished = true;
}

AudioManagerSDL::AudioManagerSDL()
{
	m_pMusicChannel = NULL;
	
}

AudioManagerSDL::~AudioManagerSDL()
{
	Kill();
}

AudioHandle AudioManagerSDL::Play( const string fileName )
{
	//LogMsg("Playing %s", fileName.c_str());
	assert(!"huh?");
	return AUDIO_HANDLE_BLANK;
}

#ifdef PLATFORM_HTML5
//Actually I guess we could support SDL2 if we wanted with emscripten, just haven't tried it
#include <SDL/SDL.h>
#else
#include <SDL2/SDL.h>
#endif



bool AudioManagerSDL::Init()
{

	//SDL_Init(SDL_INIT_AUDIO);

	SDL_InitSubSystem(SDL_INIT_AUDIO);

	
	int i;

	for (i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
		LogMsg("Audio driver %d: %s\n", i, SDL_GetAudioDriver(i));
	}

	//valid is directsound or winmm
	
#ifndef RT_USE_SDL1_MIXER
	//emscripten doesn't handle this
	if (SDL_AudioInit("directsound") != 0)
	{
		LogMsg("Error setting audio driver: %s", SDL_GetError());
	}
#endif
	

	//these two lines were added for HTML5, but don't seem to be needed for other stuff?

	int inittedFlags = Mix_Init(MIX_INIT_OGG);

	 if (inittedFlags != MIX_INIT_OGG)
	{
		LogMsg(Mix_GetError());
	}
	
	
	Mix_ReserveChannels(2);

	// we'll use SDL_Mixer to do all our sound playback. 
	// It's a simple system that's easy to use. The most 
	// complicated part about using SLD_Mixer is initting it. So 
	// let's talk about that. 
	// this is the function definition:
	// Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
	// here's what it wants:
	// 
	// frequency: this is the sample rate you want the audio to
	// play at. On the Palm, things are optimized for 44100 samples per
	// second (CD quality). Though you can send in whatever sample rate you like.
	// 
	// format: For the Palm, you should use AUDIO_S16
	// 
	// channels: 1 for mono. 2 for stereo
	// 
	// chunksize: How big one audio buffer is, in bytes.
	// 
	// this example has the recommended settings for initting the mixer:


//44100 or  22050
	int rate = 44100;
	Uint16 format = AUDIO_S16LSB;
	int channels = 2;
	int bufferSize = 2048;
	
#ifdef PLATFORM_HTML5
	int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
	assert(ret == 0);
#else

	

	if ( Mix_OpenAudio(rate, format, channels, bufferSize) == -1 )
	{
		// we had an error opening the audio
		LogMsg("unable to open Audio! Reason: %s\n", Mix_GetError());
		return false;

	}
#endif

	/*


	SDL_AudioSpec want, have;
	SDL_AudioDeviceID dev;

	SDL_memset(&want, 0, sizeof(want));
	want.freq = 22050;
	want.format = AUDIO_S16LSB;
	want.channels = 2;
	want.samples = 4096;
	//want.callback = MyAudioCallback;  // you wrote this function elsewhere.

	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (dev == 0)
	{
		LogMsg("Failed to open audio: %s\n", SDL_GetError());
	} else 
	{
		if (have.format != want.format) 
		{ // we let this one thing change.
			LogMsg("We didn't get Float32 audio format.\n");
		}
	}
*/

	if (Mix_AllocateChannels(NUM_CHANNELS) == -1)
	{
		LogMsg(Mix_GetError());
	}
	Mix_HookMusicFinished(musicFinishedCallback);

	//SDL_PauseAudioDevice(dev, 0); // start audio playing.
	SDL_PauseAudio(0);

	LogMsg("SDL2_mixer initted using %s", SDL_GetCurrentAudioDriver());
	
	
#ifdef PLATFORM_HTML5
	LogMsg("Preloading audio/blank.wav, an audio sample MUST exist here, we use it on the first tap to get around locked audio on iOS");
	Preload("audio/blank.wav");
	
#endif
	return true;
}

void AudioManagerSDL::KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying)
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
			if ((*itor)->m_pLastChannelToUse != AUDIO_HANDLE_BLANK && IsPlaying((AudioHandle)(*itor)->m_pLastChannelToUse))
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
		//m_pMusicChannel = NULL;
	}
}

void AudioManagerSDL::Kill()
{
	Mix_HaltMusic();
	if ( m_pMusicChannel != NULL )
	{
		// free up any memore in use by the music track
		Mix_FreeMusic(m_pMusicChannel);

		// set it to null for safety (ensuring that 
		// if we acidentally refer to this variable after
		// deletion, at least it will be null)
		m_pMusicChannel = NULL; 
	}

	// close out the audio
	Mix_CloseAudio();
}

bool AudioManagerSDL::DeleteSoundObjectByFileName(string fName)
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

SoundObject * AudioManagerSDL::GetSoundObjectByFileName(string fName)
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

void AudioManagerSDL::Preload( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath /*= true*/, bool bForceStreaming )
{

	if (bIsMusic) return;//we don't preload music that way

	fName = ModifiedFileName(fName);

	SoundObject *pObject = GetSoundObjectByFileName( fName.c_str());

	if (!pObject)
	{
		//create it
		pObject = new SoundObject;
		pObject->m_fileName = fName;

		//assert(! (GetFileExtension(fName) == "mp3" || GetFileExtension(fName) == "ogg") && "SDL mixer doesn't support mp3/ogg for non music playback though");
	
#ifndef PLATFORM_HTML5 //html5's emscripten version does let the browser play mp3.  Safari on ios can't play ogg though
		if (GetFileExtension(fName) == "mp3")
		{
			fName = ModifyFileExtension(fName, "ogg");
			StringReplace("/mp3", "/ogg", fName);
		} 
#endif

		string basePath;
		if (bAddBasePath)
		{
			basePath = GetBaseAppPath();
		}

//		LogMsg("Preloading sfx %s", (basePath+fName).c_str());

		pObject->m_pSound = Mix_LoadWAV( (basePath+fName).c_str());
		if (!pObject->m_pSound)
		{
			LogMsg("Error loading %s (%s)", (basePath+fName).c_str(), Mix_GetError());
			delete pObject;
			return;
		}
		
#ifdef _DEBUG
		LogMsg("Caching out %s", fName.c_str());
#endif
		
		pObject->m_bIsLooping = bLooping;

		m_soundList.push_back(pObject);
	}
}


AudioHandle AudioManagerSDL::Play( string fName, bool bLooping /*= false*/, bool bIsMusic, bool bAddBasePath, bool bForceStreaming)
{


#ifdef _DEBUG
	LogMsg("********** AudioSDL: Thinking of playing %s, music=%d", fName.c_str(), int(bIsMusic));
#endif

	if (!GetSoundEnabled() && !bIsMusic) return AUDIO_HANDLE_BLANK;
	
	fName = ModifiedFileName(fName);

	if (!GetMusicEnabled() && bIsMusic)
	{
		m_bLastMusicLooping = bLooping;
		m_lastMusicFileName = fName;

		return AUDIO_HANDLE_BLANK;
	}

	if (bIsMusic )
	{
		if (m_bLastMusicLooping == bLooping && m_lastMusicFileName == fName && m_bLastMusicLooping && IsPlaying((AudioHandle) m_pMusicChannel))
		{
			return (AudioHandle) m_pMusicChannel;
		} else
		{
			StopMusic();
		}
	}

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
	
#ifdef _DEBUG
		LogMsg("Stopped music, now playing as %s", (basePath+fName).c_str());
#endif

#ifndef PLATFORM_HTML5 //html5's emscripten version does let the browser play mp3.  Safari on ios can't play ogg though
		if (GetFileExtension(fName) == "mp3")
		{
			fName = ModifyFileExtension(fName, "ogg");
			StringReplace("/mp3", "/ogg", fName);
		}
#endif

		m_pMusicChannel = Mix_LoadMUS( (basePath+fName).c_str());

#ifdef _DEBUG
if (!m_pMusicChannel)
{
	LogMsg(Mix_GetError());
}
#endif
		if (!m_pMusicChannel && !bAddBasePath)
		{
			LogError("Couldn't load %s, trying again with full path", (basePath+fName).c_str());

			basePath = GetBaseAppPath();
			//try again with the basepath added.. the SDL sound system on webos seems to require it
			m_pMusicChannel = Mix_LoadMUS((basePath+fName).c_str());
		}

		if (!m_pMusicChannel)
		{
			LogError("Unable to load music file %s. Missing? (%s)", (basePath+fName).c_str(), Mix_GetError());
			return AUDIO_HANDLE_BLANK;
		}
		m_lastMusicID = (AudioHandle) m_pMusicChannel;
		m_bLastMusicLooping = bLooping;
		int ret = Mix_PlayMusic(m_pMusicChannel, loops);
		
		if (ret == -1)
		{
			LogError("Unable to play music file %s. %s", (GetBaseAppPath()+fName).c_str(), Mix_GetError());
		}
		SetMusicVol(m_musicVol);

		return (AudioHandle) m_pMusicChannel;
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
			return AUDIO_HANDLE_BLANK;

		}
	}


#ifdef _DEBUG
	LogMsg("AudioSDL: Playing sfx %s", fName.c_str());
#endif

	//play it
	
	int channel = Mix_PlayChannel(-1, pObject->m_pSound, loops);
	if (channel == -1)
	{
		pObject->m_pLastChannelToUse = AUDIO_HANDLE_BLANK;
		return AUDIO_HANDLE_BLANK;
	}
	pObject->m_pLastChannelToUse = channel + C_CHANNEL_OFFSET_SO_ZERO_ISNT_USED;

	//need this because sometimes it's set to nothing by default??
	SetVol(pObject->m_pLastChannelToUse, m_defaultVol);

	return (AudioHandle)pObject->m_pLastChannelToUse ;
}

AudioHandle AudioManagerSDL::Play( string fName, int vol, int pan /*= 0*/ )
{	
	
	assert(!"We don't support this");
	return AUDIO_HANDLE_BLANK;
}

void AudioManagerSDL::Update()
{
	if (g_MusicHasFinished)
	{
		//the problem is the callback is so slow, it comes after we've started new music already.
		//g_MusicHasFinished = false;
		//StopMusic();
	}
}

void AudioManagerSDL::Stop( AudioHandle soundID )
{
	
	if (!soundID) return;
	
	if (soundID == (AudioHandle)m_pMusicChannel)
	{
		StopMusic();
		return;
	}
	//pChannel->stop();

	Mix_HaltChannel(soundID-C_CHANNEL_OFFSET_SO_ZERO_ISNT_USED);

}

AudioHandle AudioManagerSDL::GetMusicChannel()
{
	return (AudioHandle)m_pMusicChannel;

}

bool AudioManagerSDL::IsPlaying( AudioHandle soundID )
{
	if (soundID == AUDIO_HANDLE_BLANK) return false;


	if (soundID == (AudioHandle) m_pMusicChannel)
	{
		return Mix_PlayingMusic() != 0;
	}

	return Mix_Playing(soundID-C_CHANNEL_OFFSET_SO_ZERO_ISNT_USED) != 0;
}


void AudioManagerSDL::SetMusicEnabled( bool bNew )
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

void AudioManagerSDL::StopMusic()
{
#ifdef _DEBUG
	LogMsg("Deleting music %s", m_lastMusicFileName.c_str());
#endif
	DeleteSoundObjectByFileName(m_lastMusicFileName);
	if (m_pMusicChannel)
	{
#ifdef _DEBUG
		LogMsg("Also halting the music channel");
#endif

		Mix_HaltMusic();
		Mix_FreeMusic(m_pMusicChannel);
	}
	m_pMusicChannel = NULL;
	m_lastMusicID = AUDIO_HANDLE_BLANK;

	m_lastMusicFileName = "";//don't care anymore

}

void AudioManagerSDL::FadeOutMusic(unsigned int duration)
{
	Mix_FadeOutMusic(duration);
}

int AudioManagerSDL::GetMemoryUsed()
{
	return 0;
}

void AudioManagerSDL::SetFrequency( AudioHandle soundID, int freq )
{
	assert(soundID);
	//SDL::Channel *pChannel = (SDL::Channel*) soundID;
	//pChannel->setFrequency(float(freq));
}

void AudioManagerSDL::SetPan( AudioHandle soundID, float pan )
{
	assert(soundID);
	
	//SDL::Channel *pChannel = (SDL::Channel*) soundID;
	//pChannel->setPan(pan);
}

uint32 AudioManagerSDL::GetPos( AudioHandle soundID )
{
	assert(soundID);
	//unsigned int pos;
//	pChannel->getPosition(&pos, SDL_TIMEUNIT_MS);
	return 0;
}

void AudioManagerSDL::SetPos( AudioHandle soundID, uint32 posMS )
{
	assert(soundID);
	
	//pChannel->setPosition(posMS, SDL_TIMEUNIT_MS);
}


void AudioManagerSDL::SetVol( AudioHandle soundID, float vol )
{
	int ivol =  int(vol*MIX_MAX_VOLUME);

	if (soundID == AUDIO_HANDLE_BLANK) return;
#ifdef _DEBUG
	//ivol = 128;
	//LogMsg("Setting audio handle %d to %d", soundID, ivol);
#endif

	Mix_Volume(soundID-C_CHANNEL_OFFSET_SO_ZERO_ISNT_USED,ivol);
	//assert(soundID);
	//pChannel->setVolume(vol);
}

void AudioManagerSDL::SetMusicVol(float vol )
{
	int ivol =  int(vol*MIX_MAX_VOLUME);
	if (m_pMusicChannel)
	{
		Mix_VolumeMusic(ivol);
	}
	m_musicVol = vol;
}


void AudioManagerSDL::SetPriority( AudioHandle soundID, int priority )
{

}


void AudioManagerSDL::Suspend()
{
	LogMsg("Pausing SDL audio");
	SDL_PauseAudio(1); //seems to do nothing on chrome webgl

	if (GetMusicEnabled())
	{
		//StopMusic();
		if (m_pMusicChannel)
		{
#ifdef _DEBUG
			LogMsg("Also halting the music channel");
#endif

			Mix_HaltMusic();
			Mix_FreeMusic(m_pMusicChannel);
		}
		m_pMusicChannel = NULL;
		m_lastMusicID = AUDIO_HANDLE_BLANK;
	}

	/*
	if (m_pMusicChannel)
	{
		m_pMusicChannel->setPaused(true);
	}
	*/
}

void AudioManagerSDL::Resume()
{
	LogMsg("Unpausing SDL audio");
	
	SDL_PauseAudio(0);

	if (GetMusicEnabled())
	{
		if (!m_lastMusicFileName.empty())
		{
			Play(m_lastMusicFileName, GetLastMusicLooping(), true);

		}
	}
	
	/*
	if (m_pMusicChannel)
	{
		m_pMusicChannel->setPaused(false);
	}
	*/

}

#endif
