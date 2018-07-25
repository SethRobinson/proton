//  ***************************************************************
//  TapSequenceDetectComponent - Creation date: 06/03/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TapSequenceDetectComponent_h__
#define TapSequenceDetectComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"

class TapSequenceDetectComponent: public EntityComponent
{
public:
	TapSequenceDetectComponent();
	virtual ~TapSequenceDetectComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void AddTapRegion(VariantList *pVList);
	void OnTouchStart(VariantList *pVList);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pAlignment;

	vector<CL_Rectf> m_tapRegions;
	int m_curTapTarget;

};

#endif // TapSequenceDetectComponent_h__