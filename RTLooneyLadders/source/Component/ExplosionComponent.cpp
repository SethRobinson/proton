#include "PlatformPrecomp.h"
#include "ExplosionComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"
#include "BuildingComponent.h"
#include "CharManagerComponent.h"

ExplosionComponent::ExplosionComponent()
{
	SetName("Explosion");
}

ExplosionComponent::~ExplosionComponent()
{

}


void ExplosionComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	
	m_pScale2d = &GetParent()->GetShared()->GetVarWithDefault("scale2d", Variant(1.0f, 1.0f))->GetVector2();
	
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	/*
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/

	//register ourselves to render if the parent does
	uint32 effect = GetVar("effect")->GetUINT32();
	int life  = 300;

	switch(effect)
	{
	case EFFECT_EXPLODE:
		m_textureFire.LoadFile("interface/particle/explosion.rttex");
		m_motionFire.set_1d_acceleration(-0.0009f);
		m_particleFire.Setup(&m_textureFire, 500);
		m_particleFire.set_color( L_Color(110,60,255, 255) );
		m_particleFire.coloring2( L_Color(255,255,100, 255), L_Color(255,60,60, 0) );
		m_particleFire.sizing2( 1.0, 2.5 );
		m_particleFire.set_motion_controller(&m_motionFire);

		m_explodeFire = L_ExplosionEffect(0,0,1000,300,300,0.17f);

		break;

	case EFFECT_STARS:
		m_textureFire.LoadFile("interface/particle/star.rttex");
		m_motionFire.set_1d_acceleration(-0.000f);
		m_particleFire.Setup(&m_textureFire, 500);
		m_particleFire.set_color( L_Color(110,60,255, 255) );
		m_particleFire.coloring2( L_Color(255,255,100, 255), L_Color(255,60,60, 0) );
		m_particleFire.sizing2( 1.0, 2.5 );
		m_particleFire.set_motion_controller(&m_motionFire);
	
		m_explodeFire = L_ExplosionEffect(0,0,30,1,2,0.2f);

		break;
	
	case EFFECT_TELEPORT:
		m_textureFire.LoadFile("interface/particle/explosion.rttex");
		m_motionFire.set_1d_acceleration(-0.0009f);
		m_particleFire.Setup(&m_textureFire, 500);
		m_particleFire.set_color( L_Color(110,60,255, 255) );
		m_particleFire.coloring2( L_Color(255,255,100, 255), L_Color(255,60,60, 0) );
		m_particleFire.rotating1(10,0);
		m_particleFire.sizing2( 1.0, 2.5 );
		m_particleFire.set_motion_controller(&m_motionFire);
		life = 800;
		m_explodeFire = L_ExplosionEffect(0,0,30,1,2,0.4f);

		break;

	default:

		LogMsg("Invalid particle type %d in ExplosionComponent", effect);
		return;
		break;
	}
	

    m_explodeFire.add(&m_particleFire);
	m_explodeFire.set_life(life); //set life of this effect
	
	//effect->set_rotation_distortion(L_2PI);
	m_explodeFire.set_size_distortion(0.8f);
	m_explodeFire.set_life_distortion(200); //set life distortion for particles
	m_explodeFire.set_speed_distortion(0.1f);
	m_explodeFire.initialize();

	m_explodeFire.trigger(true);

	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&ExplosionComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&ExplosionComponent::OnUpdate, this, _1));
}

void ExplosionComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ExplosionComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	vFinalPos = *m_pPos2d;

	CL_Vec2f vScale = *GetBuilding()->GetScale2D();
	vScale.x *= m_pScale2d->x;
	vScale.y *= m_pScale2d->y;

	//vFinalPos.y += 160;
	vFinalPos = WorldToScreenPos(vFinalPos);

	m_explodeFire.draw(vFinalPos.x, vFinalPos.y, vScale.x, vScale.y);
//	m_dropper.set_position(current_pos.x, current_pos.y);

}

void ExplosionComponent::OnUpdate(VariantList *pVList)
{
	m_explodeFire.run(GetBaseApp()->GetGameDeltaTick());
}