//  ***************************************************************
//  AdProvider - Creation date: 05/02/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef AdProvider_h__
#define AdProvider_h__

class AdManager;

//Why hardcode an ad provider type enum? AdManager may provide functionality to use ten ad-servers together based
// on fill rate, so it will want a way to identify them

enum eAdProviderType
{
	AD_PROVIDER_UNKNOWN, //this means we don't care, useful if you add your own custom ad provider
	AD_PROVIDER_CHARTBOOST,
	AD_PROVIDER_FLURRY //more like statistics than ads, but I don't really want to have to make a StaticsManager too, do I?!
};

class AdProvider
{
public:
	AdProvider();
	virtual ~AdProvider();

	virtual bool OnAddToManager(AdManager *pAdManager)=0;
	virtual string GetName()=0;
	virtual void Update()=0;
	virtual bool OnMessage( Message &m ){return false;}; //return true if it was handled and the message shouldn't be passed to anyone else

	virtual void ShowInterstitial(std::string location = "", std::string parm2 = "", std::string parm3 = "") {LogMsg("ShowInterstitial not supported by %s", GetName().c_str());};
	virtual void CacheShowInterstitial(std::string location = "", std::string parm2 = "", std::string parm3 = ""){LogMsg("CacheShowInterstitial not supported by %s", GetName().c_str());};

	virtual void ShowMoreApps(std::string location = "", std::string parm2 = "", std::string parm3 = ""){LogMsg("ShowMoreApps not supported by %s", GetName().c_str());};
	virtual void CacheShowMoreApps(std::string location = "", std::string parm2 = "", std::string parm3 = ""){LogMsg("CacheShowMoreApps not supported by %s", GetName().c_str());};

	virtual void TrackingOnPageView(){}; 
	virtual void TrackingLog(string eventName, string optionalKey = "", string optionalValue = ""){};
    virtual void StartTimedEvent( string eventName, string optionalKey = "", string optionalValue = "" ){};
    virtual void StopTimedEvent( string eventName, string optionalKey = "", string optionalValue = "" ){};

	eAdProviderType GetType() { return m_adProviderType;}

protected:

	eAdProviderType m_adProviderType;

private:
};

#endif // AdProvider_h__
