//  ***************************************************************
//  ParticleTestComponent - Creation date: 08/26/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ParticleTestComponent_h__
#define ParticleTestComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Renderer/LinearParticle.h"
#include "Renderer/SurfaceAnim.h"

class ParticleTestComponent: public EntityComponent
{
public:
	ParticleTestComponent();
	virtual ~ParticleTestComponent();

	virtual void OnAdd(Entity *pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList *pVList);
	void OnUpdate(VariantList *pVList);
	void OnInput( VariantList *pVList );

	CL_Vec2f *m_pPos2d;

	SurfaceAnim m_texture;
	L_Particle m_particle;
	L_DroppingEffect m_dropper;


	SurfaceAnim m_textureExplode;
	L_Particle m_particleExplode;
	L_MotionController m_motionController;
	L_ExplosionEffect m_explosion;

	SurfaceAnim m_textureFire;
	L_Particle m_particleFire;
	L_MotionController m_motionFire;
	L_ExplosionEffect m_explodeFire;
	L_EffectEmitter m_emitter;


};

#endif // ParticleTestComponent_h__