//  ***************************************************************
//  TouchHandlerComponent - Creation date: 05/12/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TouchHandlerComponent_h__
#define TouchHandlerComponent_h__

#include "Component.h"
#include "Entity.h"

/**
 * A component that adds touch event handling behaviour to an \c Entity.
 *
 * The name of the component is initially set to "TouchHandler".
 *
 * This component is useful if you want to handle touch input events within
 * a given rectangular area. The area is determined by the position and size
 * of the parent \c Entity.
 *
 * The following named variants are used from the parent \c Entity:
 * - <b>"pos2d" (Vector2), "size2d" (Vector2):</b> used to determine the touch area.
 * - <b>"touchPadding" (Rect):</b> padding area for the touch area. This can be used
 *   to make the touch area larger than the area of the parent \c Entity. The default is
 *   (left: 20, top: 5, right: 20, bottom: 15).
 * - <b>"ignoreTouchesOutsideRect" (uint32):</b> defines if move events should be ignored
 *   when the touch position has moved outside the touch area. 0 (the default) means that
 *   move events outside the touch area are not ignored (i.e. they are handled). Any
 *   other value means that they are ignored. If this feature is enabled it basically
 *   means that the "OnOverEnd" function doesn't get called. As a side effect the "touchOver"
 *   variant also gets out of sync so the usability of this feature is questionable.
 * - <b>"touchOver" (uint32):</b> this value keeps track if the touch position is currently
 *   inside the area or not. You can read this value but don't write it!
 *
 * When events happen the following functions of the parent \c Entity get called:
 * - OnTouchStart - a pointer was pressed inside the area.
 * - OnOverStart - a pressed pointer has entered the area. This can occur when a pointer is
 *   pressed down inside the area (i.e. right after "OnTouchStart") or when an already
 *   pressed pointer is moved inside the area.
 * - OnOverMove - a pressed pointer moves within the area.
 * - OnOverEnd - a pressed pointer is moved outside the area or the pointer is released.
 * - OnTouchEnd - a pointer was released inside the area.
 *
 * When these functions get called they get a \c VariantList that contains four members:
 * - 0: the touch point of type \c CL_Vec2f
 * - 1: a pointer to the \c Entity that was touched
 * - 2: the id of the finger, type is uint32
 * - 3: a uint32 telling if the touch point (passed in member 0) is within the touch area.
 *      1 means it's inside the area, 0 means it's outside.
 */
class TouchHandlerComponent: public EntityComponent
{
public:
	TouchHandlerComponent();
	virtual ~TouchHandlerComponent();

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
	uint32 *m_pIgnoreTouchesOutsideRect; //if this is set to 1, we will ignore "Touch end" messages that don't happen in our rect+padding

	void UpdateTouchArea(Variant *v);
	CL_Rectf m_touchArea;

};

#endif // TouchHandlerComponent_h__
