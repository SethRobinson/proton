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


#ifndef L_PARTICLEEFFECT_H
#define L_PARTICLEEFFECT_H


#include "L_Particle.h"
//#include <list>


//! Particle effect interface
class L_ParticleEffect
{

private:
	L_REAL cumulative_prob;
	int total_auto_p;


protected:
	L_Particle* fl_particle[L_PARTICLE_TYPE_LIMIT];
	L_REAL particle_prob[L_PARTICLE_TYPE_LIMIT];
	int num_particle_type;

	CL_Vec2f velocity;

	bool par_randrot_on;
	L_REAL size_distortion;
	int life_distortion;
	bool follow_shooting;

	int time_elapesed;

	bool istriggered;

	int period;
	int counter;

	int life; //L_INFINITE_LIFE for infinite life

	bool addit_vector_enabled;
	CL_Vec2f addit_vector;


	//choose one among particles according to its probability
	inline int choose_particle(void);

	//get distortion value of size among -size_distortion and size_distortion
	L_REAL rand_size(void);

	//create a particle
	void create_particle(L_REAL in_x, L_REAL in_y, CL_Vec2f* vec_t = NULL);

	//make the effect move
	void motion_process(L_REAL time_elapesed);

	//process for creating particle
	void creating_process(void);

	//function for inherited children to specify the particle creation method
	virtual void howto_emit_particle(void) = 0;


public:
	std::list<L_Particle*> particle_list;
	L_REAL x_pos;
	L_REAL y_pos;

	L_REAL x_pos_offset; //SETH
	L_REAL y_pos_offset; //SETH

	L_ParticleEffect(){};
	
	/**
	period : time(milisec) interval between two emissions, 1 <= period < infinity \n
	v_t : velocity for this effect */
	L_ParticleEffect(int period_t, int x, int y);

	/** Copy contructor.\n
	Note : this does not copy particle list. */
	L_ParticleEffect(const L_ParticleEffect& cpy);

	/** Destructor */
	virtual ~L_ParticleEffect();

	/** Add a particle type for the effect and probability for the particle to be chosen for every emission. \n
	Note : If no probability is specified, the particle will get the remaining probability to 1 ( if more than 1 probability non-specified particle,
	remaining probability to 1 will be equally divided for each. Maximum number of particle types is limited to 10. */
	int add(L_Particle* fl_p, L_REAL prob=-1);

	/** Copy effect's velocity from "v_t". */
	void set_velocity(const CL_Vec2f& v_t);

	/** Set effect's velocity(vector) using X length and Y length.\n
	Note : Use this function rather than set_position() if possible, internally there is calculation to ensure better\n
	pattern consistency even in jecky framerate environment. If you require to control exact position\n
	with no any accumulated error, call additional set_position() after run().*/
	void set_velocity(L_REAL x_length, L_REAL y_length);

	/** Set position. */
	void set_position(L_REAL x, L_REAL y);

	/** Set offset. */
	void set_offset(L_REAL x, L_REAL y); //SETH added this


	/** Apply randomization for particles' initial rotation. */
	void set_par_random_rotation(bool par_rand_rot=true);

	/** Apply distortion on particle's size. \n
	Each particle would have certain degree of different size if "size_dis" is greater than 0. */
	void set_size_distortion(L_REAL size_dis);

	/** Apply distortion on particle's life. \n
	Each particle would have certain degree of different length of life if "life_dis" is greater than 0. */
	void set_life_distortion(int life_dis);

	/** Set effect's life, L_INFINITE_LIFE for infinite life. */
	void set_life(int effect_life);

	/** Set the particle facing to the direction of emission. \n
	Note : Degree distortion is disabled, if you need the particle to follow \n
	its velocity direction even after the emission, please use L_Particle::rotating4();*/
	void set_follow_shooting(bool flag=true);

	/** Add additional vector for emitted particle, can be used for inertia effect */
	void set_addit_vector(const CL_Vec2f& v_t);

	/** Set period for emission in milli second. */
	void set_period(int millisecond);


	//======================== Get Attributes Funtions ===============================
	L_REAL get_x_pos(void);

	L_REAL get_y_pos(void);
	
	int get_particle_num(void);

	int get_life(void);

	int get_period(void);
	//================================================================================

	/** Trigger particle emission. */
	void trigger(bool bActive = true);

	/** Initialization, must be called (once) before calling run(int). */
	void initialize(void);

	void clear(); //SETH, remove all particles

	void run(int time_elapesed_t);

	void draw(int x_shift=0, int y_shift=0, float x_size_mod=1, float y_size_mod=1);

	/** Create a clone this particle effect. */
	virtual L_ParticleEffect* new_clone(void) = 0;

	/** Run all particles in the list( by calling run(int) function of particles ) with certain time. \n
	Basically this is used to resolve the inaccuracy of particle emission and movement when the frame rate is low,
	normal users should not use this function unless there is intention to write custom ParticleEffect. */
	virtual void activate_particle(int time_elapesed_t);
	void Setup(int period_t, int x, int y);
	void RenderPointSprites(SurfaceAnim *pSurf, int start, int count);
};


#endif
