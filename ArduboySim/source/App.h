/*
 *  App.h
 *  Created by Seth Robinson
 *  
 *
 */

#pragma once

#include "BaseApp.h"


enum eMode
{
	MODE_SALES,
	MODE_CHAT,
	MODE_IDLE
};
void OnTouchEvent(eMessageType msg, float x, float y);
void SetBlankIgnoreIfUnset(int ms);
void SetBlankIgnore(int ms);
void SetMainMode(eMode mode);
void SetTextDelay(int timeMS);
extern Surface g_salesBackgroundSurf;
extern Surface g_chatBackgroundSurf;
extern Surface g_screenSurf;
extern RTFont g_font;
extern RTFont g_big_font;
extern unsigned int g_intervalStartTime;
extern unsigned int g_blankTimer;
extern int g_fps;
extern unsigned int g_textTimer;
extern string g_line1, g_line2;

class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool Init();
	virtual void Kill();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void OnEnterBackground();
	virtual void OnEnterForeground();
	virtual void Update();
	void OnExitApp(VariantList *pVarList);
	virtual bool OnPreInitVideo();
	void FakeUpdate();
	
	//we'll wire these to connect to some signals we care about
	void OnAccel(VariantList *pVList);
	void OnArcadeInput(VariantList *pVList);
	Surface m_backgroundSurf;
	RTFont g_font;
	RTFont g_big_font;


	bool m_bWantToResetArduboy;
private:

	bool m_bDidPostInit;
	Surface m_surf; //for testing
};


App * GetApp();
const char * GetAppName();
const char * GetBundlePrefix();
const char * GetBundleName();
