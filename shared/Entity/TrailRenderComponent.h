//  ***************************************************************
//  TrailRenderComponent - Creation date: 08/12/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TrailRenderComponent_h__
#define TrailRenderComponent_h__

#include "Component.h"
#include "Entity.h"
#include "Renderer/Surface.h"
#include "BaseApp.h"

//what we need to remember for each part of the trail

struct TrailFrame
{
	CL_Vec2f m_pos2d;
	CL_Vec2f m_size2d;
	uint32 m_color;
	uint32 m_colorMod;
	float m_alpha;
	CL_Vec2f m_scale2d;
	float m_rotation; //in degrees
};

class TrailRenderComponent: public EntityComponent
{
public:
	TrailRenderComponent();
	virtual ~TrailRenderComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void SetFrameFromEntity(TrailFrame *pFrame);
	void SetEntityFromFrame(TrailFrame *pFrame);
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	CL_Vec2f *m_pScale2d;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	uint32 *m_pFrames;
	deque<TrailFrame> m_history;
	bool m_insideTrailDrawingNow;
	float *m_pTrailAlpha;
	uint32 *m_pTimeBetweenFramesMS;
	uint32 m_frameRecordTimer;
	eTimingSystem m_timingSystem;
	
};

#endif // TrailRenderComponent_h__