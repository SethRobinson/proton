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
//  Get(2) = the text that was spoken, Get(3) = generation time in ms
//  (uint32, request to reply), Get(4) = audio length in ms (uint32, read
//  from the WAV header, 0 if it couldn't be).
//  m_sig_error: Get(0) = error string, Get(1) = request id (uint32).
//
//  It is a request POOL: Speak() queues a line and returns its id, up to
//  SetMaxParallel() requests are on the wire at once (default 1), the rest
//  wait in a priority queue (higher priority first, FIFO within one).
//  Cancel(id) / CancelAll() drop queued lines and abort in-flight ones (the
//  server still finishes generating those, nothing can stop that); a
//  cancelled request fires no signal.  Per-request fields override the
//  client-wide ones, so one pool can serve several voices.
//
//  About SetMaxParallel: a server that serializes generation (hal does: three
//  concurrent short lines all came back together after 3x the time of one)
//  gains nothing from more than 1, and the FIRST reply arrives later since it
//  only comes back with the batch.  Raise it only for a server with several
//  workers.  To keep a long text flowing, split it into short chunks and
//  keep a couple queued: the first chunk plays while the rest generate.
//
//  Notes/limits:
//  - Needs shared/Network/NetHTTP.cpp, NetSocket.cpp, NetUtils.cpp in the
//    project (same as LLMClient; Ws2_32.lib on Windows).
//  - Plain HTTP, no TLS.  Each reply is held in memory before the file is
//    written (a paragraph of 24 kHz speech is around a megabyte).
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
#include <deque>
#include <vector>
#include "util/Variant.h" //also brings in boost::signals2
#include "Network/NetHTTP.h"

class TTSClient : public boost::signals2::trackable
{
public:

	typedef std::map<std::string, std::string> FieldMap;

	TTSClient();
	~TTSClient();

	//serverName is a bare host ("hal.local"), apiPath has no leading slash ("tts")
	void Setup(const std::string &serverName, int port, const std::string &apiPath);

	//form fields sent with every request besides the text (voice, scene,
	//seed, format...). Setting a field to "" removes it
	void SetField(const std::string &name, const std::string &value);
	void ClearFields() { m_fields.clear(); }
	void SetTextFieldName(const std::string &name) { m_textField = name; } //default "text"

	void SetTimeoutMS(int ms) { m_timeoutMS = ms; } //per attempt: give up if the server stays silent this long
	void SetMaxRetries(int n) { m_maxRetries = n; } //extra attempts after a transport failure (default 1)
	void SetMaxParallel(int n); //requests on the wire at once, default 1 (see the header comment before raising it)
	int GetMaxParallel() const { return m_maxParallel; }

	//queues text; the audio lands in outFile (overwritten). Returns the
	//request id the signals will carry, 0 if text/outFile are empty or Setup
	//wasn't called. Higher priority lines are sent before lower ones waiting
	//in the queue; per-request fields override the client-wide ones
	uint32 Speak(const std::string &text, const std::string &outFile, int priority = 0);
	uint32 Speak(const std::string &text, const std::string &outFile, const FieldMap &fields, int priority = 0);

	bool Cancel(uint32 id); //true if it was queued or in flight; no signal will fire for it
	void CancelAll();       //everything queued and in flight, go idle
	bool IsPending(uint32 id) const;  //queued or in flight
	bool IsInFlight(uint32 id) const; //on the wire right now

	void Update(); //poll every frame
	bool IsBusy() const { return GetInFlightCount() > 0 || !m_queue.empty(); }
	int GetInFlightCount() const;
	int GetQueuedCount() const { return (int)m_queue.size(); }

	//stats from the most recently finished line
	int GetLastReplyMS() const { return m_lastReplyMS; }
	int GetLastAudioBytes() const { return m_lastAudioBytes; }
	int GetLastAudioMS() const { return m_lastAudioMS; }

	//the reply-body check described above; public so tests can feed it buffers.
	//False with a reason in errOut
	static bool ValidateAudio(const uint8 *pData, int len, std::string &errOut);
	//length of a PCM WAV in ms from its header (fmt byte rate + data chunk
	//size), 0 if it isn't one we can read. A data chunk that claims more than
	//the buffer holds is clamped to what's there
	static int GetWavDurationMS(const uint8 *pData, int len);

	boost::signals2::signal<void (VariantList*)> m_sig_ready; //Get(0) = file path, Get(1) = request id, Get(2) = text, Get(3) = generation ms, Get(4) = audio ms
	boost::signals2::signal<void (VariantList*)> m_sig_error; //Get(0) = error string, Get(1) = request id

private:

	struct Request
	{
		uint32 id = 0;
		int priority = 0;
		std::string text;
		std::string outFile;
		FieldMap fields;
	};

	struct Worker
	{
		NetHTTP net;
		Request req;
		bool bBusy = false;         //a request is on the wire
		bool bRetryPending = false; //waiting out the backoff before resending req
		int attempt = 0;
		unsigned int startTick = 0;
		unsigned int retryAtTick = 0;
	};

	void Dispatch(); //hands queued lines to free workers
	void StartOnWorker(Worker &w);
	void UpdateWorker(Worker &w);
	void FreeWorker(Worker &w);
	void HandleFailure(Worker &w, const std::string &errorMsg);
	static std::string BodyPreview(const uint8 *pData, int len, int maxChars);
	static bool WriteFile(const std::string &path, const uint8 *pData, int len);

	std::string m_serverName;
	int m_port = 8899;
	std::string m_apiPath;
	FieldMap m_fields;
	std::string m_textField = "text";
	int m_timeoutMS = 90 * 1000;
	int m_maxRetries = 1;
	int m_maxParallel = 1;

	std::deque<Request> m_queue;    //waiting: higher priority first, FIFO within one
	std::vector<Worker*> m_workers; //grown on demand up to m_maxParallel
	uint32 m_nextID = 1;

	int m_lastReplyMS = 0;
	int m_lastAudioBytes = 0;
	int m_lastAudioMS = 0;
};

#endif // TTSClient_h__
