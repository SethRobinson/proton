//  ***************************************************************
//  HTTPComponent - Creation date: 06/06/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef HTTPComponent_h__
#define HTTPComponent_h__

#include "Component.h"
#include "Network/NetHTTP.h"

/* 
example of usage.  OnError or OnFinish will be called if setup.



void OnDownloadError(VariantList* pVList)
{
	NetHTTP::eError e = (NetHTTP::eError)pVList->m_variant[1].GetUINT32();

	string msg = "`4Unable to connect.  Try later. (" + toString(e) + ")";
	if (e == NetHTTP::ERROR_COMMUNICATION_TIMEOUT)
	{
		msg = "`4Connection timed out. Try Later.";
	}
	ShowTextMessageSimple(msg, 0);
}


void OnDownloadHTTPFinish(VariantList* pVList)
{
	Entity* pMenu = pVList->m_variant[0].GetComponent()->GetParent();

	LogMsg(pVList->m_variant[1].GetString().c_str());
}

....

Entity* pEntity = new Entity("FoobarCommand");

			GetEntityRoot()->AddEntity(pEntity);
			//add a HTTPComponent
			EntityComponent * pComp = pEntity->AddComponent(new HTTPComponent);
			AddFocusIfNeeded(pEntity);
			VariantList v;

			//Optionally can set post data to send.  This will force "post" mode.  If you need to POST without any data (can happen in some RESTful apis), do this:

			((HTTPComponent*)pComp)->GetNetHTTP()->SetForcePost(true);

			VariantList vPostData;

			vPostData.m_variant[0].Set("version");
			vPostData.m_variant[1].Set(toString(GetApp()->GetVersion()));
			pComp->GetFunction("AddPostData")->sig_function(&vPostData);

			vPostData.m_variant[0].Set("build");
			vPostData.m_variant[1].Set(toString(GetApp()->GetBuild()));
			pComp->GetFunction("AddPostData")->sig_function(&vPostData);

			vPostData.m_variant[0].Set("platform");
			vPostData.m_variant[1].Set(toString(GetEmulatedPlatformID()));
			pComp->GetFunction("AddPostData")->sig_function(&vPostData);

			v.Reset();

			string url = "127.0.0.1";
			uint32 port = 8880;

			v.m_variant[0].Set(url);
			v.m_variant[1].Set(port);
			v.m_variant[2].Set("api/player/pause");
			v.m_variant[3].Set(uint32(NetHTTP::END_OF_DATA_SIGNAL_HTTP)); //need this for it to detect a disconnect instead of the weird RTsoft symbol
			
			pComp->GetFunction("Init")->sig_function(&v);  (or, to schedule the call later, do: GetMessageManager()->CallComponentFunction(pComp, timeMS, "Init", &v);
			
			pComp->GetFunction("OnError")->sig_function.connect(&OnDownloadError);
			pComp->GetFunction("OnFinish")->sig_function.connect(&OnDownloadHTTPFinish);

*/

class HTTPComponent: public EntityComponent
{
public:
	HTTPComponent();
	virtual ~HTTPComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eState
	{
		STATE_IDLE,
		STATE_CHECKING_CONNECTION,
		STATE_CONNECTED,
		STATE_FINISHED
	};

	NetHTTP* GetNetHTTP() { return &m_netHTTP; } //allow direct access if needed

private:

	//our stuff
	void OnUpdate(VariantList *pVList);
	void InitAndStart(VariantList *pVList);
	void AddPostData(VariantList *pVList);
	void OnOS(VariantList *pVList);
	void PrepareConnection(VariantList *pVList);
	void SetFileOutput(VariantList *pVList);

	int m_prepareTryCount;
	NetHTTP m_netHTTP;
	eState m_state;
	string m_fileName;
};

#endif // HTTPComponent_h__