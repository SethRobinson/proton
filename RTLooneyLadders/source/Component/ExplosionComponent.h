//  ***************************************************************
//  ExplosionComponent - Creation date: 12/19/2010
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2010 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ExplosionComponent_h__
#define ExplosionComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Renderer/LinearParticle.h"
#include "Renderer/SurfaceAnim.h"
//#include "Renderer/SurfaceAnim.h"

class ExplosionComponent: public EntityComponent
{
public:
	ExplosionComponent();
	virtual ~ExplosionComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);

	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Vec2f *m_pScale2d;
	
	/*
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/

	SurfaceAnim m_textureFire;
	L_Particle m_particleFire;
	L_MotionController m_motionFire;
	L_ExplosionEffect m_explodeFire;

};

#endif // ExplosionComponent_h__