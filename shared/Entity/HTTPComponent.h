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
example of usage.  OnError or OnFinish will be called

EntityComponent *pComp = pMenu->AddComponent(new HTTPComponent);

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

VariantList v;

string url = "www.dinknetwork.com";
uint32 port = 80;
//GetApp()->GetServerInfo(url, port);

v.m_variant[0].Set(url);
v.m_variant[1].Set(port);
v.m_variant[2].Set("api");
v.m_variant[3].Set(uint32(NetHTTP::END_OF_DATA_SIGNAL_HTTP)); //need this for it to detect a disconnect instead of the weird RTsoft symbol
pComp->GetFunction("Init")->sig_function(&v);
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