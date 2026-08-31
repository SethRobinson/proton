//  ***************************************************************
//  TTSClient - Creation date: 08/31/2026
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2026 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//  Text to speech over HTTP.  POSTs a form (the text plus whatever fields you
//  set: voice, scene, language, seed...) to a TTS server and writes the audio
//  it answers with to a file you name, ready for AudioManager::Play.  Written
//  for a local generation box (POST /tts, fields text/voice/scene, reply body
//  = a WAV) but any endpoint that takes an application/x-www-form-urlencoded
//  POST and replies with the raw audio bytes plus a Content-Length works.
//
//  Sibling of LLMClient: same NetHTTP plumbing, polled from Update(), results
//  arrive through boost::signals2 signals.
//
//  Usage:
//    m_tts.Setup("hal.local", 8899, "tts");
//    m_tts.SetField("voice", "belinda");
//    m_tts.SetField("scene", "calm narrator reading a novel, warm and measured");
//    m_tts.m_sig_ready.connect(1, boost::bind(&MyClass::OnSpeechReady, this, _1));
//    m_tts.m_sig_error.connect(1, boost::bind(&MyClass::OnSpeechError, this, _1));
//    m_tts.Speak("You are standing in an open field.", "speech_0.wav");
//    ... every frame:  m_tts.Update();
//    ... in OnSpeechReady:
//    GetAudioManager()->Play(pVList->Get(0).GetString(), false, false, false);
//
//  m_sig_ready: Get(0) = audio file path, Get(1) = request id (uint32),
//  Get(2) = the text that was spoken.
//  m_sig_error: Get(0) = error string, Get(1) = request id (uint32).
//
//  One request in flight per client, plus ONE pending slot: Speak() while
//  busy parks the line there, a newer Speak() replaces it (latest wins), and
//  when the in-flight request completes its audio is dropped, unfired, if
//  something newer is waiting.  That is what a talking character wants (say
//  the newest thing, never work through a backlog of stale lines); if every
//  line must be heard, wait for IsBusy() to clear before the next Speak().
//
//  Notes/limits:
//  - Needs shared/Network/NetHTTP.cpp, NetSocket.cpp, NetUtils.cpp in the
//    project (same as LLMClient; Ws2_32.lib on Windows).
//  - Plain HTTP, no TLS.  The whole reply is held in memory before the file
//    is written (a paragraph of 24 kHz speech is around a megabyte).
//  - Generation is synchronous on the server, so nothing arrives until the
//    audio is done: the idle timeout (default 90 s) is the cap on that wait.
//    A server that is down but whose host resolves also costs that long per
//    attempt (a refused connection doesn't read as a disconnect in NetSocket),
//    so probe the server first if that matters to you.
//  - The reply is checked with ValidateAudio: an HTTP error status, or a body
//    that doesn't start with a known audio signature (RIFF/WAV, OggS, fLaC,
//    ID3, an MP3 frame), or a WAV shorter than its own header claims, is
//    reported through m_sig_error with the start of the body.
//  - Playing a rewritten file under a name the AudioManager already cached
//    replays the OLD audio: use a fresh name per line, or call
//    AudioManager::DeleteSoundObjectByFileName before reusing one.

#ifndef TTSClient_h__
#define TTSClient_h__

#include <string>
#include <map>
#include "util/Variant.h" //also brings in boost::signals2
#include "Network/NetHTTP.h"

class TTSClient : public boost::signals2::trackable
{
public:

	TTSClient();

	//serverName is a bare host ("hal.local"), apiPath has no leading slash ("tts")
	void Setup(const std::string &serverName, int port, const std::string &apiPath);

	//form fields sent with every request besides the text (voice, scene,
	//seed, format...). Setting a field to "" removes it
	void SetField(const std::string &name, const std::string &value);
	void ClearFields() { m_fields.clear(); }
	void SetTextFieldName(const std::string &name) { m_textField = name; } //default "text"

	void SetTimeoutMS(int ms) { m_timeoutMS = ms; } //per attempt: give up if the server stays silent this long
	void SetMaxRetries(int n) { m_maxRetries = n; } //extra attempts after a transport failure (default 1)

	//queues text; the audio lands in outFile (overwritten). Returns the
	//request id the signals will carry, 0 if text/outFile are empty or Setup
	//wasn't called. See the header comment for the latest-wins queueing
	uint32 Speak(const std::string &text, const std::string &outFile);

	void Update(); //poll every frame
	bool IsBusy() const { return m_bBusy || m_bRetryPending || m_bHasPending; } //anything generating or waiting to
	bool IsInFlight() const { return m_bBusy; } //a request is on the wire right now
	void Abort(); //drop the in-flight request and anything pending, go idle

	//stats from the most recent finished line
	int GetLastReplyMS() const { return m_lastReplyMS; }
	int GetLastAudioBytes() const { return m_lastAudioBytes; }
	int GetInFlightMS() const; //ms the current request has been running, 0 if idle

	//the reply-body check described above; public so tests can feed it buffers.
	//False with a reason in errOut
	static bool ValidateAudio(const uint8 *pData, int len, std::string &errOut);

	boost::signals2::signal<void (VariantList*)> m_sig_ready; //Get(0) = file path, Get(1) = request id, Get(2) = text
	boost::signals2::signal<void (VariantList*)> m_sig_error; //Get(0) = error string, Get(1) = request id

private:

	struct Request
	{
		uint32 id = 0;
		std::string text;
		std::string outFile;
	};

	void StartRequest();
	void FinishRequest(); //in-flight is done (either way): reset, promote the pending line if any
	void HandleFailure(const std::string &errorMsg);
	static std::string BodyPreview(const uint8 *pData, int len, int maxChars);
	static bool WriteFile(const std::string &path, const uint8 *pData, int len);

	std::string m_serverName;
	int m_port = 8899;
	std::string m_apiPath;
	std::map<std::string, std::string> m_fields;
	std::string m_textField = "text";
	int m_timeoutMS = 90 * 1000;
	int m_maxRetries = 1;

	NetHTTP m_netHTTP;
	Request m_current;  //what's on the wire (or retrying)
	Request m_pending;  //the newest line waiting for its turn
	bool m_bBusy = false;
	bool m_bHasPending = false;
	bool m_bRetryPending = false;
	int m_attempt = 0;
	unsigned int m_retryAtTick = 0;
	unsigned int m_requestStartTick = 0;
	uint32 m_nextID = 1;

	int m_lastReplyMS = 0;
	int m_lastAudioBytes = 0;
};

#endif // TTSClient_h__
