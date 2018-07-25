#include "PlatformPrecomp.h"
#include "ParticleTestComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"


ParticleTestComponent::ParticleTestComponent()
{
	SetName("ParticleTest");
}

ParticleTestComponent::~ParticleTestComponent()
{
}

void ParticleTestComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&ParticleTestComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&ParticleTestComponent::OnUpdate, this, _1));
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&ParticleTestComponent::OnInput, this, _1));

	m_texture.LoadFile("interface/particle/light16p.rttex");
	m_particle.Setup(&m_texture, 2000);
	m_particle.set_size(2.0);
	m_particle.coloring2(L_Color(133,255,20, 255), L_Color(255,10,10, 0));
	m_dropper.Setup(25, 240, 160);
	m_dropper.add(&m_particle);
	m_dropper.initialize();
	
	m_textureExplode.LoadFile("interface/particle/small.rttex");
	m_particleExplode.Setup(&m_textureExplode, 800);
	m_particleExplode.set_color(L_Color(110,50,255,255));
	m_particleExplode.coloring2(L_Color(110,50,255,255), L_Color(200,100,255,0), 0.6f);

	m_motionController.set_speed_limit(0.1f);
	m_motionController.set_1d_acceleration(-0.0003f);
	m_particleExplode.set_motion_controller(&m_motionController);

	m_explosion = L_ExplosionEffect(320,240,16,10,12,0.1f);
	m_explosion.add(&m_particleExplode);
	m_explosion.initialize();
	m_explosion.set_life_distortion(700);

	m_textureFire.LoadFile("interface/particle/explosion.rttex");

	m_motionFire.set_1d_acceleration(-0.0004f);
	m_particleFire.Setup(&m_textureFire, 500);
	m_particleFire.set_color( L_Color(110,60,255, 255) );
	m_particleFire.coloring2( L_Color(255,255,100, 255), L_Color(255,60,60, 0) );
	m_particleFire.sizing2( 1.0, 2.5 );
	m_particleFire.set_motion_controller(&m_motionFire);


	m_explodeFire = L_ExplosionEffect(0,0,16,4,5,0.3f);
	m_explodeFire.add(&m_particleFire);
	m_explodeFire.set_life(300); //set life of this effect
	//effect->set_rotation_distortion(L_2PI);
	m_explodeFire.set_size_distortion(0.8f);
	m_explodeFire.set_life_distortion(200); //set life distortion for particles
	m_explodeFire.set_speed_distortion(0.1f);
	m_explodeFire.initialize();

	m_emitter.Setup(&m_explodeFire);

}

void ParticleTestComponent::OnRemove()
{
	EntityComponent::OnRemove();
}

void ParticleTestComponent::OnRender(VariantList *pVList)
{
	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	m_dropper.draw((int)vFinalPos.x, (int)vFinalPos.y);
	m_explosion.draw((int)vFinalPos.x, (int)vFinalPos.y);
	m_emitter.draw((int)vFinalPos.x, (int)vFinalPos.y);
}

void ParticleTestComponent::OnUpdate(VariantList *pVList)
{
	static double rad = 0.0;
	static CL_Vec2f current_pos(240, 160);
	static CL_Vec2f prev_pos;
	
	rad += 0.0026*GetBaseApp()->GetGameDeltaTick();

	if( rad > L_2PI )
		rad -= L_2PI;

	prev_pos = current_pos;
	current_pos.x = 160*(float)cos(rad)+240;
	current_pos.y = 160*(float)sin(rad)+160;

	CL_Vec2f vel( (current_pos.x-prev_pos.x)/GetBaseApp()->GetGameDeltaTick(),
		(current_pos.y-prev_pos.y)/GetBaseApp()->GetGameDeltaTick() );

	/* it's recommended to use L_ParticleEffect::set_velocity() than just
	to use L_ParticleEffect::set_position() if the desired position of effect
	is not static or jumping. */

	m_dropper.set_velocity(vel);
	m_dropper.run(GetBaseApp()->GetGameDeltaTick());
	m_explosion.run(GetBaseApp()->GetGameDeltaTick());
	
	/* set position(although velocity has been set before) to avoid error
	being accumulated.*/
	m_dropper.set_position(current_pos.x, current_pos.y);
	m_emitter.run(GetBaseApp()->GetGameDeltaTick(), false);
}


void ParticleTestComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		m_emitter.emit(pt.x, pt.y);
		break;

	case MESSAGE_TYPE_GUI_CLICK_END:
		m_explodeFire.set_position(pt.x, pt.y);
		break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		m_explodeFire.set_position(pt.x, pt.y);
		break;
	}	

}