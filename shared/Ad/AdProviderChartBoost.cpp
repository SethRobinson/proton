#include "PlatformPrecomp.h"
#include "AdProviderChartBoost.h"
#include "Manager/MessageManager.h"

#ifdef PLATFORM_IOS
#include "AdProviderIOS_ChartBoost.h"
#endif

AdProviderChartBoost::AdProviderChartBoost()
{
	m_adProviderType = AD_PROVIDER_CHARTBOOST;
}

AdProviderChartBoost::~AdProviderChartBoost()
{
}

bool AdProviderChartBoost::OnAddToManager( AdManager *pAdManager )
{
	if (m_appID.empty())
	{
		LogError("You must call AdProviderChartBoost::SetupInfo with the info you get from ChartBoost before you add this to the manager!");
		return false;
	}

#ifdef PLATFORM_IOS
	ChartBoostIOS_StartSession(m_appID, m_appSignature);
#endif
#ifdef _DEBUG
	LogMsg("AdProviderChartBoost - Initting");
#endif

	//Android listens for this, iOS version doesn't care
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHARTBOOST_SETUP;
	o.m_string = m_appID;
	o.m_string2 = m_appSignature;
	GetBaseApp()->AddOSMessage(o);

	return true;
}

void AdProviderChartBoost::Update()
{
}

void AdProviderChartBoost::SetupInfo( const string appID, const string appSignature )
{
	m_appID = appID;
	m_appSignature = appSignature;
}

void AdProviderChartBoost::ShowInterstitial( std::string location /*= ""*/, std::string parm2 /*= ""*/, std::string parm3 /*= ""*/ )
{
#ifdef PLATFORM_IOS
	ChartBoostIOS_ShowInterstitial(location);
#endif
#ifdef _DEBUG
	LogMsg("AdProviderChartBoost::ShowInterstitial");
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHARTBOOST_SHOW_INTERSTITIAL;
	o.m_string = location;
	GetBaseApp()->AddOSMessage(o);

}

void AdProviderChartBoost::CacheShowInterstitial( std::string location /*= ""*/, std::string parm2 /*= ""*/, std::string parm3 /*= ""*/ )
{
#ifdef PLATFORM_IOS
	ChartBoostIOS_CacheShowInterstitial(location);
#endif

#ifdef _DEBUG
	LogMsg("AdProviderChartBoost::CacheShowInterstitial");
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHARTBOOST_CACHE_INTERSTITIAL;
	o.m_string = location;
	GetBaseApp()->AddOSMessage(o);
}

void AdProviderChartBoost::ShowMoreApps( std::string location /*= ""*/, std::string parm2 /*= ""*/, std::string parm3 /*= ""*/ )
{
#ifdef PLATFORM_IOS
    ChartBoostIOS_ShowMoreApps();
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHARTBOOST_SHOW_MORE_APPS;
	o.m_string = location;
	GetBaseApp()->AddOSMessage(o);

}

void AdProviderChartBoost::CacheShowMoreApps( std::string location /*= ""*/, std::string parm2 /*= ""*/, std::string parm3 /*= ""*/ )
{
#ifdef PLATFORM_IOS
	ChartBoostIOS_CacheShowMoreApps();
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_CHARTBOOST_CACHE_MORE_APPS;
	o.m_string = location;
	GetBaseApp()->AddOSMessage(o);


}

bool AdProviderChartBoost::OnMessage( Message &m )
{

	return false; //we didn't handle it, so keep processing
}