
============================================================================
 LinearParticle 0.50
============================================================================
LinearParticle is an easy-to-use 2D particle library for Clanlib. It's
simple and small but powerful enough to be used to make eye candy 2D
visual effects for games. No graphical particle system editor is available
yet. LinearParticle 0.50 has been tested to work with Clanlib 0.8.0, if you
are using it with earlier version of Clanlib, you might encounter some
integration problems.



============================================================================
 Building/Installing LinearParticle
============================================================================
To build LinearParticle lib, you need to have Clanlib installed in your
system, no other dependencies. Before compiling the examples, GFrameHandler
lib (the source code has been included) need to be builded.

Preprocessor symbols for configuring LinearParticle :

-- L_USING_DOUBLE -- 
By default, LinearParticle uses float precision real numbers calculations.
If you want LinearParticle to perform calculations with double precision,
define "L_USING_DOUBLE" before building LinearParticle lib.

-- L_DEBUG_MODE --
Define this preprocessor symbol before building LinearParticle in debug mode.


Current version of LinearParticle comes with Code::Blocks workspace file,
Code::Blocks users can just open the project file to build LinearParticle
lib and examples (using float precision by default). Non-Code::Blocks users
unfortunately need to speed little more time to set up a workspace/project
file or writing makefile to build the library and examples.

If you are using MingW32, you can skip the step to build LinearParticle and
download the precompiled library from LinearParticle webpage.



============================================================================
 LICENSE
============================================================================
LinearParticle is distributed under zlib/libpng license. Check the
"LICENSE" file for details.



============================================================================
 News
============================================================================
LinearParticle 0.50 (6th March 2007)
- LinearParticle no longer requires RTTI to be enabled to fully
function (specially thanks to Andros Bregianos).
- L_Particle::Fixed rotating4(L_REAL).
- Added few member functions for few classes and API has been changed slightly.
- Added the faeture of user callback function.


LinearParticle 0.46 (1st November 2006)
- Little changes of implementation code, API remains the same.
- Corrected the size of sketch.png (from 32x9 to 32x8) that is used by
e06_cmotion example.


LinearParticle 0.45 (27th September 2006)
- Moderate changes of API.
- Added motion controller for handling complex motion of particle(s).
- Small performance improved (use 5-10% less CPU time).


LinearParticle 0.4 (9th July 2006)
- First release version.



============================================================================
email : chinfoo@galaxist.net
website : http://www.galaxist.net
- Wong Chin Foo


Last Update : 6th March 2007
============================================================================
