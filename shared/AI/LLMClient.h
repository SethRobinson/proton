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
//  Notes/limits:
//  - Add shared/Network/NetHTTP.cpp, NetSocket.cpp, NetUtils.cpp and
//    shared/util/cJSON.c to your project (see docs/ai-llm.md).
//  - The default socket backend is plain HTTP (no TLS) and non-streaming:
//    the whole reply arrives at once.  Fine for LAN servers.
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
	void Clear() { m_messages.clear(); }

	//bound the history: keep only the last maxPairs user/assistant exchanges
	//(the system prompt is separate and always kept)
	void TrimToLastExchanges(int maxPairs);

	const std::vector<LLMMessage> & GetMessages() const { return m_messages; }

	//the full request body for a /v1/chat/completions POST
	std::string BuildChatCompletionJSON(const std::string &model, float temperature, int maxTokens) const;

private:

	std::string m_systemPrompt;
	std::vector<LLMMessage> m_messages;
};

class LLMClient : public boost::signals2::trackable
{
public:

	LLMClient();

	//serverName is a bare host ("hal.local"), apiPath has no leading slash
	//("v1/chat/completions"), model is the server's model id
	void Setup(const std::string &serverName, int port, const std::string &apiPath, const std::string &model);
	void SetParms(float temperature, int maxTokens, int maxRetries);
	void SetTimeoutMS(int ms) { m_timeoutMS = ms; } //per-attempt cap on waiting for the reply

	//snapshots the conversation into a request; false if one is in flight
	bool SendAsync(const LLMConversation &convo);

	void Update(); //poll every frame
	bool IsBusy() const { return m_bBusy || m_bRetryPending; }
	void Abort();  //cancel in-flight request and pending retries, go idle

	boost::signals2::signal<void (VariantList*)> m_sig_response; //Get(0) = assistant text
	boost::signals2::signal<void (VariantList*)> m_sig_error;    //Get(0) = error string, Get(1) = uint32 retries left

private:

	void StartRequest();
	void HandleFailure(const std::string &errorMsg);
	//pulls choices[0].message.content out of the reply; on failure returns "" and sets errOut
	std::string ParseAssistantContent(const char *pJSON, std::string &errOut);

	std::string m_serverName;
	int m_port = 8000;
	std::string m_apiPath;
	std::string m_model;

	float m_temperature = 0.7f;
	int m_maxTokens = 500;
	int m_maxRetries = 3;
	int m_timeoutMS = 15 * 1000;

	NetHTTP m_netHTTP;
	std::string m_pendingBody; //the request being sent (kept for retries)
	bool m_bBusy = false;
	bool m_bRetryPending = false;
	int m_attempt = 0;
	unsigned int m_retryAtTick = 0;
};

#endif // LLMClient_h__
