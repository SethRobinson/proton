/*
 *  App.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once
#include "BaseApp.h"
#include "Manager/AdManager.h"

//note: This is set in  the ReleaseBeta GL configuration, not here
 //#define RT_IS_BETA 1

enum eExtendedVirtualKeys
{
	VIRTUAL_KEY_CUSTOM_QUIT = VIRTUAL_KEY_CUSTOM_START

};
#ifdef RT_SCRIPT_BUILD

 //this is being build from the command line, enable special flags here
#endif

class GamepadProvider;

enum eControlStyle
{
	CONTROLS_JOYPAD,
	CONTROLS_DRAG_ANYWHERE,
	CONTROLS_FLING
};

enum eVideoFPS
{
	VIDEO_FPS_LIMIT_ON,
	VIDEO_FPS_LIMIT_OFF
};



#ifndef __APPLE__
//#define RT_EXPIRING
#endif

//#define RT_CHARTBOOST_ENABLED


class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool Init();
	virtual void Kill();
	virtual bool OnPreInitVideo();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void Update();
	virtual void OnMemoryWarning();
	virtual void OnEnterBackground();
	virtual void OnEnterForeground();

	string GetVersionString();
	float GetVersion();
	int GetBuild();
	void GetServerInfo(string &server, uint32 &port);
	VariantDB * GetShared() {return &m_varDB;}
	Variant * GetVar(const string &keyName );
	Variant * GetVarWithDefault(const string &varName, const Variant &var) {return m_varDB.GetVarWithDefault(varName, var);}
	int GetSpecial();
	void UpdateVideoSettings();
	void SaveSettings();
	void SetUsingTouchScreen(bool bNew) { m_bUsingTouchScreen = bNew; }
	bool GetUsingTouchScreen() {return m_bUsingTouchScreen;}
	bool CanDownloadDMODS() {return m_bHasDMODSupport;}
	void SetCheatsEnabled(bool bCheatsEnabled) {m_bCheatsEnabled = bCheatsEnabled;}
	bool GetCheatsEnabled() {return m_bCheatsEnabled;}
	void OnExitApp(VariantList *pVarList);
	bool GetIconsOnLeft();
	void RemoveAndAttachAllAvailableGamepads();

	AdManager * GetAdManager() {return &m_adManager;}

	void OnMessage( Message &m );
	void OnLoadSurfaces();
	void OnUnloadSurfaces();
	void AddTextToLog(const char *tex, const char *filename);
	void AddDroidKeyboardKeys();
	void RemoveAndroidKeyboardKeys();
	void AddIcadeProvider();
	bool GetForceAspectRatio();
	bool GetGhostMode() {return m_bGhostMode;}
	void SetGhostMode(bool bNew) { m_bGhostMode = bNew; }
	bool UseClassicEscapeMenu();
	vector<string> * GetReferenceToCommandLineParms() { return &m_commandLineParms; }


private:

	AdManager m_adManager;

	bool m_bDidPostInit;
	VariantDB m_varDB; //holds all data we want to save/load
	int m_special;
	bool m_bUsingTouchScreen;
	bool m_bForceAspectRatio;
	bool m_bGhostMode; //lets dink walk anywhere and ignore screenlocks

	void SaveAllData();
	void OnPreEnterBackground(VariantList *pVList);
	void OniCadeDisconnected(GamepadProvider *pProvider);
	bool DoesCommandLineParmExist(string parm);
	float m_version;
	string m_versionString;
	int m_build;

	float m_desktopVersion;
	string m_desktopVersionString;
	int m_desktopBuild;
	bool m_bHasDMODSupport;
	bool m_bCheatsEnabled;
	FILE *   m_logFileHandle;


};


App * GetApp();
const char * GetAppName();
const char * GetBundleName();
const char * GetBundlePrefix();
bool TouchesHaveBeenReceived();