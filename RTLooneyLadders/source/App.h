/*
 *  App.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

/*

This game was originally made for the Ludumdare 48 hour game making competition, so yeah, don't expect the best coding
practices...

-Seth

*/

#pragma once

#include "BaseApp.h"

#ifdef PLATFORM_ANDROID
	//Moga controller is still under NDA, so even if you try to use it isn't functional in Proton's public SVN yet.  Coming soon
	//#define RT_MOGA_ENABLED
#endif

//this define will cause windows builds to ignore the settings in main.cpp and force 1024X768
#define C_FORCE_BIG_SCREEN_SIZE_FOR_WINDOWS_BUILDS
#define TOTAL_LEVELS 6

#include "Gamepad/Gamepad.h"

class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool Init();
	virtual void Kill();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void Update();
	virtual bool OnPreInitVideo();

	string GetVersionString();
	float GetVersion();
	int GetBuild();
	void GetServerInfo(string &server, uint32 &port);
	VariantDB * GetShared() {return &m_varDB;}
	Variant * GetVar(const string &keyName );
	Variant * GetVarWithDefault(const string &varName, const Variant &var) {return m_varDB.GetVarWithDefault(varName, var);}
	int GetSpecial();
	void OnExitApp(VariantList *pVarList);
	void OnGamepadConnected(Gamepad* pPad);
	void OnGamepadDisconnected(eGamepadID id);
private:

	bool m_bDidPostInit;
	VariantDB m_varDB; //holds all data we want to save/load
};

extern App g_App;

App * GetApp();
const char * GetAppName();
const char * GetBundleName();
const char * GetBundlePrefix();
void AttachGamepadsIfPossible();