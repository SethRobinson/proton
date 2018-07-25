#include "PlatformPrecomp.h"
#include "AdProviderFlurry.h"
#include "Manager/MessageManager.h"
//#include "GUI/GenericDialogMenu.h"

AdProviderFlurry::AdProviderFlurry()
{
	m_adProviderType = AD_PROVIDER_FLURRY;
}

AdProviderFlurry::~AdProviderFlurry()
{
}

void AdProviderFlurry::SetupInfo( const string flurryAPIkey )
{
	m_apiKey = flurryAPIkey;
}

bool AdProviderFlurry::OnAddToManager( AdManager *pAdManager )
{
	if (m_apiKey.empty())
	{
		LogError("You must call AdProviderFlurry::SetupInfo with the info you get from Flurry before you add this to the manager!");
		return false;
	}

	LogMsg("AdProviderFlurry - Initting");

	//Android listens for this, iOS version doesn't care
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FLURRY_SETUP;
	o.m_string = m_apiKey;
	GetBaseApp()->AddOSMessage(o);
	return true;
}

bool AdProviderFlurry::OnMessage( Message &m )
{
	return false; //we didn't handle it, so keep processing
}

void AdProviderFlurry::TrackingOnPageView()
{
	//Android listens for this, iOS version doesn't care
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FLURRY_ON_PAGE_VIEW;
	GetBaseApp()->AddOSMessage(o);
	
}

void AdProviderFlurry::TrackingLog( string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/ )
{
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FLURRY_LOG_EVENT;
	o.m_string = eventName;
	o.m_string2 = optionalKey;
	o.m_string3 = optionalValue;
	GetBaseApp()->AddOSMessage(o);
	
}

void AdProviderFlurry::StartTimedEvent( string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/ )
{
    OSMessage o;
    o.m_type = OSMessage::MESSAGE_FLURRY_START_TIMED_EVENT;
    o.m_string = eventName;
    o.m_string2 = optionalKey;
    o.m_string3 = optionalValue;
    GetBaseApp()->AddOSMessage(o);
    
}

void AdProviderFlurry::StopTimedEvent( string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/ )
{
    OSMessage o;
    o.m_type = OSMessage::MESSAGE_FLURRY_STOP_TIMED_EVENT;
    o.m_string = eventName;
    o.m_string2 = optionalKey;
    o.m_string3 = optionalValue;
    GetBaseApp()->AddOSMessage(o);
    
}
