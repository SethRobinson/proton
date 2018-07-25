//  ***************************************************************
//  AudioManager - Creation date: 05/25/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AudioManager_h__
#define AudioManager_h__


typedef uintptr_t AudioHandle;

#define AUDIO_HANDLE_BLANK 0

/**
 * Audio manager handles the playing of audio.
 *
 * The \c AudioManager plays two kinds of audio files: music and sounds.
 * Music is meant for background music and such audio that plays possibly
 * for a long time. Sounds are meant for audio that usually play only for
 * a short time like sound effects. There can always be a maximum of one
 * music file playing simultaneously. There is no hard limit on how many
 * sounds can be playing simultaneously but the underlaying audio backend
 * may put some limit to this.
 *
 * The music and sound playing systems can be disabled and enabled
 * independently from each other.
 *
 * The actual \c AudioManager class works as a general interface for audio
 * managers. The \c AudioManager class by itself produces no sound
 * whatsoever (so it can be used to disable sound completely if instantiated
 * directly). In order to produce any audible sound use one of the backend
 * specific audio managers.
 *
 * \see AudioManagerAndroid, AudioManagerAudiere, AudioManagerBBX,
 * AudioManagerDenshion, AudioManagerFMOD, AudioManagerOS, AudioManagerSDL
 */
class AudioManager
{
public:
	AudioManager();
	virtual ~AudioManager();

	virtual bool Init() {return true;}
	virtual void Kill() {}

	/**
	 * Plays an audio specified by a file name.
	 *
	 * Returns the audio handle for the playing sound if the playing started successfully.
	 * Returns \c AUDIO_HANDLE_BLANK if the playing of the audio didn't start successfully.
	 * Also returns \c AUDIO_HANDLE_BLANK if requesting to play a sound file and sound is
	 * currently disabled or requesting to play music and music is currently disabled.
	 *
	 * \param fName the file name of the audio to play.
	 * \param bLooping whether the sound should start again when it ends, i.e. loop.
	 * \param bIsMusic specifies if this sound is the music track.
	 * \param bAddBasePath adds the base path to the file name. See \c GetBaseAppPath().
	 * \param bForceStreaming streams the audio directly from the file and doesn't load
	 *        it into memory. This flag might not be supported by all audio backends.
	 * \return The audio handle for the playing sound if the playing started successfully.
	 *         Returns \c AUDIO_HANDLE_BLANK if the playing of the audio didn't start.
	 */
	virtual AudioHandle Play(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false);

	virtual AudioHandle PlayWithAVPlayer(string fName) { return Play(fName, false, false); } //also doesn't cache and uses AV player, useful for the TTS system or oneshots

	virtual void Stop(AudioHandle soundID) {}
	virtual void Preload(string fName, bool bLooping = false, bool bIsMusic = false, bool bAddBasePath = true, bool bForceStreaming = false){}
	/**
	 * Checks if playing of music is currently enabled.
	 */
	bool GetMusicEnabled() {return m_bMusicEnabled;}
	/**
	 * Enables or disables the playing of music.
	 *
	 * When the music system is disabled any currently playing music is stopped.
	 * When the music system is later enabled again the music file that was left
	 * playing is resumed from the location where it stopped.
	 */
	virtual void SetMusicEnabled(bool bNew) {m_bMusicEnabled = bNew;}
	/**
	 * Gets the name of the file that was last played as music.
	 */
	string GetLastMusicFileName() {return m_lastMusicFileName;}
	/**
	 * Gets whether the last music file that was played was requested to loop or not.
	 */
	bool GetLastMusicLooping() {return m_bLastMusicLooping;}
	/**
	 * Returns the \c AudioHandle of the previously played music track.
	 * The music might still be playing or it might have stopped.
	 *
	 * \note The proper way of checking if music is currently playing is to use
	 * code like this: <tt>audioManager->IsPlaying(audioManager->GetLastMusicID())</tt>
	 */
	AudioHandle GetLastMusicID() {return m_lastMusicID;}
	virtual void Vibrate(int duration = 300){ if (!m_bVibrationDisabled) {LogMsg("Vibrate!");} }
	void SetVibrateDisabled(bool bNew) {m_bVibrationDisabled = bNew;}

	/**
	 * Stops the currently playing music immediately.
	 */
	virtual void StopMusic();
	/**
	 * This method must be called periodically, e.g once every frame.
	 */
	virtual void Update() {}

	/**
	 * Fades out the currently playing music during \a duration milliseconds.
	 * If no music is playing then this method does nothing.
	 */
	virtual void FadeOutMusic(unsigned int duration = 1000) {StopMusic();}
	virtual void KillCachedSounds(bool bKillMusic, bool bKillLooping, int ignoreSoundsUsedInLastMS, int killSoundsLowerPriorityThanThis, bool bKillSoundsPlaying) {}
	virtual int GetMemoryUsed(){return 0;}
	virtual void SetFrequency(AudioHandle soundID, int freq) {}
	virtual void SetPan(AudioHandle soundID, float pan) {} //0 is normal stereo, -1 is all left, +1 is all right
	virtual bool IsPlaying(AudioHandle soundID) {return false;}
	virtual void SetVol(AudioHandle soundID, float vol) {} //using audio handle -1 adjusts global vol on some systems
	virtual void SetPriority(AudioHandle soundID, int priority) {}
	virtual uint32 GetPos( AudioHandle soundID ){return 0;}
	virtual void SetPos( AudioHandle soundID, uint32 posMS ){}
	virtual void SetDLS(string fName); //(fmod only) example, "dink/midi/TimGM6mbTiny.dls" - if not set, FMOD will try to use whatever the system has.
	virtual void SetMusicVol(float vol){}
	virtual float GetMusicVol() {return m_musicVol;}
	/**
	 * Checks if playing of sounds is currently enabled.
	 */
	bool GetSoundEnabled() {return m_bSoundEnabled;}
	/**
	 * Enables or disables the playing of sounds.
	 */
	virtual void SetSoundEnabled(bool bNew) {m_bSoundEnabled = bNew;}
	virtual void Suspend(){} //stop all audio, app when into background or something
	virtual void Resume(){} //restore audio that was stopped
	void SetMusicStreaming(bool bStreaming);  //default to TRUE, if false, we'll cache entire songs instead of streaming from disk (setting to false may be better for desktops, more mem)
	
	void ForceAudioExtension(const string extension); //ONLY implemented for FMOD and AudioSDL! if "ogg", all waves will actually be changed to oggs when finding the files.  Set to "" to disable
	string ModifiedFileName(string fName);
	bool PreferOGG() {return m_bPreferOGG;}
	void SetPreferOGG(bool bNew) {m_bPreferOGG = bNew;}
	void SetDefaultVol(float vol){m_defaultVol = vol;}
	virtual string GetAudioSystemName(); //will return "unknown" or "fmodstudio" for example, the subclass returns it if it cares
	virtual void ReinitForHTML5() {};

protected:
	
	string m_lastMusicFileName;
	bool m_bLastMusicLooping;
	bool m_bMusicEnabled;
	AudioHandle m_lastMusicID;
	bool m_bVibrationDisabled;
	string m_midiSoundBankFile; //only used by FMOD currently
	float m_musicVol; // 0 means none, 1 means full blast
	bool m_bSoundEnabled;
	bool m_bStreamMusic;
	string m_forcedAudioExtension;
	bool m_bPreferOGG; //will replace mp3's with oggs in filename texts
	float m_defaultVol; //non music will be played at this vol to start with
};

bool CheckIfOtherAudioIsPlaying(); //are they playing ipod stuff before the app was run?  Should call this before playing your own.

#endif // AudioManager_h__
