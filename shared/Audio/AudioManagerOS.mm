#include "AudioManagerOS.h"
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AVFoundation/AVFoundation.h>

AVAudioPlayer * m_bgMusicPlayer;




AudioManagerOS::AudioManagerOS()
{
	m_bgMusicPlayer = NULL;
	m_bDisabledMusicRecently = false;
}

AudioManagerOS::~AudioManagerOS()
{
}


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


AudioObjectOS * AudioManagerOS::GetAudioObjectByFileName(const string &fName, bool bLooping)
{
	for (int i=0; i < m_audioList.size(); i++)
	{
		if (m_audioList[i].fName == fName)
			return &m_audioList[i];
	}

	UInt32 id = AUDIO_HANDLE_BLANK;

	//create it
	if (id == AUDIO_HANDLE_BLANK)
	{
		SystemSoundID myID;
		
		NSString *soundFile =  [NSString stringWithCString:  (GetBaseAppPath()+fName).c_str() encoding: [NSString defaultCStringEncoding]];

		NSURL *url = [NSURL fileURLWithPath:soundFile];

		OSStatus s = AudioServicesCreateSystemSoundID( (CFURLRef) url, &myID );

		if (s == noErr)
		{
				
			AudioObjectOS a;
			a.fName = fName;
			a.m_id = myID;
			m_audioList.push_back(a);
			//LogMsg("Cached %s at bufferID %d", a.fName.c_str(), a.m_id);
		} else
		{
			LogMsg("Error loading %s (OSStatus %d)", (GetBaseAppPath()+fName).c_str(), s);
			return NULL;
		}
	}
	return &m_audioList.back();
}

AudioObjectOS * AudioManagerOS::GetAudioObjectByID(uint32 id)
{
	for (int i=0; i < m_audioList.size(); i++)
	{
		if (m_audioList[i].m_id == id)
			return &m_audioList[i];
	}
	return NULL;
}

void AudioManagerOS::Vibrate(int duration)
{
	if (!m_bVibrationDisabled)
	{
		AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
	}
}

void AudioManagerOS::KillAudioObjectByFileName(const string &fName)
{

}

void AudioManagerOS::DestroyAudioCache()
{

	//m_audioList.clear();
}

void AudioManagerOS::Stop(AudioHandle id)
{
	
}

void AudioManagerOS::Preload(string fName, bool bLooping, bool bIsMusic, bool bAddBasePath , bool bForceStreaming)
{
 
	if (bIsMusic)
	{
		assert(!"We don't have that yet..");
		return;
	}
	GetAudioObjectByFileName(fName, bLooping);
}

AudioHandle AudioManagerOS::PlayWithAVPlayer( string fName)
{
	NSString *soundFile =  [NSString stringWithCString:  fName.c_str() encoding: [NSString defaultCStringEncoding]];
	
	NSError *err;
	AVAudioPlayer* audioPlayer =  [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:soundFile] error:&err];
	//[audioPlayer setDelegate:self];
	[audioPlayer prepareToPlay];
	[audioPlayer play]; // returns BOOL
	
	return AUDIO_HANDLE_BLANK;
}

AudioHandle AudioManagerOS::Play( string fName, bool bLooping /*= false*/, bool bIsMusic /*= false*/, bool bAddBasePath, bool bForceStreaming )
{
	#ifdef _DEBUG
	LogMsg("Playing %s", fName.c_str());
	#endif

	if (bIsMusic)
	{
		
		if (!GetMusicEnabled()) 
		{
			
			m_lastMusicFileName = fName;
			m_bLastMusicLooping = bLooping;

	#ifdef _DEBUG
LogMsg("Music disabled, pretending to play");
#endif
			return AUDIO_HANDLE_BLANK;
		}
		
		
		if (m_lastMusicFileName == fName && m_bLastMusicLooping == true && !m_bDisabledMusicRecently)
		{
			//it's already playing!
			#ifdef _DEBUG
			LogMsg("Seems to already be playing");
			#endif
			return AUDIO_HANDLE_BLANK;
		}
		
		string basePath;
		
		if (bAddBasePath)
		{
			basePath = GetBaseAppPath();
		}
        string fNameTemp = basePath+fName;
        StringReplace(".ogg",".mp3", fNameTemp);
        
		NSString *soundFile =  [NSString stringWithCString:  fName.c_str() encoding: [NSString defaultCStringEncoding]];
	
        if (![[NSFileManager defaultManager] fileExistsAtPath:soundFile])
        {
            LogMsg("Error: Can't locate audio file %s!",
                   fName.c_str());
            return AUDIO_HANDLE_BLANK;
        }
        
		if (m_bgMusicPlayer)
		{
			[m_bgMusicPlayer stop];
			[m_bgMusicPlayer release];
 			 m_bgMusicPlayer = NULL;
		}
		
        
		m_bgMusicPlayer = [AVAudioPlayer alloc ];
		
		[m_bgMusicPlayer initWithContentsOfURL: [NSURL fileURLWithPath:soundFile] error:nil];
		if (bLooping)
		{
			m_bgMusicPlayer.numberOfLoops = -1;
		}

	m_lastMusicFileName = fName;
	m_bLastMusicLooping = bLooping;
	m_bDisabledMusicRecently = false;
	
	[m_bgMusicPlayer prepareToPlay];

	// when you want to play the file

	[m_bgMusicPlayer play];
		
		
	return AUDIO_HANDLE_BLANK;
	}

	UInt32 soundId = AUDIO_HANDLE_BLANK;

	AudioObjectOS *pAudio = GetAudioObjectByFileName(fName, bLooping);
	if (!pAudio) return soundId;

	soundId = pAudio->m_id;

	AudioServicesPlaySystemSound(soundId);
	
	return soundId;
}

bool AudioManagerOS::Init()
{
	
	return true; //success
}

void AudioManagerOS::Kill()
{
	DestroyAudioCache();
	
}


void AudioManagerOS::SetMusicEnabled( bool bNew )
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

			m_lastMusicID = AUDIO_HANDLE_BLANK;
	
		 
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

void AudioManagerOS::StopMusic()
{
		
		LogMsg("Killing music..");
		
		if (m_bgMusicPlayer)
		{
			LogMsg("bgMusicPlayer was active, killing it");
			[m_bgMusicPlayer stop];
			[m_bgMusicPlayer release];
 			 m_bgMusicPlayer = NULL;
 			 
 		
		}
		
		 	m_lastMusicFileName = "";
			m_bLastMusicLooping = false;
			m_lastMusicID = AUDIO_HANDLE_BLANK;
			m_bDisabledMusicRecently = false;
	
}


void AudioObjectOS::Unload()
	{
		if (m_id != AUDIO_HANDLE_BLANK)
		{
			AudioServicesDisposeSystemSoundID(m_id);
				m_id = AUDIO_HANDLE_BLANK;
		}
	}