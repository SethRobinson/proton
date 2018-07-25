//  ***************************************************************
//  LogDisplayComponent - Creation date: 2/23/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef LogDisplayComponent_h__
#define LogDisplayComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Manager/Console.h"

/*
This shows a log of stuff, scrolling so the newest stuff is on the bottom.
If you use GetFunction("AddLine") (send it a VariantList with a string) to add your own stuff, it will use its internal logging
system.  

You can also set its "text" variable to just replace everything, like a TextBoxRender component.

Or, you can cast to this class and use SetConsole() to set your own custom console to feed in data. (using the Console class)

Otherwise, it will display the proton system log if you do nothing.

Eitherway, if you use its "AddLine" function, it will handle wrapping for you.

An easy way to use this to show the system log is to use the helper function below.

Just do:

SetConsole(true, (optional, true to enable scrolling)); and it will create it as an overlay.

If the "enableScrolling" var is set to 1, vertical scroll bars will be enabled.
(Note, if you do this, ScrollComponent will try to load interface/scroll_bar_caps.rttex for the image to use, you
can steal it from RTSimpleApp if needed)
*/

class LogDisplayComponent: public EntityComponent
{
public:
	LogDisplayComponent();
	virtual ~LogDisplayComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	//if you REALLY need direct access to this...
	Console * GetConsole() {return m_pActiveConsole;}
	void SetConsole(Console *pConsole); //set a custom Console object to feed in the text/lines of the log data.  We won't try to delete it

private:

	void OnRender(VariantList *pVList);
	void AddLine(VariantList *pVList);
	void OnEnableScrollingChanged(Variant *pVariant);
	void UpdateScrollBar();
	void OnTouchDragUpdate(VariantList *pVList);
	void OnOverStart(VariantList *pVList);
	void OnOverEnd(VariantList *pVList);
	void ModCurLine(float mod);
	void OnTextAdded();
	void OnUpdate(VariantList *pVList);
	void ModByDistance(float mod);
	void OnTextChanged(Variant *pDataObject);
	void InitInternalConsoleIfNeeded();
	
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	uint32 *m_pFontID;
	
	CL_Vec2f *m_pScale2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	float *m_pFontScale; //so we can set it independently of our main size, although that will be applied to the font too
	uint32 *m_pEnableScrolling; 
	Console * m_pActiveConsole;
	Console * m_pInternalConsole;
	EntityComponent *m_pScrollBarComp; //optional, only used if enabledScrolling was set to 1
	bool m_bIsDraggingLook;
	float m_curLine; //a float, so we can move slower than one whole line at a time if we wish
	bool m_bUsingCustomConsole;
	string *m_pText; //only valid if it was set, so normally you can't read from it when using AddLog or the system log ways.

	//for scrolling momentum stuff, simular to how the ScrollComponent works but.. I recreated it here as I
	//didn't think it made sense to somehow hook a ScrollComponent into this. -Seth

	CL_Vec2f m_vecDisplacement;
	float * m_pFriction, *m_pMaxScrollSpeed, *m_pPowerMod;

	deque<string> m_queuedLines; //if a user is scrolling around, we don't want to add the new lines added right away as it's visually jarring

};

//misc helpers

void ToggleConsole();
void SetConsole(bool bOn, bool bEnableScrollbars = false);
Entity * CreateLogDisplayEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale=1.0f);

#endif // LogDisplayComponent_h__