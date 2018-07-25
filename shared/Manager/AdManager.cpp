#include "PlatformPrecomp.h"
#include "AdManager.h"

AdManager::AdManager()
{
	m_bTapjoyAdReady = false;
	m_updateTimer = 0;
	m_bSendTapjoyAdRequestASAP = false;
	m_bSendTapjoyFeaturedAppRequestASAP = false;
	m_bShowTapjoyFeaturedAppASAP = false;
	m_errorCount= 0;
	m_tapPoints = -1; //-1 means don't know
	m_bUsingTapPoints= false;
	m_tapPointVariant.Set(m_tapPoints);
	m_returnState = RETURN_STATE_NONE;
	m_bShowTapjoyAdASAP = false;
	m_bShowingAd = false;
	m_vBannerSize = CL_Vec2f(640, 100);
	m_desiredBannerAlignment = ALIGNMENT_DOWN_CENTER;
	
}

AdManager::~AdManager()
{
	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		delete *itor;
	}
	m_providers.clear();
}

void AdManager::AddProvider( AdProvider *provider )
{
	m_providers.push_back(provider);

	if (!provider->OnAddToManager(this))
	{
		LogError("Unable to init ad provider %s, killing it", provider->GetName().c_str());
		SAFE_DELETE(provider);
		return;
	} else
	{
		LogMsg("Ad provider %s initialized.", provider->GetName().c_str());
	}
}

AdProvider * AdManager::GetProviderByType( eAdProviderType type )
{
	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		if ( (*itor)->GetType() == type)
		{
			return (*itor);
		}
	}

	return NULL;
}

void AdManager::SetTapjoyFeatureAppVisible( bool bVisible )
{
	if (bVisible)
	{
		if (m_bTapjoyFeaturedAppReady)
		{
			LogMsg("Showing featured app!");
			OSMessage o;
			o.m_type = OSMessage::MESSAGE_TAPJOY_SHOW_FEATURED_APP;
			o.m_x = 1; //show ad
			GetBaseApp()->AddOSMessage(o);
	
		} else
		{
			//not quite ready yet
			m_bShowTapjoyFeaturedAppASAP = true;
		}
	} else
	{
		//don't show it, if we're waiting..
		m_bSendTapjoyFeaturedAppRequestASAP = false;
	}
}

void AdManager::SetTapjoyAdVisible(bool bVisible)
{
	m_bShowingAd = bVisible;

	if (bVisible)
	{
		if (m_bTapjoyAdReady)
		{
			OSMessage o;
			o.m_type = OSMessage::MESSAGE_TAPJOY_SHOW_AD;
			o.m_x = 1; //show ad
			GetBaseApp()->AddOSMessage(o);
      
		} else
		{
			//do it after it becomes ready
			m_bShowTapjoyAdASAP = true;
	
            //it's possible no ad is ready, so we'll make sure one is loaded
            m_bSendTapjoyAdRequestASAP = true;
        }

	} else
	{
		m_bShowTapjoyAdASAP = false;
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_TAPJOY_SHOW_AD;
		o.m_x = 0; //stop showing ad
		GetBaseApp()->AddOSMessage(o);
	}
}

void AdManager::OpenTapjoyOfferWall()
{
#ifdef _DEBUG
	LogMsg("Sending message to Android to open the TJ offerwall");
#endif

	OSMessage o;
	o.m_type = OSMessage::MESSAGE_TAPJOY_SHOW_OFFERS;
	GetBaseApp()->AddOSMessage(o);
}

void AdManager::CacheTapjoyAd()
{
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_TAPJOY_GET_AD;
	GetBaseApp()->AddOSMessage(o);
	//m_bTapjoyAdReady = false; //It might be dangerous to try to show it while we're loading it.. unsure.
	m_bSendTapjoyAdRequestASAP = false;
}

void AdManager::CacheTapjoyFeaturedApp(string currencyID) //or blank for default
{
	OSMessage o;
	o.m_string = currencyID;
	o.m_type = OSMessage::MESSAGE_TAPJOY_GET_FEATURED_APP;
	GetBaseApp()->AddOSMessage(o);
	m_bSendTapjoyFeaturedAppRequestASAP = false;
}

void AdManager::OnMessage( Message &m )
{

	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		if ( (*itor)->OnMessage(m))
		{
			//it signalled that it handled it and we shouldn't continue processing
			return;
		}
	}

	if (m_returnState == RETURN_STATE_WAITING)
	{
		switch (m.GetType())
		{
		case MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN:
			m_returnState = RETURN_STATE_SUCCESS;
			ClearError();
			break;

		case MESSAGE_TYPE_TAPJOY_SPEND_TAP_POINTS_RETURN_ERROR:
			m_returnState = RETURN_STATE_ERROR;
			m_lastError = m.GetStringParm();

			break;
                
            default: ;
		}
	}

	switch (m.GetType())
	{

	
	case MESSAGE_TYPE_TAPJOY_EARNED_TAP_POINTS:
		{
#ifdef _DEBUG
			LogMsg("We just got %d tappoints!", (int)m.GetParm1());
#endif
			VariantList vList((int32)m.GetParm1());

			m_sig_tappoints_awarded(&vList); //called when awarded tap points
		}

		break;

	case MESSAGE_TYPE_TAPJOY_OFFERWALL_CLOSED:
		{
	#ifdef _DEBUG
				LogMsg("Offer wall was closed by user. (iOS only right now)");
	#endif
			m_sig_offer_wall_closed(NULL); //called when awarded tap points
		}

		break;

	case MESSAGE_TYPE_TAPJOY_TAP_POINTS_RETURN:
		
		m_tapPoints = (int32)m.GetParm1();
		m_tapPointVariant.Set(m_tapPoints);
		m_tapCurrency = m.GetStringParm();
#ifdef _DEBUG
		LogMsg("Tap points set to %d %s", m_tapPoints, m_tapCurrency.c_str());
#endif
		break;

	case MESSAGE_TYPE_TAPJOY_MOVIE_AD_READY:
#ifdef _DEBUG
		if (m.GetParm1() == 1)
		{
			LogMsg("Video ad is ready!");
		} else
		{
			LogMsg("Video ad failed %d - %s", m.GetParm2(), m.GetStringParm().c_str());
		}
#endif
		break;
		case MESSAGE_TYPE_TAPJOY_FEATURED_APP_READY:

			if (m.GetParm1() == 1)
			{
				//LogMsg("Featured app is ready");
				m_bTapjoyFeaturedAppReady = true;
				
				if (m_bShowTapjoyFeaturedAppASAP)
				{
					//LogMsg("Showing featured now");

					SetTapjoyFeatureAppVisible(true);
				}
			} else
			{
				m_errorCount++;
				//error
#ifdef _DEBUG
				LogMsg("Error getting featured app: %s", m.GetStringParm().c_str());
#endif		
				if (m.GetStringParm().find("exceeded display") != string::npos)
				{
					//LogMsg("Displayed all front page ads we can get, not showing any more.");
					SetTapjoyFeatureAppVisible(false);
				} else
				{
					m_bSendTapjoyFeaturedAppRequestASAP = true;
					m_bTapjoyFeaturedAppReady = false;

				}
			}
		
		break;

	case MESSAGE_TYPE_TAPJOY_AD_READY:
		if (m.GetParm1() == 1)
		{
#ifdef _DEBUG
			LogMsg("Tapjoy ad is ready");
#endif
			m_bTapjoyAdReady = true;

			if (m_bShowTapjoyAdASAP)
			{
				SetTapjoyAdVisible(true);
			}
		} else
		{
			//LogMsg("trying Tapjoy get ad, it had an error");
			m_bSendTapjoyAdRequestASAP = true;
			m_bTapjoyAdReady = false;
			m_errorCount++;

		}
		break;
        default: ;
	}
}

void AdManager::Update()
{

	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		(*itor)->Update();
	}

	if (m_errorCount > 15) return; //something is seriously wrong, let's not risk hammering the tapjoy service forever

	if (m_updateTimer >= GetTick(TIMER_SYSTEM)) return;
	m_updateTimer = GetTick(TIMER_SYSTEM)+ 1000;

	if (m_bSendTapjoyAdRequestASAP)
	{
		CacheTapjoyAd();
	}

	if (m_bSendTapjoyFeaturedAppRequestASAP)
	{
		CacheTapjoyFeaturedApp();
	}

}

void AdManager::GetTapPointsFromServer()
{
	if (m_bUsingTapPoints)
	{
#ifdef _DEBUG
		LogMsg("Requesting latest info from Tapjoy");
#endif
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_TAPJOY_GET_TAP_POINTS;
		GetBaseApp()->AddOSMessage(o);
	}
}

void AdManager::Init()
{
#ifdef _DEBUG
	LogMsg("AdManager initted.");
#endif
}

std::string AdManager::GetPointsString()
{
	if (m_tapPoints == -1)
	{
		return "Offline";
	}

	return toString(m_tapPoints)+" "+m_tapCurrency;
}

void AdManager::SetUsingTapPoints( bool bNew )
{
	if (bNew == m_bUsingTapPoints) return;

	assert(bNew && "Why would you turn this off?");

	m_bUsingTapPoints = bNew;

}
string AdManager::GetLastErrorString()
{
	return m_lastError;
}

void AdManager::ClearError()
{
	m_lastError.clear();
}

AdManager::eReturnState AdManager::GetReturnState()
{
	return m_returnState;
}

bool AdManager::IsReadyForTransaction()
{
	return !m_tapCurrency.empty();
}

void AdManager::ModifyTapPoints( int mod )
{
	m_returnState = RETURN_STATE_WAITING;
	ClearError();

	if (mod < 0)
	{
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_TAPJOY_SPEND_TAP_POINTS;
		o.m_parm1 = abs(mod); //turn it positive
		GetBaseApp()->AddOSMessage(o);
	} else
	{
		OSMessage o;
		o.m_type = OSMessage::MESSAGE_TAPJOY_AWARD_TAP_POINTS;
		o.m_parm1 = mod;
		GetBaseApp()->AddOSMessage(o);
	}
}

void AdManager::OnRender()
{

#ifdef WIN32
	if (m_bShowingAd)
	{
		//draw a fake rectangle the same size as the real ad will be, this is just for testing, so you get an idea
		//of how big the ad when developing on Windows, and when it will pop up

		CL_Vec2f vRatio = CL_Vec2f(1,1);
		if (GetFakePrimaryScreenSizeX() != 0)
		{
			vRatio.x = (GetScreenSizeXf()/float(GetOriginalScreenSizeX()));
			vRatio.y =(GetScreenSizeYf()/float(GetOriginalScreenSizeY()));
		}
		rtRect r(0,0, (int)(m_vBannerSize.x*vRatio.x),(int)(m_vBannerSize.y*vRatio.y));
		
		//move to bottom
		r.AdjustPosition(0, GetScreenSizeY()-r.GetHeight());

		//center
		r.AdjustPosition( (GetScreenSizeX()-r.GetWidth())/2, 0 );
		
		DrawFilledRect(r, MAKE_RGBA(40,255,40,200));
	}
#endif

}

void AdManager::SetupBanner(CL_Vec2f vBannerSize, eAlignment alignment /*= ALIGNMENT_DOWN_CENTER*/ )
{
	m_desiredBannerAlignment = alignment;
	m_vBannerSize = vBannerSize;
	
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_REQUEST_AD_SIZE;
	o.m_x = m_vBannerSize.x;
	o.m_y = m_vBannerSize.y;
	o.m_parm1 = m_desiredBannerAlignment; //not actually respected yet

	GetBaseApp()->AddOSMessage(o);
	
}

bool AdManager::ProviderExistsByType( eAdProviderType type )
{
	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		if ( (*itor)->GetType() == type)
		{
			return true;
		}
	}

	return false;	
}

void AdManager::TrackingLog( string eventName, string optionalKey /*= ""*/, string optionalValue /*= ""*/ )
{
	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		(*itor)->TrackingLog(eventName, optionalKey, optionalValue);
	}
}

void AdManager::TrackingOnPageView()
{
	list<AdProvider*>::iterator itor = m_providers.begin();
	for (;itor != m_providers.end(); itor++)
	{
		(*itor)->TrackingOnPageView();
	}
}

void AdManager::InitTapjoy( string tapjoyID, string tapjoyAppSecretKey )
{

	//Let the individual platform handle it.  The android side will ignore if it is already initted via Main.java, which was the old way.  (So old apps don't need to be changed)
#ifdef _DEBUG
    LogMsg("Sending Tapjoy init message");
#endif
    OSMessage o;
	o.m_type = OSMessage::MESSAGE_TAPJOY_INIT_MAIN;
	o.m_string = tapjoyID;
	o.m_string2 = tapjoyAppSecretKey;
	GetBaseApp()->AddOSMessage(o);

}


void AdManager::SetUserID( string userID)
{
	
#ifdef _DEBUG
	LogMsg("Sending Tapjoy user id message");
#endif
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_TAPJOY_SET_USERID;
	o.m_string = userID;
	GetBaseApp()->AddOSMessage(o);
}

