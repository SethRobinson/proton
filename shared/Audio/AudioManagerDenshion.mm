#include "AudioManagerDenshion.h"
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>
#include "CocosDenshion/SimpleAudioEngine.h"

#import "CocosDenshion/CDAudioManager.h"
#if TARGET_OS_IPHONE == 1
#include <AVFoundation/AVFoundation.h>
#endif

#include "BaseApp.h"

AVAudioPlayer * m_bgMusicPlayer;


AudioManagerDenshion::AudioManagerDenshion() :
	m_musicFadeOutInProgress(false),
	m_musicFadeOutStartTime(0),
	m_musicFadeOutDuration(0)
{
	m_bgMusicPlayer = NULL;
	m_bDisabledMusicRecently = false;
	
	m_lastMusicID = (AudioHandle)-2;
}

AudioManagerDenshion::~AudioManagerDenshion()
{
}

bool AudioManagerDenshion::Init()
{
	[CDSoundEngine setMixerSampleRate: 22050];
	[SimpleAudioEngine sharedEngine];

    //Let's make sure it's fully initialized before continuing, fixes issues with
    //using the 60beat joystick with this..(it uses the audio system for the mic..)
    
    while ([CDAudioManager sharedManagerState] != kAMStateInitialised) 
    {
		[NSThread sleepForTimeInterval:0.1];
	}	
	[[CDAudioManager sharedManager].soundEngine setSourceGroupNonInterruptible:0 isNonInterruptible:TRUE];
	[[CDAudioManager sharedManager] setResignBehavior:kAMRBStopPlay autoHandle:YES];
	
	LogMsg("Initialized Denshion");
	return true; //success
}

#if TARGET_OS_IPHONE == 1

bool CheckIfOtherAudioIsPlaying()
{
UInt32 isPlaying;
UInt32 propertySize = sizeof(isPlaying);

AudioSessionInitialize(NULL, NULL, NULL, NULL);
AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &isPlaying);

#ifdef _DEBUG
	LogMsg("Playing is %d", isPlaying);
#endif
if (isPlaying != 0) 
{
		return true;
}

	
/*
// since no other audio is *supposedly* playing, then we will make darn sure by changing the audio session category temporarily
// to kick any system remnants out of hardware (iTunes (or the iPod App, or whatever you wanna call it) sticks around)
UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
AudioSessionSetProperty(kAudioSessionProperty_Audi oCategory, sizeof(sessionCategory), &sessionCategory);
AudioSessionSetActive(YES);

// now change back to ambient session category so our app honors the "silent switch"
sessionCategory = kAudioSessionCategory_AmbientSound;
AudioSessionSetProperty(kAudioSessionProperty_Audi oCategory, sizeof(sessionCategory), &sessionCategory);
*/

return false;
}

#endif

void AudioManagerDenshion::Vibrate(int duration)
{
#if TARGET_OS_IPHONE == 1

	if (!m_bVibrationDisabled)
	{
		AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
	}
#endif
}


void AudioManagerDenshion::DestroyAudioCache()
{

}

void AudioManagerDenshion::Stop(AudioHandle id)
{
	[[SimpleAudioEngine sharedEngine]stopEffect: id];
}

void AudioManagerDenshion::Preload(string fName, bool bLooping, bool bIsMusic, bool bAddBasePath , bool bForceStreaming)
{
	if (bIsMusic)
	{
		assert(!"We don't have that yet..");
		return;
	}
    string basePath;
    
    if (bAddBasePath)
    {
        basePath = GetBaseAppPath();
    }
    
	NSString *soundFile =  [NSString stringWithCString:  (basePath+fName).c_str() encoding: [NSString defaultCStringEncoding]];
	
	[[SimpleAudioEngine sharedEngine] preloadEffect: soundFile];
	//GetAudioObjectByFileName(fName, bLooping);
}

AudioHandle AudioManagerDenshion::PlayWithAVPlayer( string fName)
{
	if (!m_bSoundEnabled) return AUDIO_HANDLE_BLANK;

	NSString *soundFile =  [NSString stringWithCString:  fName.c_str() encoding: [NSString defaultCStringEncoding]];
	
	NSError *err;
	AVAudioPlayer* audioPlayer =  [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:soundFile] error:&err];
	//[audioPlayer setDelegate:self];
	[audioPlayer prepareToPlay];
	[audioPlayer play]; // returns BOOL
	
	return AUDIO_HANDLE_BLANK;
}

AudioHandle AudioManagerDenshion::Play( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath, bool bForceStreaming )
{
#ifdef _DEBUG
	//LogMsg("Playing %s", fName.c_str());
#endif

	if (!GetSoundEnabled() && !bIsMusic) return AUDIO_HANDLE_BLANK;
	
	string basePath;

	if (bAddBasePath)
	{
		basePath = GetBaseAppPath();
	}

	if (bIsMusic)
	{
		if (!GetMusicEnabled()) 
		{
#ifdef _DEBUG
			LogMsg("Music disabled");
#endif

			m_lastMusicFileName = fName;
			m_bLastMusicLooping = bLooping;

			return AUDIO_HANDLE_BLANK;
		}
		
		if (m_lastMusicFileName == fName && m_bLastMusicLooping == true && !m_bDisabledMusicRecently)
		{
			if (IsPlaying(m_lastMusicID))
			{
#ifdef _DEBUG
				LogMsg("Seems to already be playing");
#endif
				return m_lastMusicID;
			}
		}
		
		string fNameTemp = basePath+fName;
		StringReplace(".ogg",".mp3", fNameTemp);

		NSString *soundFile =  [NSString stringWithCString:  fNameTemp.c_str() encoding: [NSString defaultCStringEncoding]];
	
		if (![[NSFileManager defaultManager] fileExistsAtPath:soundFile])
		{
			LogMsg("Error: Can't locate audio file %s!", fName.c_str());
			return AUDIO_HANDLE_BLANK;
		}
		if (m_bgMusicPlayer)
		{
			[m_bgMusicPlayer stop];
			[m_bgMusicPlayer release];
			m_bgMusicPlayer = NULL;
		}
		
		m_bgMusicPlayer = [AVAudioPlayer alloc];

		[m_bgMusicPlayer initWithContentsOfURL: [NSURL fileURLWithPath:soundFile] error:nil];
		
		if (bLooping)
		{
			m_bgMusicPlayer.numberOfLoops = -1;
		}
		
		m_lastMusicFileName = fName;
		m_bLastMusicLooping = bLooping;
		m_bDisabledMusicRecently = false;
		m_bgMusicPlayer.volume = m_musicVol;
		[m_bgMusicPlayer prepareToPlay];
		[m_bgMusicPlayer play];
		
		return m_lastMusicID;
	}

	UInt32 soundId = AUDIO_HANDLE_BLANK;

	NSString *soundFile =  [NSString stringWithCString:  (basePath+fName).c_str() encoding: [NSString defaultCStringEncoding]];
	
	soundId = [[SimpleAudioEngine sharedEngine] playEffect: soundFile pitch:1.0f pan:0.0f gain:m_defaultVol loop:bLooping];
	return soundId;
}

void AudioManagerDenshion::Kill()
{
	DestroyAudioCache();
}

void AudioManagerDenshion::SetMusicEnabled( bool bNew )
{
	if (bNew != m_bMusicEnabled)
	{
		AudioManager::SetMusicEnabled(bNew);
		if (!bNew)
		{
			//music has been disabled
			if (m_bgMusicPlayer)
			{
				[m_bgMusicPlayer stop];
				[m_bgMusicPlayer release];
				m_bgMusicPlayer = NULL;
				m_bDisabledMusicRecently = true;
			}
		} else
		{
			//turn music back on?
			if (!m_lastMusicFileName.empty())
			{
				Play(m_lastMusicFileName, m_bLastMusicLooping, true);
			}
			m_bDisabledMusicRecently = true;
		}
	}
}

void AudioManagerDenshion::StopMusic()
{
	LogMsg("Killing music..");
	
	if (m_bgMusicPlayer)
	{
		LogMsg("bgMusicPlayer was active, killing it");
		[m_bgMusicPlayer stop];
		[m_bgMusicPlayer release];
		m_bgMusicPlayer = NULL;
		
		m_musicFadeOutInProgress = false;
	}
	
	m_lastMusicFileName = "";
	m_bLastMusicLooping = false;
	m_bDisabledMusicRecently = false;
}

void AudioManagerDenshion::Update()
{
	if (m_musicFadeOutInProgress)
	{
		unsigned int now = GetBaseApp()->GetGameTick();
		
		if (now - m_musicFadeOutStartTime >= m_musicFadeOutDuration)
		{
			StopMusic();
			m_musicFadeOutInProgress = false;
		} else
		{
			if (now - m_musicFadeOutPreviouslySetTime >= 100)
			{
				float phase = 1.0f - (float(now - m_musicFadeOutStartTime) / float(m_musicFadeOutDuration));
				m_bgMusicPlayer.volume = phase * m_musicVol;
				m_musicFadeOutPreviouslySetTime = now;
			}
		}
	}
}

void AudioManagerDenshion::FadeOutMusic(unsigned int duration)
{
	if (m_bgMusicPlayer == NULL)
	{
		return;
	}
	
	if (duration == 0)
	{
		StopMusic();
		return;
	}
	
	m_musicFadeOutInProgress = true;
	m_musicFadeOutDuration = duration;
	m_musicFadeOutStartTime = GetBaseApp()->GetGameTick();
	m_musicFadeOutPreviouslySetTime = m_musicFadeOutStartTime;
}

bool AudioManagerDenshion::IsPlaying(AudioHandle soundID)
{
	if (soundID == m_lastMusicID)
	{
		return m_bgMusicPlayer != NULL;
	}
	
	// FIXME report the correct playing status for sound effects
	return false;
}

void AudioManagerDenshion::SetVol( AudioHandle soundID, float vol )
{
	if (!soundID || soundID == CD_NO_SOURCE) return;

    alGetError();

	if (soundID == -1)
	{
		//-1 means global
		alListenerf(AL_GAIN, vol);
	} else
	{
		alSourcef(soundID, AL_GAIN, vol);
	}
    
    ALenum error = alGetError();

    if(error != AL_NO_ERROR) 
    {
       LogMsg("AudioManagerDenshion, can't set volume: %x", error);
    }	
    
}

void AudioManagerDenshion::SetMusicVol(float vol )
{
	//CDSoundEngine *sndEng = [[SimpleAudioEngine sharedEngine]  getSoundEngine];
	if (m_bgMusicPlayer)
	{
		m_bgMusicPlayer.volume =vol;
	}
	m_musicVol = vol;
}


