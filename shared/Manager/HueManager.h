//  ***************************************************************
//  HueManager - Creation date: 8/1/2023 3:00:57 PM
//  -------------------------------------------------------------
//  License: Uh, check for license.txt or license.md for that?
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include "hueplusplus/Bridge.h"
#include "hueplusplus/WinHttpHandler.h"


class HueManager
{
public:
	HueManager();
	virtual ~HueManager();

	void Update();
	bool Init(string bridgeIP, string userName);

	void SetLightRGB(string lightName, string rgb, bool bAllowPartialMatch);
	vector<int> GetLightIDsByName(string name, bool bAllowPartialMatch);

protected:

	std::vector<hueplusplus::BridgeFinder::BridgeIdentification> m_bridges;
	std::shared_ptr<hueplusplus::WinHttpHandler> handler;
	std::vector<hueplusplus::Light> lights;
	hueplusplus::Bridge *m_pBridge = NULL;
	bool m_lightsInitted = false;

};

void SetHueLightRGB(VariantList* pVList);
