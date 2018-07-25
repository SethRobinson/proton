//  ***************************************************************
//  AdProviderFlurry - Creation date: 06/26/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AdProviderFlurry_h__
#define AdProviderFlurry_h__

#include "AdProvider.h"

class AdProviderFlurry : public AdProvider
{
public:
	AdProviderFlurry();
	virtual ~AdProviderFlurry();

	//overriding API stuff
	virtual bool OnAddToManager(AdManager *pAdManager);
	virtual string GetName() {return "Flurry";}
	virtual void Update() {};
	virtual bool OnMessage( Message &m ); //return true if it was handled and the message shouldn't be passed to anyone else

	virtual void TrackingOnPageView();
	virtual void TrackingLog(string eventName, string optionalKey = "", string optionalValue = "");
	virtual void StartTimedEvent(string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/);
	virtual void StopTimedEvent(string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/);

	//custom functions chartboost specific
	void SetupInfo(const string flurryAPIkey);




private:

	string m_apiKey;
};

#endif // AdProviderFlurry_h__
