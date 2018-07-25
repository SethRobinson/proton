//  ***************************************************************
//  TouchDragComponent - Creation date: 02/04/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

/*
Creates an invisible area that responds to dragging a touch and sends the exact pixel offet dragged.
Connect to its function "OnTouchDragUpdate".  It passes a VariantList with <Component*> itself, and a CL_Vec2d (offset dragged)

You can also connect to the "OnOverStart" and "OnOverEnd" of its parent entity if you need that info.

Why use this?  It's a way I enabled "drag around the screen" to look/rotate in 3d games.

- It ignores touches marked as "handled".  (so if a user clicks a button and drags off it, it will ignore that)
- It marks touches it responds to as handled, so buttons and other components will ignore that touch.
*/

#ifndef TouchDragComponent_h__
#define TouchDragComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

class TouchDragComponent: public EntityComponent
{
public:
	TouchDragComponent();
	virtual ~TouchDragComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eVisualStyle
	{
		STYLE_NONE,
	};

	void OnInput( VariantList *pVList );

	void ModLastPos( CL_Vec2d vPos );
	void SetLastPos(CL_Vec2d vLastPos); //for a hack I had to do on a project..
	void ResetLastPos();

private:

	void SetPosition(CL_Vec2f vInputPos);
	void EndDrag(int fingerID, CL_Vec2f pt);
	uint32 *m_pDisabled;
	uint32 * m_pVisualStyle;
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pMult; //multiplied against the final result, so you can make it go backwards or whatever
	uint32 *m_pSwapXAndY, *m_pReverseX, *m_pReverseY;
	CL_Vec2f m_lastPos;
	int m_activeFingerID;
	FunctionObject *m_pOnTouchDragUpdate;
	CL_Rectf *m_pTouchPadding;

};

#endif // TouchDragComponent_h__