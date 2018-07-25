This is a console application that converts textures to .rttex's, turns BMFont font files into .rtfont and other cool stuff.

Compiling it will take some skillz.

You'll need Clanlib 1.x (not 2.X!) and the PVRTexLib.

Windows
-------

Note: RTPack.exe is already included, you don't need to compile this!!

Linux
-----

Requirements for compilation:
1) Download PVRTexLib from http://www.imgtec.com/powervr/insider/powervr-pvrtexlib.asp
   and extract it under tools/RTPack (so that the 'PVRTexLib' becomes a subdirectory of 'RTPack' directory)
2) Install ClabLib 1.x development libraries to your system, e.g. 'yum install ClanLib1-devel'
3) Install zlib development libraries to your system, e.g. 'yum install zlib-devel'
4) You also need the usual C++ compilation tools, like libstdc++. The build scripts need make and cmake

Now run build.sh in tools/RTPack. If all goes well a binary called 'RTPack' appears to a subdirectory called 'build'.
You can then copy that to wherever it's needed.


-Seth