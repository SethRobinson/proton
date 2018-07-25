//  ***************************************************************
//  TouchStripComponent - Creation date: 08/04/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//makes a strip or rectangle and returns an xy vector from 0 to 1 on each axis if you connect a signal to its parent's "OnTouchStripUpdate"
//NOTE: Requires a TouchHandlerComponent to work

#ifndef TouchStripComponent_h__
#define TouchStripComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

class TouchStripComponent: public EntityComponent
{
public:
	TouchStripComponent();
	virtual ~TouchStripComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

	enum eVisualStyle
	{
		STYLE_NONE,
	};

private:

	void OnOverStart(VariantList *pVList);
	void OnOverMove(VariantList *pVList);
	void SetPosition(CL_Vec2f vPos);
	void OnTouchStart(VariantList *pVList);
	void OnInput(VariantList *pVList);
	void OnPrintDiagnostics(VariantList *pVList);
	uint32 *m_pDisabled;
	uint32 * m_pVisualStyle;
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pMult; //multiplied against the final result, so you can make it go backwards or whatever
	uint32 *m_pSwapXAndY, *m_pReverseX, *m_pReverseY;
	CL_Vec2f m_lastPos;
	int m_activeFinger;
	FunctionObject *m_pOnTouchStripUpdate;
	CL_Rectf *m_pTouchPadding;
	uint32 *m_pAlignment;

};

#endif // TouchStripComponent_h__