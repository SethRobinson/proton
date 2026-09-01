//  ***************************************************************
//  LLMClient - Creation date: 08/31/2026
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2026 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//  A minimal client for OpenAI-compatible chat completion servers (llama.cpp,
//  vLLM, LM Studio, the real OpenAI API over a proxy, etc).
//
//  LLMConversation holds the message history and builds the request JSON;
//  LLMClient owns a NetHTTP and runs one request at a time, polled from your
//  app's Update().  Completion/failure is signaled boost::signals2 style.
//
//  Usage:
//    m_llm.Setup("hal.local", 8000, "v1/chat/completions", "Qwen3.8-Flash-Next");
//    m_llm.m_sig_response.connect(1, boost::bind(&MyClass::OnLLMReply, this, _1));
//    m_llm.m_sig_error.connect(1, boost::bind(&MyClass::OnLLMError, this, _1));
//    m_convo.SetSystemPrompt("You are...");
//    m_convo.AddUserMessage("Hello!");
//    m_llm.SendAsync(m_convo);
//    ... every frame:  m_llm.Update();
//
//  m_sig_response: Get(0) = assistant reply text (add it back to your
//  conversation with AddAssistantMessage if you want multi-turn memory).
//  m_sig_error: Get(0) = error string, Get(1) = uint32 retries left (fires
//  once per failed attempt; 0 means the client gave up and is idle again).
//
//  Streaming (opt-in, SetStreaming(true)): the request asks for an SSE
//  stream and m_sig_delta fires as fragments arrive (Get(0) = the text,
//  Get(1) = uint32 eLLMDeltaKind: the reply, or the model's reasoning).
//  m_sig_response still fires at the end with the whole reply, so handlers
//  written for the non-streamed case keep working.  GetStreamedReasoning()
//  and GetStreamedContent() hold the text so far while a request is in
//  flight; GetLastReasoning() is the reasoning of the last completed reply
//  (captured for non-streamed replies too, when the server sends it).
//
//  Notes/limits:
//  - Add shared/Network/NetHTTP.cpp, NetSocket.cpp, NetUtils.cpp and
//    shared/util/cJSON.c to your project (see docs/ai-llm.md).
//  - The default socket backend is plain HTTP (no TLS).  Fine for LAN
//    servers.  Streaming needs the socket backend (NetHTTP stream mode);
//    the html5/libcurl backends deliver the reply whole at the end.
//  - One request in flight per LLMClient; SendAsync returns false if busy.

#ifndef LLMClient_h__
#define LLMClient_h__

#include <string>
#include <vector>
#include "util/Variant.h" //also brings in boost::signals2
#include "Network/NetHTTP.h"

struct LLMMessage
{
	LLMMessage() {}
	LLMMessage(const std::string &role, const std::string &content) : m_role(role), m_content(content) {}

	std::string m_role;    //"user", "assistant" (system is handled separately)
	std::string m_content;
};

class LLMConversation
{
public:

	void SetSystemPrompt(const std::string &prompt) { m_systemPrompt = prompt; }
	const std::string & GetSystemPrompt() const { return m_systemPrompt; }

	void AddUserMessage(const std::string &text) { m_messages.push_back(LLMMessage("user", text)); }
	void AddAssistantMessage(const std::string &text) { m_messages.push_back(LLMMessage("assistant", text)); }
	void RemoveLastMessage() { if (!m_messages.empty()) m_messages.pop_back(); } //e.g. a user turn whose request got aborted
	void Clear() { m_messages.clear(); }

	//bound the history: keep only the last maxPairs user/assistant exchanges
	//(the system prompt is separate and always kept)
	void TrimToLastExchanges(int maxPairs);

	//drops the oldest count messages (e.g. turns an app has summarized
	//elsewhere), then any leading non-user messages so the history still
	//starts on a user turn
	void RemoveOldestMessages(int count);

	//system prompt plus every message, in chars: a cheap token estimate
	//(English runs about 4 chars a token)
	int GetTotalChars() const;

	const std::vector<LLMMessage> & GetMessages() const { return m_messages; }

	//the full request body for a /v1/chat/completions POST. maxTokens <= 0
	//sends no max_tokens at all (the model stops when it's done, within the
	//server's own limit). bStream adds "stream":true plus
	//stream_options.include_usage (so the usage stats still arrive, as the
	//stream's final chunk)
	std::string BuildChatCompletionJSON(const std::string &model, float temperature, int maxTokens, bool bStream = false) const;

private:

	std::string m_systemPrompt;
	std::vector<LLMMessage> m_messages;
};

enum eLLMDeltaKind
{
	LLM_DELTA_CONTENT = 0,  //the reply
	LLM_DELTA_REASONING = 1 //the model's thinking (reasoning models with thinking on)
};

//Incremental parser for an OpenAI-compatible SSE chat completion stream.
//Pure: feed it "the body so far" (it keeps its own consumed offset) and
//drain the deltas. Handles delta.content, delta.reasoning /
//delta.reasoning_content, a leading <think>...</think> block inline in the
//content (the tags may be split across deltas), the usage chunk,
//finish_reason, "data: [DONE]", error events, and a body that turns out not
//to be SSE at all (a JSON error object, or a server that ignored stream:true)
class LLMStreamParser
{
public:

	struct Delta
	{
		eLLMDeltaKind kind;
		std::string text;
	};

	enum eBodyKind
	{
		BODY_UNKNOWN, //nothing decisive received yet
		BODY_SSE,     //"data:" lines
		BODY_OTHER    //not a stream: parse the whole body as a normal reply
	};

	void Reset();
	//bFinal: the body is complete, also take an unterminated last line and
	//flush anything the <think> router was holding back
	void Feed(const std::string &bodySoFar, bool bFinal);
	std::vector<Delta> & GetPendingDeltas() { return m_pending; } //drain and clear after each Feed

	eBodyKind GetBodyKind() const { return m_bodyKind; }
	bool IsDone() const { return m_bDone; } //saw data: [DONE]
	int GetChunkCount() const { return m_chunks; }
	const std::string & GetError() const { return m_error; } //non-empty once an error event arrived
	const std::string & GetFinishReason() const { return m_finishReason; } //"stop", "length", ... ("" if not sent)
	int GetPromptTokens() const { return m_promptTokens; }
	int GetCompletionTokens() const { return m_completionTokens; }
	const std::string & GetContent() const { return m_content; }     //accumulated
	const std::string & GetReasoning() const { return m_reasoning; } //accumulated

private:

	void OnLine(const std::string &line);
	void OnChunk(const std::string &json);
	void RouteContent(const std::string &text); //the <think> state machine
	void FlushThink();
	void Emit(eLLMDeltaKind kind, const std::string &text);

	enum eThinkState { THINK_UNDECIDED, THINK_INSIDE, THINK_PLAIN };

	size_t m_consumed = 0;
	eBodyKind m_bodyKind = BODY_UNKNOWN;
	bool m_bDone = false;
	int m_chunks = 0;
	std::string m_error;
	std::string m_finishReason;
	std::string m_content;
	std::string m_reasoning;
	std::string m_thinkPending; //content held back until we know whether it opens a <think> tag
	eThinkState m_thinkState = THINK_UNDECIDED;
	int m_promptTokens = 0;
	int m_completionTokens = 0;
	std::vector<Delta> m_pending;
};

class LLMClient : public boost::signals2::trackable
{
public:

	LLMClient();

	//serverName is a bare host ("hal.local"), apiPath has no leading slash
	//("v1/chat/completions"), model is the server's model id
	void Setup(const std::string &serverName, int port, const std::string &apiPath, const std::string &model);
	//maxTokens <= 0 = no cap (no max_tokens in the request; the server's limit applies)
	void SetParms(float temperature, int maxTokens, int maxRetries);
	//idle cap per attempt: give up if nothing arrives for this long. A stream
	//resets it with every fragment, so with streaming on it only has to cover
	//the wait for the first token
	void SetTimeoutMS(int ms) { m_timeoutMS = ms; }

	//a JSON object whose fields get merged into every request body; for
	//server-specific extras, e.g. vLLM/Qwen:
	//  {"chat_template_kwargs":{"enable_thinking":false}}
	void SetExtraBodyJSON(const std::string &jsonObject) { m_extraBodyJSON = jsonObject; }

	//opt-in SSE streaming (see the header comment). Takes effect at the next
	//SendAsync
	void SetStreaming(bool bStream) { m_bStreaming = bStream; }
	bool GetStreaming() const { return m_bStreaming; }

	//mirror every request/response/error body to a text file, timestamped
	//("" disables). Truncates any existing file unless bAppend.
	void SetLogFile(const std::string &path, bool bAppend = false);
	const std::string & GetLogFile() const { return m_logPath; }
	//a tag on this client's log entries ("[summary] REQUEST ..."), for apps
	//that mirror several clients into one file
	void SetLogLabel(const std::string &label) { m_logLabel = label; }

	//stats from the most recent completed reply (0 until one finishes); the
	//token counts come from the server's "usage" object when present
	int GetLastPromptTokens() const { return m_lastPromptTokens; }
	int GetLastCompletionTokens() const { return m_lastCompletionTokens; }
	int GetLastReplyMS() const { return m_lastReplyMS; }
	float GetLastTPS() const; //completion tokens per second of the last reply
	int GetLastRequestBytes() const { return m_lastRequestBytes; } //POST body size
	int GetInFlightMS() const; //ms the current request has been running, 0 if idle
	int GetLastFirstTokenMS() const { return m_lastFirstTokenMS; } //streamed replies: ms until the first fragment (0 otherwise)
	const std::string & GetLastFinishReason() const { return m_lastFinishReason; } //"stop", "length"... ("" if the server didn't say)

	//the model's reasoning ("thinking") text: what the server sent alongside
	//the last completed reply ("" if none), and the live text of the request
	//in flight when streaming (cleared when an attempt starts; still
	//readable after Abort until the next SendAsync)
	const std::string & GetLastReasoning() const { return m_lastReasoning; }
	const std::string & GetStreamedReasoning() const { return m_streamReasoning; }
	const std::string & GetStreamedContent() const { return m_streamContent; }
	int GetStreamChunkCount() const { return m_streamChunks; }

	//snapshots the conversation into a request; false if one is in flight
	bool SendAsync(const LLMConversation &convo);

	void Update(); //poll every frame
	bool IsBusy() const { return m_bBusy || m_bRetryPending; }
	void Abort();  //cancel in-flight request and pending retries, go idle

	boost::signals2::signal<void (VariantList*)> m_sig_response; //Get(0) = assistant text
	boost::signals2::signal<void (VariantList*)> m_sig_error;    //Get(0) = error string, Get(1) = uint32 retries left
	boost::signals2::signal<void (VariantList*)> m_sig_delta;    //streaming only: Get(0) = text fragment, Get(1) = uint32 eLLMDeltaKind

private:

	void StartRequest();
	void HandleFailure(const std::string &errorMsg);
	//the non-streamed path: parse the whole body, log it, deliver or fail
	void DeliverWholeReply(const char *pRaw);
	//streaming: feed the parser, fire m_sig_delta for what's new
	void PumpStream(const std::string &body, bool bFinal);
	void FinishStreamedReply();
	//pulls choices[0].message.content out of the reply; on failure returns "" and sets errOut.
	//Also captures usage stats, finish_reason and any reasoning text.
	std::string ParseAssistantContent(const char *pJSON, std::string &errOut);
	void ReadUsage(void *pRootJSON); //m_lastPromptTokens/m_lastCompletionTokens from a reply's "usage"
	void AppendToLog(const std::string &header, const std::string &body);

	std::string m_serverName;
	int m_port = 8000;
	std::string m_apiPath;
	std::string m_model;

	float m_temperature = 0.7f;
	int m_maxTokens = 500;
	int m_maxRetries = 3;
	int m_timeoutMS = 15 * 1000;

	std::string MergeExtraBody(const std::string &body) const;

	NetHTTP m_netHTTP;
	std::string m_extraBodyJSON;
	std::string m_pendingBody; //the request being sent (kept for retries)
	bool m_bBusy = false;
	bool m_bRetryPending = false;
	int m_attempt = 0;
	unsigned int m_retryAtTick = 0;

	//streaming
	bool m_bStreaming = false;
	bool m_bStreamRequest = false; //what the request in flight asked for (snapshot of m_bStreaming at SendAsync)
	LLMStreamParser m_parser;
	std::string m_streamContent;
	std::string m_streamReasoning;
	int m_streamChunks = 0;
	bool m_bGotFirstToken = false;

	std::string m_logPath;
	std::string m_logLabel;
	unsigned int m_requestStartTick = 0;
	int m_lastRequestBytes = 0;
	int m_lastReplyMS = 0;
	int m_lastFirstTokenMS = 0;
	int m_lastPromptTokens = 0;
	int m_lastCompletionTokens = 0;
	std::string m_lastReasoning;
	std::string m_lastFinishReason;
};

#endif // LLMClient_h__
