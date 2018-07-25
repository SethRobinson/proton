#include "PlatformPrecomp.h"

#include "HTTPComponent.h"
#include "BaseApp.h"

#define C_MAX_PREPARE_TRIES 15

HTTPComponent::HTTPComponent()
{
	SetName("HTTP");
}

HTTPComponent::~HTTPComponent()
{
}

void HTTPComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_state = STATE_IDLE;
	m_prepareTryCount = 0;
	//our stuff
	
	//any post var data you want to send, must send it before the Init
	GetFunction("AddPostData")->sig_function.connect(1, boost::bind(&HTTPComponent::AddPostData, this, _1));

	//normally the downloaded file is held in memory, for big files you should use this..
	GetFunction("SetFileOutput")->sig_function.connect(1, boost::bind(&HTTPComponent::SetFileOutput, this, _1));

	//send it server name (string), port (uint32), query (query)
	GetFunction("Init")->sig_function.connect(1, boost::bind(&HTTPComponent::InitAndStart, this, _1));
	
	//You can hook into "OnError" and "OnFinish" to be notified when stuff happens.. search below for those.
	
	//the rest is used internally and should be called by outside processes....
	
	//Example of usage:

	/*


	//Assumes you made a component called WikiScan and called this from within it, but really you can wire the OnFinish/OnError to
	//anything...

	VariantList v;
	v.Get(0).Set("en.wikipedia.org");
	v.Get(1).Set(uint32(80));
	v.Get(2).Set("wiki/Bulletin_Board_System");

	m_pHttpComp->GetFunction("OnFinish")->sig_function.connect(1, boost::bind(&WikiScanComponent::OnPageDownloaded, this, _1));	
	m_pHttpComp->GetFunction("OnError")->sig_function.connect(1, boost::bind(&WikiScanComponent::OnDownloadError, this, _1));	

	m_pHttpComp->GetFunction("Init")->sig_function(&v);


	*/
	
	//used internally
	GetFunction("PrepareConnection")->sig_function.connect(1, boost::bind(&HTTPComponent::PrepareConnection, this, _1));

	//the entities stuff
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&HTTPComponent::OnUpdate, this, _1));

	//register ourselves to get OS messages
	GetBaseApp()->m_sig_os.connect(1, boost::bind(&HTTPComponent::OnOS, this, _1));

}

void HTTPComponent::InitAndStart(VariantList *pVList)
{
	assert(m_state == STATE_FINISHED || m_state == STATE_IDLE);

	if (m_state == STATE_FINISHED)
	{
		//reset everything to do it again
		m_netHTTP.Reset();
	}

	NetHTTP::eEndOfDataSignal endOfDataSignal = NetHTTP::END_OF_DATA_SIGNAL_RTSOFT_MARKER;
	if (pVList->Get(3).GetType() == Variant::TYPE_UINT32)
	{
		//fourth parm holds the end of data type, but we'll have to cast it
		endOfDataSignal = (NetHTTP::eEndOfDataSignal)pVList->Get(3).GetUINT32();
	}

	m_netHTTP.Setup(pVList->m_variant[0].GetString(),pVList->m_variant[1].GetUINT32(),  pVList->m_variant[2].GetString(), endOfDataSignal);
	m_state = STATE_CHECKING_CONNECTION;

	PrepareConnection(NULL);
}

void HTTPComponent::PrepareConnection(VariantList *pVList)
{
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHECK_CONNECTION;
	GetBaseApp()->AddOSMessage(o);
}

void HTTPComponent::AddPostData(VariantList *pVList)
{
	m_netHTTP.AddPostData(pVList->m_variant[0].GetString(), (byte*)pVList->m_variant[1].GetString().c_str(), pVList->m_variant[1].GetString().size());
}

void HTTPComponent::SetFileOutput(VariantList *pVList)
{
	m_fileName = pVList->Get(0).GetString();
}


void HTTPComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void HTTPComponent::OnUpdate(VariantList *pVList)
{
	
	m_netHTTP.Update();

	if (m_state != STATE_CONNECTED) return;

	if (m_netHTTP.GetError() != NetHTTP::ERROR_NONE)
	{
		//send a pointer to us, and the actual error
		m_state = STATE_FINISHED;
        VariantList vList(this, uint32(m_netHTTP.GetError()));
                          
		GetFunction("OnError")->sig_function(&vList);
		return;
	}

	
	if (m_netHTTP.GetState() == NetHTTP::STATE_ACTIVE)
	{
        VariantList vList(this, uint32(m_netHTTP.GetDownloadedBytes()),uint32(m_netHTTP.GetExpectedBytes()));
                          
		GetFunction("OnStatusUpdate")->sig_function(&vList);
	}

	if (m_netHTTP.GetState() == NetHTTP::STATE_FINISHED)
	{
		//all done! send a pointer to us, and the data we made
		if (this == 0) LogMsg("We got probs");
#ifdef _DEBUG
		LogMsg("Downloaded %d bytes", m_netHTTP.GetDownloadedBytes());
#endif
		m_state = STATE_FINISHED;
		if (m_netHTTP.GetDownloadedData())
		{
            VariantList vList(this,Variant((char*)m_netHTTP.GetDownloadedData()));
			GetFunction("OnFinish")->sig_function(&vList);
		} else
		{
            VariantList vList(this,Variant(""));
			GetFunction("OnFinish")->sig_function(&vList);
		}
	}
}

void HTTPComponent::OnOS(VariantList *pVList)
{
	eMessageType mType = (eMessageType)(int)pVList->m_variant[0].GetFloat();

	if (m_state == STATE_FINISHED) return;

	switch (mType)
	{
	case MESSAGE_TYPE_OS_CONNECTION_CHECKED:
		eOSSTreamEvent event = (eOSSTreamEvent)(int) pVList->m_variant[1].GetVector2().x;
		//LogMsg("Connection is %d", event);

		switch (event)
		{
		case RT_kCFStreamEventOpenCompleted:
			
			//should we also check this?  I guess we don't have to, our connection is good already and we don't care via what

			/*
			eNetworkType type = IsNetReachable("rtsoft.com");

			//this
			if (type == C_NETWORK_NONE)
			{
			ShowError(pMenu, "Network is currently unavailable.");
			return;
			}
			*/
#ifdef _DEBUG
			LogMsg("Initiating real connection");
#endif
			
			m_state = STATE_CONNECTED;
			m_netHTTP.Start();
			if (!m_fileName.empty())
			{
				m_netHTTP.SetFileOutput(m_fileName);
			}

			break;

		case RT_kCFStreamEventErrorOccurred:
			//error.  But maybe it's just going through the process of warming up the wi-fi?  Try again in a bit.
			//try again in a bit
			
			if (m_prepareTryCount > C_MAX_PREPARE_TRIES)
			{
                VariantList vList(this, uint32(event));
				GetFunction("OnError")->sig_function(&vList);
				return;
			}
			GetMessageManager()->CallComponentFunction(this, 1000, "PrepareConnection");
			m_prepareTryCount++;
			break;


		default:
            {
                VariantList vList(this, uint32(event));
                GetFunction("OnError")->sig_function(&vList);
            }
			break;
		}
	
		break;
	}
}