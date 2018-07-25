#ifndef RANDOMAUDIOPLAYERCOMPONENT_H
#define RANDOMAUDIOPLAYERCOMPONENT_H

#include "Entity/Component.h"

/**
 * A component that plays audio files randomly.
 *
 * The name of the component is initially set to "RandomAudioPlayer".
 *
 * The component is given a list of audio file names where it chooses one at a time
 * randomly and plays it. The interval of the playing can be adjusted with minimum
 * and maximum boundaries. The component doesn't play the same audio file twice in
 * a row (given that there are more than one sound files to be played).
 *
 * \note When the component is destroyed it does not stop any currently playing sounds.
 *
 * The following named variants are used inside the component itself:
 * - <b>"audioFiles" (string):</b> a list of audio file names separated with colons (:).
 *   The format for the file names is the same what \c AudioManager::Play() accepts.
 *   This component doesn't check if the files are found nor whether the audio player
 *   manages to play them successfully. By default the list is empty so no sounds are played.
 * - <b>"minDelayBetweenPlays" (uint32):</b> sets the minimum delay between the playing
 *   of the sounds in milliseconds. Default is 1000ms (1 second).
 * - <b>"maxDelayBetweenPlays" (uint32):</b> sets the maximum delay between the playing
 *   of the sounds in milliseconds. Default is 2000ms (2 seconds).
 * - <b>"disabled" (uint32):</b> if 0, the component is enabled and it plays sounds.
 *   All other values disable the component. The default is 0, so the component is enabled.
 */
class RandomAudioPlayerComponent : public EntityComponent {
public:
	RandomAudioPlayerComponent();
	virtual ~RandomAudioPlayerComponent();

	virtual void OnAdd(Entity *pEnt);

private:
	uint32 *m_disabled;
	std::vector<std::string> m_audioFiles;
	uint32 *m_minDelayBetweenPlays;
	uint32 *m_maxDelayBetweenPlays;

	std::string m_lastPlayedAudioFile;

	void updateAudioFiles(Variant *var);
	void scheduleNewPlay(Variant *unused);
	void playRandomAudio(VariantList *unused);

};

#endif // RANDOMAUDIOPLAYERCOMPONENT_H
