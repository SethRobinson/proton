//  ***************************************************************
//  AdManager - Creation date: 09/30/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*

Seth's class to control Tapjoy and other ad providers.

See the RTAdTest example.

Here is how to add it to an existing project:

1.  Add #include "Manager/AdManager.h" to your App.h
2.  Add AdManager m_adManager; as a member variable in your App class
3.  Add m_adManager.Init(); so it gets called once at startup somewhere
4.  Add m_adManager.Update(); to your App::Update() function
5.  Add m_adManager.OnRender(); the the BOTTOM of your App::Draw() function, this is to show fake ads when testing from Windows
6.  Override BaseApp::OnMessage in your App class and add m_adManager.OnMessage(m);
	Be sure to also call the base version, example:
	void App::OnMessage( Message &m )
	{
		m_adManager.OnMessage(m);
		BaseApp::OnMessage(m);
	}
*/

#ifndef AdManager_h__
#define AdManager_h__

#include "Ad/AdProvider.h"

class AdManager
{
public:
	
	enum eReturnState
	{
		RETURN_STATE_NONE,
		RETURN_STATE_WAITING,
		RETURN_STATE_ERROR,
		RETURN_STATE_SUCCESS

	};
	
	AdManager();
	virtual ~AdManager();

	//general

	void Init();	//call this at start of app, will cache ads to get them ready
	void Update(); //call this from your App::Update
	void OnMessage(Message &m); //call this from your App::OnMessage(Message &m);
	Variant m_tapPointVariant; //can hook onto this to get updated when it changes if needed, using sigslots
	string GetLastErrorString();
	void ClearError();
	eReturnState GetReturnState();
	bool IsReadyForTransaction(); //only need to check this to give/remove tap points
	void OnRender(); //currently only used to render a fake rect on windows to show where the ad would be.  (ads only display in Android)

	void AddProvider(AdProvider *provider);
	AdProvider * GetProviderByType(eAdProviderType type);
	bool ProviderExistsByType(eAdProviderType type);
	
	void TrackingOnPageView(); //handled by Flurry or other trackers
	void TrackingLog(string eventName, string optionalKey = "", string optionalValue = "");

	//Tapjoy specific, supported on Android and iOS

	void InitTapjoy(string tapjoyID, string tapjoyAppSecretKey);
	void SetUserID(string userID); //if using un-managed currency, you need to send this so it knows who to give gems to on the callback
	void SetTapjoyAdVisible(bool bVisible); //will load an ad if not cached, so it might not display right away
	void SetTapjoyFeatureAppVisible(bool bVisible); //will load an ad if not cached, so it might not display right away
	void CacheTapjoyAd();
	void CacheTapjoyFeaturedApp(string currencyID = string()); //this was actually retrofitted to work for the TJ video offers
	bool IsTapJoyAdReady() {return m_bTapjoyAdReady;}
	string GetPointsString();
	void OpenTapjoyOfferWall();
	void SetUsingTapPoints(bool bNew);
	void ModifyTapPoints(int mod);
	void GetTapPointsFromServer(); //calling this also causes any "hey, you got tapjoy points" notifications to be sent
	void SetupBanner(CL_Vec2f vBannerSize, eAlignment alignment = ALIGNMENT_DOWN_CENTER); //alignment is ignored from now, always bottom centered

	boost::signal<void (VariantList*)> m_sig_tappoints_awarded; //called when awarded tap points
	boost::signal<void (VariantList*)> m_sig_offer_wall_closed; //ios only

private:

	eReturnState m_returnState;
	bool m_bTapjoyAdReady;
	bool m_bTapjoyFeaturedAppReady;

	bool m_bShowTapjoyAdASAP;
	bool m_bShowTapjoyFeaturedAppASAP;

	unsigned int m_updateTimer;
	bool m_bSendTapjoyAdRequestASAP;
	bool m_bSendTapjoyFeaturedAppRequestASAP;

	int m_errorCount;
	int32 m_tapPoints; //currency used with tapjoy
	bool m_bUsingTapPoints;
	string m_tapCurrency;
	string m_lastError;
	bool m_bShowingAd;
	eAlignment m_desiredBannerAlignment;
	CL_Vec2f m_vBannerSize;

	list<AdProvider*> m_providers;

};

#endif // AdManager_h__