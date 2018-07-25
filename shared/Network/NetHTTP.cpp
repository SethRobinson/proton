#include "PlatformPrecomp.h"

#if defined PLATFORM_HTML5
#include "../html5/NetHTTP_HTML5.cpp"
#else

#include "NetHTTP.h"
#include "NetUtils.h"
#include "util/TextScanner.h"

#define NET_END_MARK_CHECK_DELAY_MS 333
#define C_END_DOWNLOAD_MARKER_STRING "RTENDMARKERBS1001"
#define C_DEFAULT_IDLE_TIMEOUT_MS (25*1000)

NetHTTP::NetHTTP()
{
	m_pFile = NULL;
	Reset(true);
}

NetHTTP::~NetHTTP()
{
	if (m_pFile)
	{
		fclose(m_pFile);
		RemoveFile(m_fileName);
		m_pFile = NULL;
	}
}

void NetHTTP::Reset(bool bClearPostdata)
{

	if (m_pFile)
	{
		fclose(m_pFile);
		RemoveFile(m_fileName);
		m_pFile = NULL;
	}

	m_endOfDataSignal = END_OF_DATA_SIGNAL_RTSOFT_MARKER; //don't change the default, Seth's software relies on it
	m_state = STATE_IDLE;
	m_error = ERROR_NONE;
	m_timer = 0;
	m_idleTimeOutMS = C_DEFAULT_IDLE_TIMEOUT_MS;
	m_expectedFileBytes= 0;
	m_downloadData.clear();
	m_replyHeader.clear();
	m_query.clear();
	if (bClearPostdata)
	{
		m_contentType = "";
		m_postData.clear();
		
	}
	m_bytesWrittenToFile = 0;
}

void NetHTTP::Setup( string serverName, int port, string query, eEndOfDataSignal eodSignal )
{
	m_endOfDataSignal = eodSignal;
	m_serverName = serverName;
	m_port = port;
	m_query = query;
}

bool NetHTTP::SetFileOutput(const string &fName)
{
	assert(!m_pFile);

	m_pFile = fopen(fName.c_str(), "wb");
	
	m_fileName = fName; //save for later so we can delete this file if anything goes wrong
	
	if (!m_pFile)
	{
		OnError(ERROR_WRITING_FILE);
		return false;
	}

	return true;
}

bool NetHTTP::AddPostData( const string &name, const byte *pData, int len/*=-1*/ )
{
	if (m_postData.length() != 0)
	{
		//adding to other named post data, so we need a separator
		m_postData += "&";
	}

	//at this stage we need to encode it for safe html transfer, before we get the length
	URLEncoder encoder;

	if (!name.empty())
	{
		encoder.encodeData((const byte*)name.c_str(), name.length(), m_postData);
		m_postData += '=';
		if (len == -1) len = strlen((const char*) pData);

		encoder.encodeData(pData, len, m_postData);
	} else
	{
		//it's put data.  No name for it
		m_contentType = "put";
		m_postData = string((char*)pData);
	}

	
#ifdef _DEBUG

	/*
	LogMsg("Postdata is now %s", m_postData.c_str());


	URLDecoder decoder;

	string encoded;
	encoder.encodeData(pData, len, encoded);
	vector<byte> decoded = decoder.decodeData(encoded);
	int sizeOfDecoded = decoded.size();
	LogMsg("Size of decoded should be %u", decoded.size());
	assert(memcmp(pData, &decoded[0], len) == 0 && "Encryption of string error, they don't match");
	*/

#endif

	return true;
}


bool NetHTTP::AddPutData( const string data )
{
	return AddPostData("",  (byte*)data.c_str(),data.size());
}

string NetHTTP::BuildHTTPHeader()
{
	string header, stCommand;

	if (m_postData.length() > 0)
	{
		stCommand = "POST";
	} else
	{
		stCommand = "GET";
	}

	if (!m_contentType.empty())
	{
		stCommand = "PUT";
	}

	string queryEncoded;
	
	queryEncoded = m_query;
	StringReplace(" ", "+", queryEncoded);

	header = stCommand + " /" + queryEncoded + " HTTP/1.0\r\n";
	header += "Accept: */*\r\n";
	header += "Host: " + m_serverName + "\r\n";

	if (m_postData.length() > 0)
	{
		header += "Content-Type: application/x-www-form-urlencoded\r\n";
		header += "Content-Length: "+toString(m_postData.length())+"\r\n";
	}

	header +="\r\n"; //add the final CR to indicate we're done with the header

	return header;
}

bool NetHTTP::Start()
{
	m_bytesWrittenToFile = 0;
	m_error = ERROR_NONE;
	m_downloadData.clear();
	m_downloadHeader.clear();
	m_expectedFileBytes = 0;
	string header = BuildHTTPHeader();

#ifdef _DEBUG
	LogMsg(header.c_str());
#endif
	//take on the post data if applicable
	
#ifdef _DEBUG
LogMsg("Opening %s on port %d", m_serverName.c_str(), m_port);
#endif
	if (!m_netSocket.Init(m_serverName, m_port))
	{
		OnError(ERROR_CANT_RESOLVE_URL);
		return false;
	}

	m_state = STATE_ACTIVE;
	m_netSocket.Write(header);
	m_netSocket.Write(m_postData);
	return true;
}


bool CheckCharVectorForString(vector<char> &v, string marker, int *pIndexOfMarkerEndPosOut)
{
	int correctCount = 0;
	assert(marker.size() > 0);

	for (unsigned int i=0; i < v.size(); i++)
	{
		if (v[i] == marker[correctCount] )
		{
			//so far so good
			correctCount++;
			if (correctCount == marker.size())
			{
				if (pIndexOfMarkerEndPosOut)
				{
					*pIndexOfMarkerEndPosOut = i+1;
				}
				return true; //found it
			}
		} else
		{
			//nah
			correctCount = 0;
		}
	}

	return false;
}

int NetHTTP::ScanDownloadedHeader()
{
	TextScanner t(m_downloadHeader.c_str());
	string temp = t.GetParmString("Content-Length", 1, ":");
	m_expectedFileBytes = atoi(temp.c_str());

	int resultCode = atol(SeparateStringSTL(t.m_lines[0], 1, ' ').c_str());
	switch (resultCode)
	{
		case 404:
			OnError(ERROR_404_FILE_NOT_FOUND);
			break;
		
		case 301: //moved permanently
		case 302: //moved temporarily
		
		string url = t.GetParmString("Location:",1, " ");

		if (!url.empty())
		{
			string domain;
			string request;
			int port = 80;
			BreakDownURLIntoPieces(url, domain, request, port);
			string fNameTemp = m_fileName;
			Reset(false); //end connection, setup new one
			if (!fNameTemp.empty())
			{
				SetFileOutput(fNameTemp);
			}
			Setup(domain, port, request, m_endOfDataSignal);
			Start();
		}
		
	}

	return resultCode;
}

void NetHTTP::Update()
{
	m_netSocket.Update();

	if (m_state == STATE_ACTIVE)
	{
		//we don't know how many bytes are coming in this case, so we'll look for a special marker.

		if (m_timer < GetTick())
		{
			m_timer = GetTick()+NET_END_MARK_CHECK_DELAY_MS;
		
			vector<char> &s = m_netSocket.GetBuffer();
			
			if (m_downloadHeader.empty())
			{
				//keep trying to locate the header portion
				int indexOfEndOfHeader;
				if (!CheckCharVectorForString(m_netSocket.GetBuffer(), "\r\n\r\n", &indexOfEndOfHeader)
					&& !CheckCharVectorForString(m_netSocket.GetBuffer(), "\n\n", &indexOfEndOfHeader))
				{
					//haven't downloaded enough yet I guess
				} else
				{
					m_downloadHeader.insert(m_downloadHeader.begin(), s.begin(), s.begin()+indexOfEndOfHeader);
					
					//now would be a good time to scan it...
					int result = ScanDownloadedHeader();
					if (result == 301 || result == 302)
					{
						//new connection was started, let it happen naturally..
						return;
					}
					if (GetState() == STATE_ABORT) return;


					if (m_pFile)
					{
						//we want to start putting things into file, so we'll need to cut this header crap out
						s.erase(s.begin(), s.begin()+indexOfEndOfHeader);

					}
				}
			}
			
			if (m_expectedFileBytes == 0)
			{

				//if we know it's a .html file we can look for the double CR to know if it's done

				switch (m_endOfDataSignal)
				{
					case END_OF_DATA_SIGNAL_HTTP:
					{
						bool bFoundMarker = CheckCharVectorForString(m_netSocket.GetBuffer(), "\n\n");
						if (bFoundMarker || m_netSocket.WasDisconnected())
						{
							FinishDownload();
							return;
						}
					}
					break;
					
					case END_OF_DATA_SIGNAL_RTSOFT_MARKER:
						{
							bool bFoundMarker = CheckCharVectorForString(m_netSocket.GetBuffer(), C_END_DOWNLOAD_MARKER_STRING);
							if (bFoundMarker || m_netSocket.WasDisconnected())
							{
								FinishDownload();
								return;
							}

						}
						break;

				}
			
			} else
			{
				
				if (m_pFile)
				{
					//we're writing to file instead of the memory method
					if (s.size() > 0)
					{
						//write this to file
						int bytesWritten = fwrite(&s[0], 1, s.size(), m_pFile);
						if (bytesWritten != s.size())
						{
							OnError(ERROR_WRITING_FILE);
						}
						m_bytesWrittenToFile += bytesWritten;
						s.clear();
					}

					if (m_bytesWrittenToFile >= int(m_expectedFileBytes))
					{
						FinishDownload();
						return;
					}
				} else
				{
					//standard way of detecting the end
					if (m_netSocket.GetBuffer().size()-m_downloadHeader.size() >= m_expectedFileBytes)
					{
						FinishDownload();
						return;
					}

					
				}
				
			}
		}

		if (m_netSocket.GetIdleTimeMS() > m_idleTimeOutMS)
		{
			OnError(ERROR_COMMUNICATION_TIMEOUT);
			return;
		}
	}
}

void NetHTTP::OnError(eError e)
{
	m_error = e;
	m_state = STATE_ABORT;
	m_netSocket.Kill();

}
void NetHTTP::FinishDownload()
{
	if (m_downloadHeader.empty())
	{
		OnError(ERROR_CANT_RESOLVE_URL);
		return;
	}

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
		m_state = STATE_FINISHED;
		return;

	}
	//prepare the data so we can share it later when asked
	vector<char> &s = m_netSocket.GetBuffer();

	if (m_expectedFileBytes == 0)
	{
		switch(m_endOfDataSignal)
		{
		case END_OF_DATA_SIGNAL_HTTP:
			m_downloadData.insert(m_downloadData.begin(), s.begin()+m_downloadHeader.length(), s.end());
			break;

		case END_OF_DATA_SIGNAL_RTSOFT_MARKER:
			m_downloadData.insert(m_downloadData.begin(), s.begin()+m_downloadHeader.length(), s.end()-strlen(C_END_DOWNLOAD_MARKER_STRING));
			break;
		}
	} else
	{
		m_downloadData.insert(m_downloadData.begin(), s.begin()+m_downloadHeader.length(), s.end());
	}

	m_downloadData.push_back(0); //useful if used like a string
	m_state = STATE_FINISHED;
}

const byte * NetHTTP::GetDownloadedData()
{
	if (m_downloadData.empty())
	{
		return NULL; 
	}
	return (const byte*)&m_downloadData[0];
}

int NetHTTP::GetDownloadedBytes()
{
	if (m_pFile || m_bytesWrittenToFile != 0)
	{
		return m_bytesWrittenToFile;
	}
	
	if (m_downloadData.size() == 0) return 0;
	return m_downloadData.size()-1; //the -1 is for the null we added
}

#endif