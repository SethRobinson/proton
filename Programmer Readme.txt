For the documentation, tutorials, wiki, and forums, please visit:

www.protonsdk.com

Note:  Some .dlls like audiere.dll and SDL2 stuff have identical names for the 
32 and 64 bit versions, this means that if you get an obscure error like "The application was
unable to start correctly" it probably means your .dll is the wrong bit version.  You'll need
to go find the correct version from <protondir>\shared\win\audiere or 
<protondir>shared\win\lib and copy those .dlls to your ../bin directory to fix it.

Note:  RTSimpleApp has the 64 bit audiere.dll included now, so use the 64 bit build there for simplicity.

Note: The Debug_GL_SDL configuration uses SDL for both the Audio and renderer/message pump/gamepads, while I normally
wouldn't use this in a Windows build, it's useful for testing things how they will work for the linux builds, which
all use SDL2 under the hood.

-Seth (seth@rtsoft.com)