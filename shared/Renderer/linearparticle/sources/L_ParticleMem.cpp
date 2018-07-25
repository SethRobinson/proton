#include "PlatformPrecomp.h"

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


#include "L_ParticleMem.h"


L_Particle** L_ParticleMem::mem;
int L_ParticleMem::head=-1;
int L_ParticleMem::mem_size=0;
int L_ParticleMem::pointSpriteArraySize=0;

vector<PointSprite> L_ParticleMem::pointSpriteArray;
GLuint L_ParticleMem::pointSpriteBufferID = 0;

void L_ParticleMem::init(int size)
{
	mem_size = size;
	mem = new L_Particle*[mem_size];

	int i;
	for( i=0; i<mem_size; i++ )
	{
		mem[i] = new L_Particle();
	}

	//buffer for fast point sprites
	//how many can we batch at once?
#ifndef C_GL_MODE
	pointSpriteArraySize = 500;
	pointSpriteArray.resize(pointSpriteArraySize);
	glGenBuffers(1, &pointSpriteBufferID);
	CHECK_GL_ERROR();
#endif
}


void L_ParticleMem::deinit(void)
{
	int i;
	for( i=0; i<mem_size; i++ )
	{
		delete mem[i];
	}

	delete[] mem;

	#ifndef C_GL_MODE
	glDeleteBuffers(1, &pointSpriteBufferID);
	pointSpriteArraySize = 0;
	pointSpriteArray.clear();
	pointSpriteBufferID = 0;
	#endif
}
