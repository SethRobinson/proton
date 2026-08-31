#include "PlatformPrecomp.h"
#include "TTSClient.h"

TTSClient::TTSClient()
{
}

TTSClient::~TTSClient()
{
	CancelAll();
	for (size_t i = 0; i < m_workers.size(); i++)
		delete m_workers[i];
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

void TTSClient::SetMaxParallel(int n)
{
	m_maxParallel = (n < 1) ? 1 : n;
}

int TTSClient::GetInFlightCount() const
{
	int count = 0;
	for (size_t i = 0; i < m_workers.size(); i++)
		if (m_workers[i]->bBusy || m_workers[i]->bRetryPending) count++;
	return count;
}

uint32 TTSClient::Speak(const std::string &text, const std::string &outFile, int priority)
{
	return Speak(text, outFile, FieldMap(), priority);
}

uint32 TTSClient::Speak(const std::string &text, const std::string &outFile, const FieldMap &fields, int priority)
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
	r.priority = priority;
	r.text = text;
	r.outFile = outFile;
	r.fields = fields;

	//behind everything of the same or higher priority
	std::deque<Request>::iterator it = m_queue.begin();
	while (it != m_queue.end() && it->priority >= priority)
		++it;
	m_queue.insert(it, r);

	Dispatch();
	return r.id;
}

bool TTSClient::Cancel(uint32 id)
{
	for (std::deque<Request>::iterator it = m_queue.begin(); it != m_queue.end(); ++it)
	{
		if (it->id == id)
		{
			m_queue.erase(it);
			return true;
		}
	}
	for (size_t i = 0; i < m_workers.size(); i++)
	{
		Worker &w = *m_workers[i];
		if ((w.bBusy || w.bRetryPending) && w.req.id == id)
		{
			LogMsg("TTSClient: line #%u cancelled while in flight", id);
			FreeWorker(w);
			Dispatch();
			return true;
		}
	}
	return false;
}

void TTSClient::CancelAll()
{
	m_queue.clear();
	for (size_t i = 0; i < m_workers.size(); i++)
	{
		Worker &w = *m_workers[i];
		if (w.bBusy || w.bRetryPending)
			FreeWorker(w);
	}
}

bool TTSClient::IsPending(uint32 id) const
{
	for (std::deque<Request>::const_iterator it = m_queue.begin(); it != m_queue.end(); ++it)
		if (it->id == id) return true;
	for (size_t i = 0; i < m_workers.size(); i++)
		if ((m_workers[i]->bBusy || m_workers[i]->bRetryPending) && m_workers[i]->req.id == id) return true;
	return false;
}

bool TTSClient::IsInFlight(uint32 id) const
{
	for (size_t i = 0; i < m_workers.size(); i++)
		if ((m_workers[i]->bBusy || m_workers[i]->bRetryPending) && m_workers[i]->req.id == id) return true;
	return false;
}

void TTSClient::Dispatch()
{
	while (!m_queue.empty())
	{
		Worker *pFree = NULL;
		for (size_t i = 0; i < m_workers.size() && !pFree; i++)
		{
			if (!m_workers[i]->bBusy && !m_workers[i]->bRetryPending)
				pFree = m_workers[i];
		}
		if (!pFree && (int)m_workers.size() < m_maxParallel)
		{
			pFree = new Worker;
			m_workers.push_back(pFree);
		}
		if (!pFree)
			return; //every slot is taken, the rest keep waiting

		pFree->req = m_queue.front();
		m_queue.pop_front();
		pFree->attempt = 0;
		StartOnWorker(*pFree);
	}
}

void TTSClient::StartOnWorker(Worker &w)
{
	w.attempt++;
	w.bRetryPending = false;
	w.startTick = GetTick();

	w.net.Reset(true);
	w.net.Setup(m_serverName, m_port, m_apiPath, NetHTTP::END_OF_DATA_SIGNAL_HTTP);
	w.net.SetIdleTimeoutMS(m_timeoutMS);
	//NetHTTP's default post encoding (application/x-www-form-urlencoded) is
	//what a form-field TTS endpoint expects; the text goes first
	w.net.AddPostData(m_textField, (const uint8*)w.req.text.c_str(), (int)w.req.text.length());

	//client-wide fields, overridden by the request's own
	FieldMap fields = m_fields;
	for (FieldMap::const_iterator it = w.req.fields.begin(); it != w.req.fields.end(); ++it)
		fields[it->first] = it->second;
	for (FieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it)
	{
		if (!it->second.empty()) //the encoder asserts on empty data
			w.net.AddPostData(it->first, (const uint8*)it->second.c_str(), (int)it->second.length());
	}

	w.bBusy = true;
	if (!w.net.Start())
		HandleFailure(w, "couldn't start HTTP request (can't resolve " + m_serverName + "?)");
}

void TTSClient::FreeWorker(Worker &w)
{
	w.net.Reset(true);
	w.bBusy = false;
	w.bRetryPending = false;
	w.req = Request();
}

void TTSClient::Update()
{
	//by index: a signal handler may Speak() and grow the vector
	for (size_t i = 0; i < m_workers.size(); i++)
		UpdateWorker(*m_workers[i]);
	Dispatch();
}

void TTSClient::UpdateWorker(Worker &w)
{
	if (w.bRetryPending)
	{
		if ((int)(GetTick() - w.retryAtTick) >= 0)
			StartOnWorker(w);
		return;
	}

	if (!w.bBusy)
		return;

	w.net.Update();

	if (w.net.GetError() != NetHTTP::ERROR_NONE)
	{
		char err[64];
		sprintf(err, "HTTP error %d", (int)w.net.GetError());
		HandleFailure(w, err);
		return;
	}

	if (w.net.GetState() != NetHTTP::STATE_FINISHED)
		return;

	int replyMS = (int)(GetTick() - w.startTick);
	const uint8 *pData = w.net.GetDownloadedData();
	int len = w.net.GetDownloadedBytes();
	int resultCode = w.net.GetResultCode();

	std::string err;
	if (resultCode >= 400)
		err = "server replied " + toString(resultCode) + ": " + BodyPreview(pData, len, 300);
	else
		ValidateAudio(pData, len, err);

	if (!err.empty())
	{
		HandleFailure(w, err);
		return;
	}

	if (!WriteFile(w.req.outFile, pData, len))
	{
		HandleFailure(w, "couldn't write " + w.req.outFile);
		return;
	}

	int audioMS = GetWavDurationMS(pData, len);
	m_lastReplyMS = replyMS;
	m_lastAudioBytes = len;
	m_lastAudioMS = audioMS;
	LogMsg("TTSClient: line #%u ready, %d chars -> %.1f s of audio in %d ms (%s)",
		w.req.id, (int)w.req.text.length(), audioMS / 1000.0f, replyMS, w.req.outFile.c_str());

	Request done = w.req;
	FreeWorker(w);
	Dispatch(); //the slot goes to the next line before the handler runs

	VariantList v;
	v.Get(0).Set(done.outFile);
	v.Get(1).Set(uint32(done.id));
	v.Get(2).Set(done.text);
	v.Get(3).Set(uint32(replyMS));
	v.Get(4).Set(uint32(audioMS));
	m_sig_ready(&v);
}

void TTSClient::HandleFailure(Worker &w, const std::string &errorMsg)
{
	w.net.Reset(true);
	w.bBusy = false;

	int retriesLeft = m_maxRetries - (w.attempt - 1);
	if (retriesLeft < 0) retriesLeft = 0;

	LogMsg("TTSClient: line #%u attempt %d failed (%s), %d retries left", w.req.id, w.attempt, errorMsg.c_str(), retriesLeft);

	if (retriesLeft > 0)
	{
		w.bRetryPending = true;
		w.retryAtTick = GetTick() + 1500 * w.attempt; //linear backoff
		return;
	}

	Request failed = w.req;
	FreeWorker(w);
	Dispatch();

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

static uint32 ReadLE32(const uint8 *p)
{
	return (uint32)p[0] | ((uint32)p[1] << 8) | ((uint32)p[2] << 16) | ((uint32)p[3] << 24);
}

int TTSClient::GetWavDurationMS(const uint8 *pData, int len)
{
	if (!pData || len < 12 || memcmp(pData, "RIFF", 4) != 0 || memcmp(pData + 8, "WAVE", 4) != 0)
		return 0;

	uint32 byteRate = 0;
	uint32 dataSize = 0;
	int pos = 12;
	while (pos + 8 <= len)
	{
		const uint8 *pChunk = pData + pos;
		uint32 chunkSize = ReadLE32(pChunk + 4);
		if (memcmp(pChunk, "fmt ", 4) == 0 && chunkSize >= 16 && pos + 8 + 16 <= len)
		{
			byteRate = ReadLE32(pChunk + 8 + 8); //sample rate * channels * bytes per sample
		}
		else if (memcmp(pChunk, "data", 4) == 0)
		{
			dataSize = chunkSize;
			uint32 present = (uint32)(len - pos - 8);
			if (dataSize > present) dataSize = present; //truncated, or a streaming encoder's placeholder
			break;
		}
		pos += 8 + (int)chunkSize + (int)(chunkSize & 1); //chunks are word aligned
		if (chunkSize > (uint32)len) break; //nonsense size, don't loop on it
	}

	if (byteRate == 0 || dataSize == 0)
		return 0;
	return (int)(((unsigned long long)dataSize * 1000) / byteRate);
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
