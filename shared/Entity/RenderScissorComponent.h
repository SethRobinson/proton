//  ***************************************************************
//  RenderScissorComponent - Creation date: 11/30/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef RenderScissorComponent_h__
#define RenderScissorComponent_h__

#include "Component.h"

class Entity;

class RenderScissorComponent: public EntityComponent
{
public:
	RenderScissorComponent();
	virtual ~RenderScissorComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();
	void FilterOnRender(VariantList *pVList);
	void PostOnRender(VariantList *pVList);
	//our stuff


	enum eScissorPosition //this is todo, a plan for when I want custom positioning.
	{
		POSITION_FROM_SIZE,
		POSITION_CUSTOM
	};

private:

	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pPos2d;
	uint32 *m_pAlignment;
	uint32 *m_pScissorMode; //todo
	CL_Vec2f m_oldScissorPos; //to put it back
	CL_Vec2f m_oldScissorSize; //to put it back
	bool m_bOldScissorEnabled;
};

#endif // RenderScissorComponent_h__