/*
 *  App.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once

#include "BaseApp.h"

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
	virtual void OnEnterBackground();
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

private:

	void SaveOurStuff();

	bool m_bDidPostInit;
	VariantDB m_varDB; //holds all data we want to save/load
	int m_special;
};
 

extern App g_App;

App * GetApp();
const char * GetAppName();
const char * GetBundleName();
const char * GetBundlePrefix();
