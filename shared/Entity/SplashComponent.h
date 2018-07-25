//  ***************************************************************
//  SplashComponent - Creation date: 1/5/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef SplashComponent_h__
#define SplashComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"

/*
Displays a serious of splash screens in order.  You can set the timeout, they are also skippable by clicking.

It deletes itself after the last screen is shown.

Example usage:

Entity *pSplash = GetEntityRoot()->AddEntity(new Entity());
SplashComponent *pSplashComp = (SplashComponent*) pSplash->AddComponent(new SplashComponent);
pSplashComp->AddSplash("splash_1.rttex", 2500, true, true, MAKE_RGBA(0,0,0,255));
pSplashComp->AddSplash("splash_2.rttex", 2500, true, true, MAKE_RGBA(0,0,0,255));
pSplashComp->AddSplash("splash_3.rttex", 2500);
AddFocusIfNeeded(pSplash);


//If you'd like to run something when the last splash is shown, connect to it deleting itself like this:
pSplash->GetFunction("OnDelete")->sig_function.connect(1, boost::bind(&MyApp::SplashScreensFinished, this, _1));


You can also connect to a signal to get notification and filename as each splash displays, to load stuff or play music:

Just add: 

pSplashComp->m_sig_on_splash_change.connect(OnSplashChanged);


And here is the function it is calling:

void OnSplashChanged(VariantList *pVList)
{
	LogMsg("Now showing %s in entity %s", pVList->Get(1).GetString().c_str(), pVList->Get(0).GetString().c_str());
}


*/

class SplashScreenItem
{
public:
	string m_fileName;
	int m_timeOutMS;
	bool m_bScaleToFitScreen;
	bool m_bPerserveAspectRatio;
	unsigned int m_bgColor;

};

class SplashComponent: public EntityComponent
{
public:
	SplashComponent();
	virtual ~SplashComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void AddSplash(string fileName, int timeOutMS=300, bool bScaleToFitScreen=true, bool bPerserveAspectRatio=false, unsigned int bgColor = MAKE_RGBA(0,0,0,255));

	boost::signal<void (VariantList*)> m_sig_on_splash_change; //called when a splash screen is started, sends the entity that the splash is on

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	Entity * ActivateSplash(SplashScreenItem &item);
	CL_Vec2f *m_pPos2d;
	
	/*
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	deque<SplashScreenItem> m_splashes;
};

#endif // SplashComponent_h__