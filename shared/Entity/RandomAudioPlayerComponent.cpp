#include "RandomAudioPlayerComponent.h"

#include "Manager/MessageManager.h"
#include "util/MiscUtils.h"

#include "BaseApp.h"

RandomAudioPlayerComponent::RandomAudioPlayerComponent() {
	SetName("RandomAudioPlayer");
}

RandomAudioPlayerComponent::~RandomAudioPlayerComponent() {
}

void RandomAudioPlayerComponent::OnAdd(Entity *pEnt) {
	EntityComponent::OnAdd(pEnt);

	m_disabled = &GetVarWithDefault("disabled", uint32(0))->GetUINT32();
	updateAudioFiles(GetVarWithDefault("audioFiles", Variant("")));
	m_minDelayBetweenPlays = &GetVarWithDefault("minDelayBetweenPlays", uint32(1000))->GetUINT32();
	m_maxDelayBetweenPlays = &GetVarWithDefault("maxDelayBetweenPlays", uint32(2000))->GetUINT32();

	GetVar("audioFiles")->GetSigOnChanged()->connect(boost::bind(&RandomAudioPlayerComponent::updateAudioFiles, this, _1));
	GetVar("minDelayBetweenPlays")->GetSigOnChanged()->connect(boost::bind(&RandomAudioPlayerComponent::scheduleNewPlay, this, _1));
	GetVar("maxDelayBetweenPlays")->GetSigOnChanged()->connect(boost::bind(&RandomAudioPlayerComponent::scheduleNewPlay, this, _1));
	GetVar("disabled")->GetSigOnChanged()->connect(boost::bind(&RandomAudioPlayerComponent::scheduleNewPlay, this, _1));

	GetFunction("PlayRandomAudio")->sig_function.connect(boost::bind(&RandomAudioPlayerComponent::playRandomAudio, this, _1));
}

void RandomAudioPlayerComponent::updateAudioFiles(Variant *var) {
	m_audioFiles.clear();

	m_audioFiles = StringTokenize(var->GetString(), ":");
}

void RandomAudioPlayerComponent::scheduleNewPlay(Variant *unused) {
	MessageManager* mm = GetMessageManager();

	mm->DeleteMessagesToComponent(this);

	if (!*m_disabled) {
		mm->CallComponentFunction(this, RandomRange(*m_minDelayBetweenPlays, *m_maxDelayBetweenPlays), "PlayRandomAudio");
	}
}

void RandomAudioPlayerComponent::playRandomAudio(VariantList *unused) {
	if (!*m_disabled && !m_audioFiles.empty()) {
		string sound;
		do {
			sound = m_audioFiles.at(Random(m_audioFiles.size()));
		} while (m_audioFiles.size() > 1 && sound == m_lastPlayedAudioFile);
		m_lastPlayedAudioFile = sound;

		GetAudioManager()->Play(sound);
	}

	scheduleNewPlay(NULL);
}
