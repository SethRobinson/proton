#include "PlatformPrecomp.h"
#include "TTSClient.h"

TTSClient::TTSClient()
{
}

void TTSClient::Setup(const std::string &serverName, int port, const std::string &apiPath)
{
	m_serverName = serverName;
	m_port = port;
	m_apiPath = apiPath;
}

void TTSClient::SetField(const std::string &name, const std::string &value)
{
	if (value.empty())
		m_fields.erase(name);
	else
		m_fields[name] = value;
}

int TTSClient::GetInFlightMS() const
{
	if (!m_bBusy)
		return 0;
	return (int)(GetTick() - m_requestStartTick);
}

uint32 TTSClient::Speak(const std::string &text, const std::string &outFile)
{
	if (m_serverName.empty())
	{
		LogMsg("TTSClient: Speak() before Setup(), ignoring");
		return 0;
	}
	if (text.empty() || outFile.empty())
		return 0;

	Request r;
	r.id = m_nextID++;
	r.text = text;
	r.outFile = outFile;

	if (m_bBusy || m_bRetryPending)
	{
		//latest wins: whatever was waiting never gets generated
		if (m_bHasPending)
			LogMsg("TTSClient: line #%u replaced pending line #%u", r.id, m_pending.id);
		m_pending = r;
		m_bHasPending = true;
		return r.id;
	}

	m_current = r;
	m_attempt = 0;
	StartRequest();
	return r.id;
}

void TTSClient::StartRequest()
{
	m_attempt++;
	m_bRetryPending = false;
	m_requestStartTick = GetTick();

	m_netHTTP.Reset(true);
	m_netHTTP.Setup(m_serverName, m_port, m_apiPath, NetHTTP::END_OF_DATA_SIGNAL_HTTP);
	m_netHTTP.SetIdleTimeoutMS(m_timeoutMS);
	//NetHTTP's default post encoding (application/x-www-form-urlencoded) is
	//what a form-field TTS endpoint expects; the text goes first
	m_netHTTP.AddPostData(m_textField, (const uint8*)m_current.text.c_str(), (int)m_current.text.length());
	for (std::map<std::string, std::string>::const_iterator it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		if (!it->second.empty()) //the encoder asserts on empty data
			m_netHTTP.AddPostData(it->first, (const uint8*)it->second.c_str(), (int)it->second.length());
	}

	m_bBusy = true;
	if (!m_netHTTP.Start())
		HandleFailure("couldn't start HTTP request (can't resolve " + m_serverName + "?)");
}

void TTSClient::Abort()
{
	m_netHTTP.Reset(true);
	m_bBusy = false;
	m_bRetryPending = false;
	m_bHasPending = false;
	m_current = Request();
	m_pending = Request();
}

void TTSClient::FinishRequest()
{
	m_netHTTP.Reset(true);
	m_bBusy = false;
	m_bRetryPending = false;
	m_current = Request();

	if (m_bHasPending)
	{
		m_current = m_pending;
		m_pending = Request();
		m_bHasPending = false;
		m_attempt = 0;
		StartRequest();
	}
}

void TTSClient::Update()
{
	if (m_bRetryPending)
	{
		if (m_bHasPending)
		{
			//no point retrying a line that's already been superseded
			LogMsg("TTSClient: line #%u superseded before its retry, skipping it", m_current.id);
			FinishRequest();
			return;
		}
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

	if (m_netHTTP.GetState() != NetHTTP::STATE_FINISHED)
		return;

	m_lastReplyMS = (int)(GetTick() - m_requestStartTick);

	if (m_bHasPending)
	{
		//a newer line arrived while this one was generating: it's stale now
		LogMsg("TTSClient: line #%u finished (%d ms) but #%u supersedes it, dropped", m_current.id, m_lastReplyMS, m_pending.id);
		FinishRequest();
		return;
	}

	const uint8 *pData = m_netHTTP.GetDownloadedData();
	int len = m_netHTTP.GetDownloadedBytes();
	int resultCode = m_netHTTP.GetResultCode();

	std::string err;
	if (resultCode >= 400)
		err = "server replied " + toString(resultCode) + ": " + BodyPreview(pData, len, 300);
	else
		ValidateAudio(pData, len, err);

	if (!err.empty())
	{
		HandleFailure(err);
		return;
	}

	if (!WriteFile(m_current.outFile, pData, len))
	{
		HandleFailure("couldn't write " + m_current.outFile);
		return;
	}

	m_lastAudioBytes = len;
	LogMsg("TTSClient: line #%u ready, %d chars -> %d bytes in %d ms (%s)", m_current.id,
		(int)m_current.text.length(), len, m_lastReplyMS, m_current.outFile.c_str());

	Request done = m_current;
	FinishRequest(); //may start the pending line; do it before handing out the result

	VariantList v;
	v.Get(0).Set(done.outFile);
	v.Get(1).Set(uint32(done.id));
	v.Get(2).Set(done.text);
	m_sig_ready(&v);
}

void TTSClient::HandleFailure(const std::string &errorMsg)
{
	m_netHTTP.Reset(true);
	m_bBusy = false;

	int retriesLeft = m_maxRetries - (m_attempt - 1);
	if (retriesLeft < 0) retriesLeft = 0;

	LogMsg("TTSClient: line #%u attempt %d failed (%s), %d retries left", m_current.id, m_attempt, errorMsg.c_str(), retriesLeft);

	if (retriesLeft > 0 && !m_bHasPending)
	{
		m_bRetryPending = true;
		m_retryAtTick = GetTick() + 1500 * m_attempt;
		return;
	}

	Request failed = m_current;
	FinishRequest(); //moves on to the pending line if there is one

	VariantList v;
	v.Get(0).Set(errorMsg);
	v.Get(1).Set(uint32(failed.id));
	m_sig_error(&v);
}

//the start of a reply body as printable text, for error messages
std::string TTSClient::BodyPreview(const uint8 *pData, int len, int maxChars)
{
	if (!pData || len <= 0)
		return "(empty reply)";
	std::string s;
	for (int i = 0; i < len && (int)s.length() < maxChars; i++)
	{
		unsigned char c = pData[i];
		s += (c >= 32 && c < 127) ? (char)c : (c == '\n' ? ' ' : '.');
	}
	return s;
}

bool TTSClient::ValidateAudio(const uint8 *pData, int len, std::string &errOut)
{
	errOut.clear();

	if (!pData || len <= 0)
	{
		errOut = "empty reply";
		return false;
	}
	if (len < 64)
	{
		errOut = "reply too small to be audio (" + toString(len) + " bytes): " + BodyPreview(pData, len, 64);
		return false;
	}

	if (memcmp(pData, "RIFF", 4) == 0)
	{
		//the RIFF size field is the file size minus 8: a download that ended
		//early (a lost Content-Length, a dropped connection) shows up here.
		//0 and 0xFFFFFFFF are what streaming encoders write, accept those
		uint32 riffSize = (uint32)pData[4] | ((uint32)pData[5] << 8) | ((uint32)pData[6] << 16) | ((uint32)pData[7] << 24);
		if (riffSize != 0 && riffSize != 0xFFFFFFFF && riffSize + 8 > (uint32)len)
		{
			errOut = "truncated WAV: header says " + toString((int)(riffSize + 8)) + " bytes, got " + toString(len);
			return false;
		}
		return true;
	}
	if (memcmp(pData, "OggS", 4) == 0) return true; //ogg/opus
	if (memcmp(pData, "fLaC", 4) == 0) return true; //flac
	if (memcmp(pData, "ID3", 3) == 0) return true;  //mp3 with a tag
	if (pData[0] == 0xFF && (pData[1] & 0xE0) == 0xE0) return true; //bare mp3 frame

	errOut = "reply isn't audio: " + BodyPreview(pData, len, 300);
	return false;
}

bool TTSClient::WriteFile(const std::string &path, const uint8 *pData, int len)
{
	FILE *fp = fopen(path.c_str(), "wb");
	if (!fp)
		return false;
	bool bOK = ((int)fwrite(pData, 1, len, fp) == len);
	fclose(fp);
	return bOK;
}
