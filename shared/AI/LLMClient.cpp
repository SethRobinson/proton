#include "PlatformPrecomp.h"
#include "LLMClient.h"
#include "util/cJSON.h"
#include <time.h>

static std::string LogTimestamp()
{
	time_t now = time(NULL);
	struct tm *pTM = localtime(&now);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", pTM);
	return buf;
}

void LLMConversation::TrimToLastExchanges(int maxPairs)
{
	int maxMessages = maxPairs * 2;
	if ((int)m_messages.size() <= maxMessages)
		return;

	m_messages.erase(m_messages.begin(), m_messages.end() - maxMessages);

	//don't start the window mid-exchange: the first kept message should be
	//from the user
	while (!m_messages.empty() && m_messages[0].m_role != "user")
		m_messages.erase(m_messages.begin());
}

std::string LLMConversation::BuildChatCompletionJSON(const std::string &model, float temperature, int maxTokens, bool bStream) const
{
	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, "model", model.c_str());
	cJSON_AddNumberToObject(pRoot, "temperature", temperature);
	cJSON_AddNumberToObject(pRoot, "max_tokens", maxTokens);

	if (bStream)
	{
		cJSON_AddTrueToObject(pRoot, "stream");
		//the usage object only comes with a stream when asked for (a final
		//chunk with no choices)
		cJSON *pOptions = cJSON_AddObjectToObject(pRoot, "stream_options");
		cJSON_AddTrueToObject(pOptions, "include_usage");
	}

	cJSON *pMsgs = cJSON_AddArrayToObject(pRoot, "messages");

	if (!m_systemPrompt.empty())
	{
		cJSON *pMsg = cJSON_CreateObject();
		cJSON_AddStringToObject(pMsg, "role", "system");
		cJSON_AddStringToObject(pMsg, "content", m_systemPrompt.c_str());
		cJSON_AddItemToArray(pMsgs, pMsg);
	}

	for (unsigned int i = 0; i < m_messages.size(); i++)
	{
		cJSON *pMsg = cJSON_CreateObject();
		cJSON_AddStringToObject(pMsg, "role", m_messages[i].m_role.c_str());
		cJSON_AddStringToObject(pMsg, "content", m_messages[i].m_content.c_str());
		cJSON_AddItemToArray(pMsgs, pMsg);
	}

	char *pText = cJSON_PrintUnformatted(pRoot);
	std::string json = pText ? pText : "";
	if (pText) cJSON_free(pText);
	cJSON_Delete(pRoot);
	return json;
}

//*** LLMStreamParser

static const char C_THINK_OPEN[] = "<think>";
static const char C_THINK_CLOSE[] = "</think>";

void LLMStreamParser::Reset()
{
	m_consumed = 0;
	m_bodyKind = BODY_UNKNOWN;
	m_bDone = false;
	m_chunks = 0;
	m_error.clear();
	m_finishReason.clear();
	m_content.clear();
	m_reasoning.clear();
	m_thinkPending.clear();
	m_thinkState = THINK_UNDECIDED;
	m_promptTokens = 0;
	m_completionTokens = 0;
	m_pending.clear();
}

void LLMStreamParser::Feed(const std::string &body, bool bFinal)
{
	//complete lines only, from where we left off
	while (m_consumed < body.length())
	{
		size_t nl = body.find('\n', m_consumed);
		if (nl == std::string::npos)
		{
			if (!bFinal)
				break; //wait for the rest of the line
			nl = body.length();
		}
		std::string line = body.substr(m_consumed, nl - m_consumed);
		m_consumed = (nl < body.length()) ? nl + 1 : nl;
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		OnLine(line);
	}

	if (bFinal)
		FlushThink();
}

void LLMStreamParser::OnLine(const std::string &line)
{
	if (m_bodyKind == BODY_OTHER)
		return; //the whole body gets parsed as a normal reply by the caller

	if (m_bodyKind == BODY_UNKNOWN)
	{
		if (line.empty())
			return;
		bool bSSE = line.compare(0, 5, "data:") == 0 || line.compare(0, 6, "event:") == 0
			|| line.compare(0, 3, "id:") == 0 || line[0] == ':';
		m_bodyKind = bSSE ? BODY_SSE : BODY_OTHER;
		if (!bSSE)
			return;
	}

	if (line.compare(0, 5, "data:") != 0)
		return; //event:/id:/retry:/comment lines carry nothing we need

	size_t start = 5;
	while (start < line.length() && line[start] == ' ')
		start++;
	std::string payload = line.substr(start);
	if (payload.empty())
		return;
	if (payload == "[DONE]")
	{
		m_bDone = true;
		return;
	}
	OnChunk(payload);
}

void LLMStreamParser::OnChunk(const std::string &json)
{
	cJSON *pRoot = cJSON_Parse(json.c_str());
	if (!pRoot)
	{
		std::string preview = json.length() > 200 ? json.substr(0, 200) : json;
		LogMsg("LLMStreamParser: skipping a chunk that isn't JSON: %s", preview.c_str());
		return;
	}
	m_chunks++;

	//error events: OpenAI's {"error":{"message":...}}, vLLM's {"object":"error","message":...}
	cJSON *pError = cJSON_GetObjectItem(pRoot, "error");
	cJSON *pTopMsg = cJSON_GetObjectItem(pRoot, "message");
	if (pError && !cJSON_IsNull(pError))
	{
		cJSON *pMsg = cJSON_GetObjectItem(pError, "message");
		m_error = (pMsg && cJSON_IsString(pMsg)) ? pMsg->valuestring : "server returned an error";
	}
	else if (pTopMsg && cJSON_IsString(pTopMsg) && !cJSON_GetObjectItem(pRoot, "choices"))
	{
		m_error = pTopMsg->valuestring;
	}

	//"usage": null on every chunk but the last, with include_usage
	cJSON *pUsage = cJSON_GetObjectItem(pRoot, "usage");
	if (pUsage && !cJSON_IsNull(pUsage))
	{
		cJSON *pPrompt = cJSON_GetObjectItem(pUsage, "prompt_tokens");
		cJSON *pCompletion = cJSON_GetObjectItem(pUsage, "completion_tokens");
		if (pPrompt && cJSON_IsNumber(pPrompt)) m_promptTokens = pPrompt->valueint;
		if (pCompletion && cJSON_IsNumber(pCompletion)) m_completionTokens = pCompletion->valueint;
	}

	cJSON *pChoices = cJSON_GetObjectItem(pRoot, "choices");
	cJSON *pChoice = pChoices ? cJSON_GetArrayItem(pChoices, 0) : NULL;
	if (pChoice)
	{
		cJSON *pFinish = cJSON_GetObjectItem(pChoice, "finish_reason");
		if (pFinish && cJSON_IsString(pFinish))
			m_finishReason = pFinish->valuestring;

		cJSON *pDelta = cJSON_GetObjectItem(pChoice, "delta");
		if (pDelta)
		{
			//the reasoning parser's field: "reasoning" (newer vLLM) or "reasoning_content" (older, DeepSeek)
			cJSON *pReasoning = cJSON_GetObjectItem(pDelta, "reasoning");
			if (!pReasoning || !cJSON_IsString(pReasoning))
				pReasoning = cJSON_GetObjectItem(pDelta, "reasoning_content");
			if (pReasoning && cJSON_IsString(pReasoning) && pReasoning->valuestring[0])
				Emit(LLM_DELTA_REASONING, pReasoning->valuestring);

			cJSON *pContent = cJSON_GetObjectItem(pDelta, "content");
			if (pContent && cJSON_IsString(pContent) && pContent->valuestring[0])
				RouteContent(pContent->valuestring);
		}
	}

	cJSON_Delete(pRoot);
}

//content deltas: a reply that opens with <think> (no reasoning parser on
//the server) is reasoning until </think>. The tags can arrive split across
//deltas, so only the bytes that could still be part of a tag are held back
void LLMStreamParser::RouteContent(const std::string &text)
{
	m_thinkPending += text;
	const size_t openLen = strlen(C_THINK_OPEN);
	const size_t closeLen = strlen(C_THINK_CLOSE);

	while (true)
	{
		if (m_thinkState == THINK_UNDECIDED)
		{
			//leading whitespace before the tag is fine
			size_t i = 0;
			while (i < m_thinkPending.length() && isspace((unsigned char)m_thinkPending[i]))
				i++;
			size_t have = m_thinkPending.length() - i;
			size_t cmp = (have < openLen) ? have : openLen;
			if (cmp > 0 && m_thinkPending.compare(i, cmp, C_THINK_OPEN, cmp) != 0)
			{
				m_thinkState = THINK_PLAIN;
				continue;
			}
			if (have < openLen)
				return; //"<thi": could still become the tag, hold it
			m_thinkPending.erase(0, i + openLen);
			m_thinkState = THINK_INSIDE;
			continue;
		}

		if (m_thinkState == THINK_INSIDE)
		{
			size_t end = m_thinkPending.find(C_THINK_CLOSE);
			if (end != std::string::npos)
			{
				Emit(LLM_DELTA_REASONING, m_thinkPending.substr(0, end));
				m_thinkPending.erase(0, end + closeLen);
				m_thinkState = THINK_PLAIN;
				continue;
			}
			//emit everything except a tail that could be the start of </think>
			size_t keep = 0;
			size_t maxK = (m_thinkPending.length() < closeLen - 1) ? m_thinkPending.length() : closeLen - 1;
			for (size_t k = maxK; k > 0; k--)
			{
				if (m_thinkPending.compare(m_thinkPending.length() - k, k, C_THINK_CLOSE, k) == 0)
				{
					keep = k;
					break;
				}
			}
			if (m_thinkPending.length() > keep)
			{
				Emit(LLM_DELTA_REASONING, m_thinkPending.substr(0, m_thinkPending.length() - keep));
				m_thinkPending.erase(0, m_thinkPending.length() - keep);
			}
			return;
		}

		//THINK_PLAIN: it's all reply text
		if (!m_thinkPending.empty())
		{
			Emit(LLM_DELTA_CONTENT, m_thinkPending);
			m_thinkPending.clear();
		}
		return;
	}
}

//the stream is over: whatever was held back is real text
void LLMStreamParser::FlushThink()
{
	if (!m_thinkPending.empty())
	{
		Emit(m_thinkState == THINK_INSIDE ? LLM_DELTA_REASONING : LLM_DELTA_CONTENT, m_thinkPending);
		m_thinkPending.clear();
	}
	if (m_thinkState == THINK_UNDECIDED)
		m_thinkState = THINK_PLAIN;
}

void LLMStreamParser::Emit(eLLMDeltaKind kind, const std::string &text)
{
	if (text.empty())
		return;
	if (kind == LLM_DELTA_REASONING)
		m_reasoning += text;
	else
		m_content += text;

	Delta d;
	d.kind = kind;
	d.text = text;
	m_pending.push_back(d);
}

//*** LLMClient

LLMClient::LLMClient()
{
}

void LLMClient::Setup(const std::string &serverName, int port, const std::string &apiPath, const std::string &model)
{
	m_serverName = serverName;
	m_port = port;
	m_apiPath = apiPath;
	m_model = model;
}

void LLMClient::SetParms(float temperature, int maxTokens, int maxRetries)
{
	m_temperature = temperature;
	m_maxTokens = maxTokens;
	m_maxRetries = maxRetries;
}

void LLMClient::SetLogFile(const std::string &path, bool bAppend)
{
	m_logPath = path;
	if (m_logPath.empty() || bAppend)
		return;

	FILE *fp = fopen(m_logPath.c_str(), "wb");
	if (!fp)
	{
		LogMsg("LLMClient: can't create log file %s", m_logPath.c_str());
		m_logPath.clear();
		return;
	}
	std::string header = "LLM log started " + LogTimestamp() + "\n";
	fwrite(header.c_str(), 1, header.length(), fp);
	fclose(fp);
}

void LLMClient::AppendToLog(const std::string &header, const std::string &body)
{
	if (m_logPath.empty())
		return;

	FILE *fp = fopen(m_logPath.c_str(), "ab");
	if (!fp)
		return;
	std::string entry = "\n==== " + LogTimestamp() + " " + header + " ====\n" + body + "\n";
	fwrite(entry.c_str(), 1, entry.length(), fp);
	fclose(fp);
}

float LLMClient::GetLastTPS() const
{
	if (m_lastReplyMS <= 0 || m_lastCompletionTokens <= 0)
		return 0;
	return m_lastCompletionTokens * 1000.0f / m_lastReplyMS;
}

int LLMClient::GetInFlightMS() const
{
	if (!IsBusy())
		return 0;
	return (int)(GetTick() - m_requestStartTick);
}

bool LLMClient::SendAsync(const LLMConversation &convo)
{
	if (IsBusy())
		return false;

	m_bStreamRequest = m_bStreaming;
	m_pendingBody = MergeExtraBody(convo.BuildChatCompletionJSON(m_model, m_temperature, m_maxTokens, m_bStreamRequest));
	m_lastRequestBytes = (int)m_pendingBody.length();

	char header[256];
	sprintf(header, "REQUEST to %s:%d/%s (%d bytes%s)", m_serverName.c_str(), m_port, m_apiPath.c_str(),
		m_lastRequestBytes, m_bStreamRequest ? ", streaming" : "");
	AppendToLog(header, m_pendingBody);

	m_attempt = 0;
	StartRequest();
	return true;
}

std::string LLMClient::MergeExtraBody(const std::string &body) const
{
	if (m_extraBodyJSON.empty())
		return body;

	cJSON *pRoot = cJSON_Parse(body.c_str());
	cJSON *pExtra = cJSON_Parse(m_extraBodyJSON.c_str());

	if (!pRoot || !pExtra)
	{
		LogMsg("LLMClient: SetExtraBodyJSON content isn't valid JSON, ignoring");
		if (pRoot) cJSON_Delete(pRoot);
		if (pExtra) cJSON_Delete(pExtra);
		return body;
	}

	cJSON *pChild = pExtra->child;
	while (pChild)
	{
		cJSON *pNext = pChild->next; //detaching invalidates the links
		cJSON *pDetached = cJSON_DetachItemViaPointer(pExtra, pChild);
		cJSON_DeleteItemFromObject(pRoot, pDetached->string); //extra wins on collision
		cJSON_AddItemToObject(pRoot, pDetached->string, pDetached);
		pChild = pNext;
	}

	char *pText = cJSON_PrintUnformatted(pRoot);
	std::string merged = pText ? pText : body;
	if (pText) cJSON_free(pText);
	cJSON_Delete(pRoot);
	cJSON_Delete(pExtra);
	return merged;
}

void LLMClient::StartRequest()
{
	m_attempt++;
	m_bRetryPending = false;
	m_requestStartTick = GetTick();

	//every attempt starts a fresh stream (a retry resends the same body)
	m_parser.Reset();
	m_streamContent.clear();
	m_streamReasoning.clear();
	m_streamChunks = 0;
	m_bGotFirstToken = false;
	m_lastFirstTokenMS = 0;

	m_netHTTP.Reset(true);
	m_netHTTP.Setup(m_serverName, m_port, m_apiPath, NetHTTP::END_OF_DATA_SIGNAL_HTTP);
	m_netHTTP.SetStreamMode(m_bStreamRequest); //after Reset/Setup, which clear it
	m_netHTTP.SetIdleTimeoutMS(m_timeoutMS);
	m_netHTTP.SetPostHeaderOverride("Content-Type: application/json\r\n");
	m_netHTTP.AddPostData("", (const uint8*)m_pendingBody.c_str(), (int)m_pendingBody.length());

	if (!m_netHTTP.Start())
	{
		m_bBusy = true; //so HandleFailure's retry path works consistently
		HandleFailure("couldn't start HTTP request");
		return;
	}

	m_bBusy = true;
}

void LLMClient::Abort()
{
	m_netHTTP.Reset(true); //closing the socket also makes vLLM stop generating
	m_bBusy = false;
	m_bRetryPending = false;
	m_pendingBody.clear();
}

void LLMClient::Update()
{
	if (m_bRetryPending)
	{
		if ((int)(GetTick() - m_retryAtTick) >= 0)
			StartRequest();
		return;
	}

	if (!m_bBusy)
		return;

	m_netHTTP.Update();

	if (m_netHTTP.GetError() != NetHTTP::ERROR_NONE)
	{
		char err[64];
		sprintf(err, "HTTP error %d", (int)m_netHTTP.GetError());
		HandleFailure(err);
		return;
	}

	if (m_bStreamRequest && m_netHTTP.GetState() == NetHTTP::STATE_ACTIVE)
	{
		if (m_netHTTP.IsHeaderReceived())
		{
			PumpStream(m_netHTTP.GetStreamBody(), false);
			if (!m_bBusy)
				return; //a delta handler aborted, or the server sent an error event
			if (m_parser.IsDone())
				FinishStreamedReply(); //no need to wait for the server to close the connection
		}
		return;
	}

	if (m_netHTTP.GetState() == NetHTTP::STATE_FINISHED)
	{
		if (m_bStreamRequest)
		{
			FinishStreamedReply();
		}
		else
		{
			m_lastReplyMS = (int)(GetTick() - m_requestStartTick);
			DeliverWholeReply((const char*)m_netHTTP.GetDownloadedData());
		}
	}
}

void LLMClient::DeliverWholeReply(const char *pRaw)
{
	std::string parseErr;
	std::string content = ParseAssistantContent(pRaw, parseErr);

	char header[128];
	sprintf(header, "RESPONSE (%d ms, prompt %d tok, completion %d tok, %.1f tok/s)",
		m_lastReplyMS, m_lastPromptTokens, m_lastCompletionTokens, GetLastTPS());
	AppendToLog(header, pRaw ? pRaw : "(empty)");

	if (!parseErr.empty())
	{
		HandleFailure(parseErr);
		return;
	}

	m_netHTTP.Reset(true);
	m_bBusy = false;
	m_pendingBody.clear();

	VariantList v;
	v.Get(0).Set(content);
	m_sig_response(&v);
}

void LLMClient::PumpStream(const std::string &body, bool bFinal)
{
	m_parser.Feed(body, bFinal);
	m_streamChunks = m_parser.GetChunkCount();

	//drain first, signal after: a handler may Abort(), which clears NetHTTP's buffer
	std::vector<LLMStreamParser::Delta> deltas;
	deltas.swap(m_parser.GetPendingDeltas());

	for (size_t i = 0; i < deltas.size(); i++)
	{
		if (deltas[i].kind == LLM_DELTA_REASONING)
			m_streamReasoning += deltas[i].text;
		else
			m_streamContent += deltas[i].text;

		if (!m_bGotFirstToken)
		{
			m_bGotFirstToken = true;
			m_lastFirstTokenMS = (int)(GetTick() - m_requestStartTick);
		}

		VariantList v;
		v.Get(0).Set(deltas[i].text);
		v.Get(1).Set(uint32(deltas[i].kind));
		m_sig_delta(&v);
		if (!m_bBusy)
			return; //a handler called Abort()
	}

	if (!m_parser.GetError().empty())
		HandleFailure("server error: " + m_parser.GetError());
}

void LLMClient::FinishStreamedReply()
{
	m_lastReplyMS = (int)(GetTick() - m_requestStartTick);

	//a copy: the Reset below clears NetHTTP's buffer
	std::string body = m_netHTTP.GetStreamBody();
	if (body.empty() && m_netHTTP.GetDownloadedData())
		body = (const char*)m_netHTTP.GetDownloadedData(); //a backend without stream mode delivers it whole here

	PumpStream(body, true);
	if (!m_bBusy)
		return;

	if (m_parser.GetBodyKind() != LLMStreamParser::BODY_SSE)
	{
		//not a stream after all: an error object (HTTP 4xx/5xx), or a server
		//that ignored stream:true. The non-streamed parse handles both
		DeliverWholeReply(body.c_str());
		return;
	}

	m_lastPromptTokens = m_parser.GetPromptTokens();
	m_lastCompletionTokens = m_parser.GetCompletionTokens();
	m_lastFinishReason = m_parser.GetFinishReason();
	m_lastReasoning = m_streamReasoning;

	char header[256];
	sprintf(header, "RESPONSE (streamed, %d chunks, %d ms, first token %d ms, prompt %d tok, completion %d tok, %.1f tok/s, raw %d bytes)",
		m_streamChunks, m_lastReplyMS, m_lastFirstTokenMS, m_lastPromptTokens, m_lastCompletionTokens, GetLastTPS(), (int)body.length());
	std::string logBody;
	if (!m_streamReasoning.empty())
		logBody += "[reasoning]\n" + m_streamReasoning + "\n";
	logBody += "[content]\n" + m_streamContent;
	AppendToLog(header, logBody);

	if (!m_parser.IsDone())
		LogMsg("LLMClient: the stream ended without [DONE] (finish_reason \"%s\")", m_lastFinishReason.c_str());

	if (m_streamContent.empty())
	{
		//finish_reason "length" inside the reasoning: the budget ran out before the reply
		HandleFailure(m_streamReasoning.empty() ? "stream ended without content"
			: "model produced only reasoning, no content (out of tokens?)");
		return;
	}

	m_netHTTP.Reset(true);
	m_bBusy = false;
	m_pendingBody.clear();

	VariantList v;
	v.Get(0).Set(m_streamContent);
	m_sig_response(&v);
}

void LLMClient::HandleFailure(const std::string &errorMsg)
{
	m_netHTTP.Reset(true);
	m_bBusy = false;

	int retriesLeft = m_maxRetries - m_attempt;
	if (retriesLeft < 0) retriesLeft = 0;

	if (retriesLeft > 0)
	{
		m_bRetryPending = true;
		m_retryAtTick = GetTick() + 2000 * m_attempt; //simple linear backoff
	}
	else
	{
		m_pendingBody.clear();
	}

	LogMsg("LLMClient: attempt %d failed (%s), %d retries left", m_attempt, errorMsg.c_str(), retriesLeft);

	char header[128];
	sprintf(header, "ERROR (attempt %d, %d retries left)", m_attempt, retriesLeft);
	AppendToLog(header, errorMsg);

	VariantList v;
	v.Get(0).Set(errorMsg);
	v.Get(1).Set(uint32(retriesLeft));
	m_sig_error(&v);
}

void LLMClient::ReadUsage(void *pRootJSON)
{
	m_lastPromptTokens = 0;
	m_lastCompletionTokens = 0;
	cJSON *pUsage = cJSON_GetObjectItem((cJSON*)pRootJSON, "usage");
	if (pUsage && !cJSON_IsNull(pUsage))
	{
		cJSON *pPrompt = cJSON_GetObjectItem(pUsage, "prompt_tokens");
		cJSON *pCompletion = cJSON_GetObjectItem(pUsage, "completion_tokens");
		if (pPrompt && cJSON_IsNumber(pPrompt)) m_lastPromptTokens = pPrompt->valueint;
		if (pCompletion && cJSON_IsNumber(pCompletion)) m_lastCompletionTokens = pCompletion->valueint;
	}
}

std::string LLMClient::ParseAssistantContent(const char *pJSON, std::string &errOut)
{
	errOut.clear();
	m_lastReasoning.clear();
	m_lastFinishReason.clear();

	if (!pJSON || pJSON[0] == 0)
	{
		errOut = "empty response body";
		return "";
	}

	cJSON *pRoot = cJSON_Parse(pJSON);
	if (!pRoot)
	{
		errOut = "response isn't valid JSON";
		return "";
	}

	ReadUsage(pRoot);

	std::string content;

	//OpenAI-style errors are {"error":{"message":...}}; vLLM uses a top-level
	//{"object":"error","message":...}
	cJSON *pError = cJSON_GetObjectItem(pRoot, "error");
	cJSON *pTopMsg = cJSON_GetObjectItem(pRoot, "message");
	if (!pError && pTopMsg && cJSON_IsString(pTopMsg) && !cJSON_GetObjectItem(pRoot, "choices"))
	{
		errOut = std::string("server error: ") + pTopMsg->valuestring;
	}
	else if (pError)
	{
		cJSON *pMsg = cJSON_GetObjectItem(pError, "message");
		if (pMsg && cJSON_IsString(pMsg))
			errOut = std::string("server error: ") + pMsg->valuestring;
		else
			errOut = "server returned an error";
	}
	else
	{
		cJSON *pChoices = cJSON_GetObjectItem(pRoot, "choices");
		cJSON *pChoice = pChoices ? cJSON_GetArrayItem(pChoices, 0) : NULL;
		cJSON *pMessage = pChoice ? cJSON_GetObjectItem(pChoice, "message") : NULL;
		cJSON *pContent = pMessage ? cJSON_GetObjectItem(pMessage, "content") : NULL;

		cJSON *pFinish = pChoice ? cJSON_GetObjectItem(pChoice, "finish_reason") : NULL;
		if (pFinish && cJSON_IsString(pFinish))
			m_lastFinishReason = pFinish->valuestring;

		//the reasoning parser's field: "reasoning" (newer vLLM) or "reasoning_content"
		cJSON *pReasoning = pMessage ? cJSON_GetObjectItem(pMessage, "reasoning") : NULL;
		if (!pReasoning || !cJSON_IsString(pReasoning))
			pReasoning = pMessage ? cJSON_GetObjectItem(pMessage, "reasoning_content") : NULL;
		if (pReasoning && cJSON_IsString(pReasoning))
			m_lastReasoning = pReasoning->valuestring;

		if (pContent && cJSON_IsString(pContent) && pContent->valuestring[0])
		{
			content = pContent->valuestring;

			//no reasoning parser on the server: a leading <think> block is the
			//reasoning. Captured here but left in the content (callers strip it)
			if (m_lastReasoning.empty())
			{
				size_t start = content.find_first_not_of(" \t\r\n");
				if (start != std::string::npos && content.compare(start, 7, "<think>") == 0)
				{
					size_t end = content.find("</think>", start);
					m_lastReasoning = content.substr(start + 7, (end == std::string::npos) ? std::string::npos : end - start - 7);
				}
			}
		}
		else if (!m_lastReasoning.empty() || (pMessage && cJSON_GetObjectItem(pMessage, "reasoning")))
		{
			//reasoning models can burn the whole max_tokens budget "thinking"
			//and return content:null; the fix is usually to disable thinking
			//(SetExtraBodyJSON) or raise maxTokens
			errOut = "model produced only reasoning, no content (out of tokens?)";
		}
		else if (pContent && cJSON_IsString(pContent))
		{
			content = pContent->valuestring; //empty: the caller's problem, as before
		}
		else
		{
			errOut = "response is missing choices[0].message.content";
		}
	}

	cJSON_Delete(pRoot);

	if (!errOut.empty())
	{
		//show what actually came back, it makes server-side problems debuggable
		std::string preview(pJSON);
		if (preview.length() > 400) preview.resize(400);
		LogMsg("LLMClient: raw response was: %s", preview.c_str());
	}

	return content;
}
