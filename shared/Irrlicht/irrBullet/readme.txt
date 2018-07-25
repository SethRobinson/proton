 _         _____     _ _     _   
|_|___ ___| __  |_ _| | |___| |_ 
| |  _|  _| __ -| | | | | -_|  _|
|_|_| |_| |_____|___|_|_|___|_|  
  
                                                                                 
NOTE: Extract bin.7z to see the compiled examples!

For class list and documentation, refer to doc/doxygen/html/index.html

For the FAQ list, refer to doc/FAQ/FAQ.pdf

1. Introduction:

Thank you for using irrBullet!

irrBullet is more than just a wrapper, it's an extension library for Bullet physics.
It includes many high-level features not found in other Bullet wrappers or even Bullet itself.

It's engineered in an efficient way that's easy to extend.

It includes interfaces to physics objects, and pointers inside those
interfaces so that you can use the Bullet library to its full extent
if you need access to a function that is not yet included in the interfaces.

In the "libs" folder you will find pre-built Win32 MinGW-GCC and Microsoft Visual C++ 2010 and 2008 irrBullet
and Bullet libraries ready for use.


If you spot any bugs or you would like to leave feadback,
let me know at fighterstw@hotmail.com


2. Starting a project:

To begin using basic irrBullet features in your project, 
you must follow these steps:


2.1. Add these search directories to your project:
	%irrBullet%/source
	%irrBullet%/source/bheaders
	%irrBullet%/source/bheaders/Bullet

2.2. add "#include <irrBullet.h>" to the top of your project
with the other includes. 

2.3. add these files to your linker (available in the libs/ folder):
	libirrBullet.a, libbulletdynamics.a, libbulletsoftbody.a
	libGIMPACTUtils.a (if you use GImpact),
	liblinearmath.a, libbulletcollision.a

	Make sure the linker files are in that order for irrBullet
	or your project will not compile.

3. Extra:

The header files to the Bullet Physics library are included under
source/bheaders/Bullet. A Code::Blocks project file to compile irrBullet is under
the source/ folder, an MSVC 2010 project is included in source/msvc/2010,
and an MSVC 2008 project is included in source/msvc/2008

The media in the bin/ folder is not all mine, some of it is from
the Irrlicht examples folder. The Irrlicht 1.7.1 dll is included
for the examples to run.


Copyright (C) 2009-2011 Josiah Hartzell (Skyreign Software)
