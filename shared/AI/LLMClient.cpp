#include "PlatformPrecomp.h"
#include "LLMClient.h"
#include "util/cJSON.h"

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

std::string LLMConversation::BuildChatCompletionJSON(const std::string &model, float temperature, int maxTokens) const
{
	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, "model", model.c_str());
	cJSON_AddNumberToObject(pRoot, "temperature", temperature);
	cJSON_AddNumberToObject(pRoot, "max_tokens", maxTokens);

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

bool LLMClient::SendAsync(const LLMConversation &convo)
{
	if (IsBusy())
		return false;

	m_pendingBody = MergeExtraBody(convo.BuildChatCompletionJSON(m_model, m_temperature, m_maxTokens));
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

	m_netHTTP.Reset(true);
	m_netHTTP.Setup(m_serverName, m_port, m_apiPath, NetHTTP::END_OF_DATA_SIGNAL_HTTP);
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
	m_netHTTP.Reset(true);
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

	if (m_netHTTP.GetState() == NetHTTP::STATE_FINISHED)
	{
		std::string parseErr;
		std::string content = ParseAssistantContent((const char*)m_netHTTP.GetDownloadedData(), parseErr);
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

	VariantList v;
	v.Get(0).Set(errorMsg);
	v.Get(1).Set(uint32(retriesLeft));
	m_sig_error(&v);
}

std::string LLMClient::ParseAssistantContent(const char *pJSON, std::string &errOut)
{
	errOut.clear();

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

		if (pContent && cJSON_IsString(pContent))
		{
			content = pContent->valuestring;
		}
		else if (pMessage && cJSON_GetObjectItem(pMessage, "reasoning"))
		{
			//reasoning models can burn the whole max_tokens budget "thinking"
			//and return content:null; the fix is usually to disable thinking
			//(SetExtraBodyJSON) or raise maxTokens
			errOut = "model produced only reasoning, no content (out of tokens?)";
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
