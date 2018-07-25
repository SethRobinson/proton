//  ***************************************************************
//  AdProviderChartBoost - Creation date: 05/02/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*

This is only setup for iOS and Android, although it will compile in Win.

To compile for iOS, you need to download their SDK and put it in /shared/iOS/ChartBoost

(You'll see I already made a dir there with a readme)

You'll need to add the its .a file to your project.


For Android, you'll need to add the .lib to /shared/android/optional_src/libs/Flurry

If you don't see any ads/menus pop up, it's probably not setup right in your ChartBoost control panel, it doesn't give ANY
log messages to let you know what's going on.

See the RTAdTest example for a usage example
*/

#ifndef AdProviderChartBoost_h__
#define AdProviderChartBoost_h__

#include "AdProvider.h"

class AdProviderChartBoost : public AdProvider
{
public:
	AdProviderChartBoost();
	virtual ~AdProviderChartBoost();

	//overriding API stuff
	virtual bool OnAddToManager(AdManager *pAdManager);
	virtual string GetName() {return "ChartBoost";}
	virtual void Update();
	virtual bool OnMessage( Message &m ); //return true if it was handled and the message shouldn't be passed to anyone else

	virtual void ShowInterstitial(std::string location = "", std::string parm2 = "", std::string parm3 = "");
	virtual void CacheShowInterstitial(std::string location = "", std::string parm2 = "", std::string parm3 = "");

	virtual void ShowMoreApps(std::string location = "", std::string parm2 = "", std::string parm3 = "");
	virtual void CacheShowMoreApps(std::string location = "", std::string parm2 = "", std::string parm3 = "");

	//custom functions chartboost specific
	void SetupInfo(const string appID, const string appSignature);

protected:

	string m_appID;
	string m_appSignature;
};

#endif // AdProviderChartBoost_h__