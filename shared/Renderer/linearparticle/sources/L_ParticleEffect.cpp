#include "PlatformPrecomp.h"

#define L_DEBUG_MODE

//===============================================================================
//
// LinearParticle Copyright (c) 2006 Wong Chin Foo
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software in a
// product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
//===============================================================================


#include "L_ParticleEffect.h"
#include "L_ParticleMem.h"

#if !defined(C_GL_MODE) &&  !defined( ANDROID_NDK) && !defined( PLATFORM_BBX) && !defined( RT_WEBOS) && !defined( RT_GLES_ADAPTOR_MODE)
	#define RT_USE_POINT_SPRITES
#endif


void L_ParticleEffect::Setup(int period_t, int x, int y)
{
	x_pos = (float)x;
	y_pos = (float)y;
	x_pos_offset = 0; //SETH
	y_pos_offset = 0; //SETH

	par_randrot_on = false;
	size_distortion = 0;
	life_distortion = 0;
	follow_shooting = false;

	num_particle_type = 0;

	cumulative_prob = 0;
	total_auto_p = 0;

	istriggered = true;
	period = period_t;
	counter = period;
	life = L_INFINITE_LIFE;
	addit_vector_enabled = false;
	velocity = CL_Vec2f(0,0);

	for (int i=0; i < L_PARTICLE_TYPE_LIMIT; i++)
	{
		fl_particle[i] = NULL;
	}
}

L_ParticleEffect::L_ParticleEffect(int period_t, int x, int y)
{
	Setup(period_t, x, y);
}


L_ParticleEffect::L_ParticleEffect(const L_ParticleEffect& cpy)
{
	num_particle_type = cpy.num_particle_type;

	int i;
	for( i=0; i<num_particle_type; i++ )
	{
		fl_particle[i] = cpy.fl_particle[i];
		particle_prob[i] = cpy.particle_prob[i];
	}

	velocity = cpy.velocity;

	x_pos = cpy.x_pos;
	y_pos = cpy.y_pos;

	par_randrot_on = cpy.par_randrot_on;
	size_distortion = cpy.size_distortion;
	life_distortion = cpy.life_distortion;
	follow_shooting = cpy.follow_shooting;

	cumulative_prob = cpy.cumulative_prob;
	total_auto_p = cpy.total_auto_p;

	istriggered = cpy.istriggered;
	period = cpy.period;
	counter = cpy.counter;

	life = cpy.life;

	addit_vector_enabled = cpy.addit_vector_enabled;
	addit_vector = cpy.addit_vector;
}


L_ParticleEffect::~L_ParticleEffect()
{
/*
	std::list<L_Particle*>::iterator iter = particle_list.begin();
	while( iter != particle_list.end() )
	{
		iter = particle_list.erase(iter);
	}
*/

	clear();
}

void L_ParticleEffect::clear()
{
	particle_list.clear(); //SETH optimization?
}

int L_ParticleEffect::add(L_Particle* fl_p, L_REAL prob)
{
	//Particle adding fails
	if( num_particle_type >= L_PARTICLE_TYPE_LIMIT )
	{
		#ifdef L_DEBUG_MODE
			LogMsg("LinearParticle : Could not add more than % particles for an effect.", L_PARTICLE_TYPE_LIMIT);
		#endif

		return 0;
	}

	else
	{
		fl_particle[num_particle_type] = fl_p;

		if( prob >= 0 )
		{
			if(cumulative_prob + prob > 1)
			{
				particle_prob[num_particle_type] = 1 - cumulative_prob;
				cumulative_prob = 1;
			}

			else
			{
				particle_prob[num_particle_type] = prob;
				cumulative_prob += particle_prob[num_particle_type];
			}
		}

		else
		{
			particle_prob[num_particle_type] = -1;
			total_auto_p++;
		}

		num_particle_type++;

		return 1;
	}
}


void L_ParticleEffect::set_velocity(const CL_Vec2f& v_t)
{
	velocity = v_t;
}


void L_ParticleEffect::set_position(L_REAL x, L_REAL y)
{
	x_pos = x+x_pos_offset;
	y_pos = y+y_pos_offset;
}
void L_ParticleEffect::set_offset(L_REAL x, L_REAL y)
{
	
	//remove any applied offset
	x_pos -= x_pos_offset;
	y_pos -= y_pos_offset;

	x_pos_offset = x;
	y_pos_offset = y;

	//add the new offset
	x_pos += x_pos_offset;
	y_pos += y_pos_offset;

}



void L_ParticleEffect::set_velocity(L_REAL x_length, L_REAL y_length)
{
	velocity = CL_Vec2f( x_length, y_length );
}


void L_ParticleEffect::set_par_random_rotation(bool par_rand_rot)
{
	par_randrot_on = par_rand_rot;
}


void L_ParticleEffect::set_size_distortion(L_REAL size_dis)
{
	size_distortion = size_dis;
}


void L_ParticleEffect::set_life_distortion(int life_dis)
{
	life_distortion = life_dis;
	if( life_distortion < 0 )
		life_distortion = -life_distortion;
}


void L_ParticleEffect::set_follow_shooting(bool flag)
{
	follow_shooting = flag;
}


void L_ParticleEffect::set_life(int effect_life)
{
	life = effect_life;
}


L_REAL L_ParticleEffect::rand_size(void)
{
	L_REAL current_size_dis = L_RAND_REAL_1() * size_distortion;
	if( rand()%2 == 0 )
		current_size_dis = -current_size_dis;

	return current_size_dis;
}


void L_ParticleEffect::set_addit_vector(const CL_Vec2f& v_t)
{
	addit_vector_enabled = true;
	addit_vector = v_t;
}


void L_ParticleEffect::set_period(int millisecond)
{
	period = millisecond;
}


int L_ParticleEffect::choose_particle(void)
{
	L_REAL dice = L_RAND_REAL_1();
	L_REAL slider = 0;
	int i;
	for( i=0; i<num_particle_type; i++ )
	{
		slider += particle_prob[i];

		if( dice <= slider )
			break;
	}

	return i;
}


void L_ParticleEffect::create_particle(L_REAL in_x, L_REAL in_y, CL_Vec2f* vec_t)
{
	int chosen = choose_particle();

	L_Particle* par_new;
	
	if (!fl_particle[chosen])
	{
		LogMsg("L_ParticleEffect::create_particle> Can't add invalid particle");
		return;
	}
	L_NEW_PAR( par_new, *fl_particle[chosen] );

	CL_Vec2f vec_t2 = CL_Vec2f(0,0);
	if(addit_vector_enabled == true)
	{
		if(vec_t != NULL)
			vec_t2 = addit_vector + *vec_t;

		else
			vec_t2 = addit_vector;
	}

	else
	{
		if(vec_t != NULL)
			vec_t2 = *vec_t;
	}

	par_new->set_velocity(vec_t2);


	if(follow_shooting)
	{
		par_new->set_rotation(linear_get_radian(*vec_t));
	}

	else if(par_randrot_on)
	{
		par_new->set_rotation2((float)(L_RAND_REAL_2()*L_2PI));
	}

	if(size_distortion != 0)
	{
		par_new->set_size( par_new->get_ref_size()+rand_size() );
	}

	if(life_distortion != 0)
	{
		int distort = rand() % life_distortion + 1;

		if(rand() % 2 == 0)
			distort = -distort;

		par_new->set_life(par_new->get_remaininig_life()+distort);
	}

	par_new->x_pos = in_x;
	par_new->y_pos = in_y;
	
	par_new->initialize();
	
	//add to list
	particle_list.push_back(par_new);
}


void L_ParticleEffect::motion_process(L_REAL time_elapesed)
{
	x_pos += velocity.x*time_elapesed;
	y_pos += velocity.y*time_elapesed;
}


void L_ParticleEffect::creating_process(void)
{
	if( istriggered && life > 0 )
	{
		static int i;
		static int loop_counter;

		i = 0;
		while(i < time_elapesed)
		{
			loop_counter=0;

			//===================================
			int time_elapesed_minus_i = time_elapesed - i;
			int period_minus_counter = period-counter;

			if(time_elapesed_minus_i < period_minus_counter)
			{
				counter += time_elapesed_minus_i;
				i += time_elapesed_minus_i;
				loop_counter += time_elapesed_minus_i;
			}

			else
			{
				counter += period_minus_counter;
				i += period_minus_counter;
				loop_counter += period_minus_counter;
			}
			//===================================

			motion_process((float)loop_counter);
			activate_particle(loop_counter);

			if(counter >= period)
			{
				howto_emit_particle();
				counter -= period;
			}
		}

		//istriggered = false; //SETH I changed his to be a vairable you can turn on and off instead of called every frame
		//addit_vector_enabled = false;
	}

	else
	{
		motion_process((float)time_elapesed);
		activate_particle(time_elapesed);
	}
}


L_REAL L_ParticleEffect::get_x_pos(void)
{
	return x_pos;
}


L_REAL L_ParticleEffect::get_y_pos(void)
{
	return y_pos;
}


int L_ParticleEffect::get_particle_num(void)
{
	return particle_list.size();
}


int L_ParticleEffect::get_life(void)
{
	return life;
}

int L_ParticleEffect::get_period(void)
{
	return period;
}

void L_ParticleEffect::initialize(void)
{
	// initialize prob for each added particle
	int i;
	for( i=0; i<num_particle_type; i++ )
	{
		if(particle_prob[i] == -1)
			particle_prob[i] = (1 - cumulative_prob) / total_auto_p;
	}
}
#ifdef RT_USE_POINT_SPRITES
void L_ParticleEffect::RenderPointSprites(SurfaceAnim *pSurf, int start, int count)
{
	pSurf->Bind();
	glBlendFunc( GL_SRC_ALPHA, GL_ONE);

	//this point sprite code was based on code from http://www.71squared.com/2009/05/iphone-game-programming-tutorial-8-particle-emitter/ which
	//was based on some cocos2d code I think -Seth

	glBindBuffer(GL_ARRAY_BUFFER, L_ParticleMem::pointSpriteBufferID);
	CHECK_GL_ERROR();
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite)*count, &L_ParticleMem::pointSpriteArray[0], GL_DYNAMIC_DRAW);
	CHECK_GL_ERROR();
	glEnable(GL_BLEND);

	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE_OES);
	CHECK_GL_ERROR();
	glTexEnvi( GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE );
	CHECK_GL_ERROR();
	// Enable vertex arrays and bind to the vertices VBO which has been created
	glEnableClientState(GL_VERTEX_ARRAY);
	CHECK_GL_ERROR();
	glBindBuffer(GL_ARRAY_BUFFER, L_ParticleMem::pointSpriteBufferID);
	CHECK_GL_ERROR();
	// Configure the vertex pointer which will use the vertices VBO
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	CHECK_GL_ERROR();
	// Enable the point size array
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);

	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));

	// Enable the use of the color array
	glEnableClientState(GL_COLOR_ARRAY);

	// Configure the color pointer specifying how many values there are for each color and their type
	glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*3));

	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	//NOTE: It crashes here on the WebOS GLES windows emulator .. but runs on the device.  driver bug I guess -Seth
	//Another note:  It also can crash a Touchpad so.. not going to use this optimized point sprite stuff for webos :(
	glDrawArrays(GL_POINTS, start,  count);
	CHECK_GL_ERROR();
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERROR();
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisableClientState(GL_COLOR_ARRAY);

	CHECK_GL_ERROR();
	glDisable(GL_POINT_SPRITE_OES);
	CHECK_GL_ERROR();
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisableClientState(GL_COLOR_ARRAY);
	CHECK_GL_ERROR();
	glDisable(GL_POINT_SPRITE_OES);
	glDisable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	CHECK_GL_ERROR();
}
#endif

void L_ParticleEffect::draw(int x_shift, int y_shift, float x_size_mod, float y_size_mod)
{

if (particle_list.size() == 0) return;

	//LogMsg("particles: %d", particle_list.size());
	std::list<L_Particle*>::iterator iter = particle_list.begin();

#ifndef RT_USE_POINT_SPRITES

	//OPTIMIZE: naive way to draw them with separate blits for normal GL mode or android code
	g_globalBatcher.Flush();
	while( iter != particle_list.end() )
	{
		(*iter)->draw(x_shift, y_shift, x_size_mod, y_size_mod);
		iter++;
	}
	
	g_globalBatcher.Flush(RenderBatcher::FLUSH_SETUP);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE); //custom render mode change

	g_globalBatcher.Flush(RenderBatcher::FLUSH_RENDER);

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  //undo custom render mode change
	g_globalBatcher.Flush(RenderBatcher::FLUSH_UNSETUP);

#else

	SurfaceAnim *pSurf = NULL;

	int count = 0;
	glPushMatrix();
	glTranslatef((GLfloat)x_shift, (GLfloat)y_shift, 0);

	while( iter != particle_list.end() )
	{
		
		if (count >= L_ParticleMem::pointSpriteArraySize)
		{
			//reached the max of the buffer size.. dump now
			RenderPointSprites(pSurf, 0, count);
			count = 0;
		}
		
		if (pSurf != (*iter)->surface)
		{
			if (pSurf != NULL)
			{
				//uh oh... we can't handle 2 textures with our batching process.  We need to sort the textures.. well, 
				//I don't feel like making that happen so let's just render what we've got so far before changing it
				RenderPointSprites(pSurf, 0, count);
					count = 0;
			}

			pSurf = (*iter)->surface;
		}
		L_ParticleMem::pointSpriteArray[count].vPos.x = (*iter)->x_pos;
		L_ParticleMem::pointSpriteArray[count].vPos.y = (*iter)->y_pos;
		L_ParticleMem::pointSpriteArray[count].size = pSurf->GetRawTextureWidth()*(*iter)->get_size();
		L_ParticleMem::pointSpriteArray[count].color = *(uint32*)&(*iter)->current_color;
		count++;
		iter++;
	}
	
	RenderPointSprites(pSurf, 0, count);
	glPopMatrix();

#endif
	
}


void L_ParticleEffect::activate_particle(int time_elapesed_t)
{
	std::list<L_Particle*>::iterator iter = particle_list.begin();
	while( iter != particle_list.end() )
	{
		if( (*iter)->is_alive() )
		{
			(*iter)->run(time_elapesed_t);
			iter++;
		}
		else
		{
			iter = particle_list.erase(iter);
		}
	}

	//if life is not infinite
	if(life < L_INFINITE_LIFE)
		life -= time_elapesed_t;
}

void L_ParticleEffect::trigger(bool bActive)
{
	istriggered = bActive;
}


void L_ParticleEffect::run(int time_elapesed_t)
{
	time_elapesed = time_elapesed_t;
	creating_process();
}
