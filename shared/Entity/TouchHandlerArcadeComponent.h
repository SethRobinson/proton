//  ***************************************************************
//  TouchHandlerArcadeComponent - Creation date: 10/24/2012
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2012 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TouchHandlerArcadeComponent_h__
#define TouchHandlerArcadeComponent_h__

#include "Component.h"
#include "Entity.h"

//Like a normal TouchHandlerComponent, but with better multi-touch support, and marks its buttons as "handled", better for
//say, arrow buttons for an arcade style button 

//Also, it detects pinches, calls OnPinchMod on parent with the parms being parent entity, pinch % change. (0.5 would mean pinched half the screens worth)
//Calls OnPinchStart and OnPinchEnd as well (on the parent entity)
//If dontClaimOwnerShip is set to 1, it will never mark touches as handled, good for a bg that has buttons in front of it

class TouchHandlerArcadeComponent: public EntityComponent
{
public:
	TouchHandlerArcadeComponent();
	virtual ~TouchHandlerArcadeComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	void OnInput(VariantList *pVList);

private:

	void HandleClickStart(CL_Vec2f &pt, uint32 fingerID);
	void HandleClickMove(CL_Vec2f &pt, uint32 fingerID);
	void HandleClickEnd(CL_Vec2f &pt, uint32 fingerID);
	
	Variant *m_pTouchOver;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pPos2d;
	CL_Rectf *m_pTouchPadding;
	uint32 *m_pAlignment;
	uint32 *m_pAllowSlideOns; //if false/0, will only respond to clicks that originate on us (good for backgrounds, bad for buttons)
	uint32 *m_pDontClaimOwnership; //if 1, we'll never mark touches as owned by us, and stop tracking touches that suddenly get marked.  Good for backgrounds

	void UpdateTouchArea(Variant *v);
	void HandleClickStartSecond(CL_Vec2f &pt, uint32 fingerID);
	void HandleClickMoveSecond( CL_Vec2f &pt, uint32 fingerID );
	void HandleClickEndSecond( CL_Vec2f &pt, uint32 fingerID );
	void ReleaseClick(CL_Vec2f vPt, uint32 fingerID);
	void ReleaseTouchIfNeeded();
	void EndPinchIfNeeded();
	CL_Rectf m_touchArea;
	
	int m_activeFinger;
	int m_secondFinger; //we monitor this for noticing pinch zooming
	
	CL_Vec2f m_fingerStartPos;
	CL_Vec2f m_secondFingerStartPos;
	bool m_bIsPinching;
	uint32 *m_pDisabled;


};

#endif // TouchHandlerArcadeComponent_h__
