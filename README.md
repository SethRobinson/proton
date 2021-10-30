# Proton SDK

## For tutorials and more info, visit [The Proton SDK wiki](https://www.protonsdk.com)

License: [BSD style with attribution required](https://github.com/SethRobinson/proton/blob/master/license.txt)

Seth's GL/GLES messy multi-platform C++ game SDK.  Can output to **Windows**, **Linux** (including the **Raspberry Pi**), **HTML5**, **OS X**,  **iOS**, **Android**

A component based toolbox of useful things built up over the last ten years.  Instead of a giant .lib you link only the .cpp files used when possible to simplify multiplatform support as well as keep code size down.



It's kind of an SDL-like on steroids (while also being able to target SDL2 for setup/input/audio itself when needed) but generally gets the best results with its own native implementations of things. For example, it can target the following audio subsystems: SDL2_mixer, Audiere, FMOD, FMODStudio, Native iOS, Native Android, Denshion, Native Flash

It's designed with a "Write stuff in Windows with Visual Studio 2017, then compile/export to other platforms as needed" mentality.

Deprecated platforms no longer actively supported:  Flash, BBX, WebOS

Some things written with Proton:

* [Growtopia](https://www.growtopiagame.com) - 2D MMO, a good example of using Proton's GUI for many screen sizes.
* [Dungeon Scroll](https://www.rtsoft.com/pages/dscroll_mobile.php) - A word game.  ([HTML5 version](http://www.dungeonscroll.com))
* [Dink Smallwood](https://www.rtsoft.com/pages/dink.php) - Good example of porting old code to Proton to add touch controls and multiplatform support. Open source. [HTML5 version](https://www.rtsoft.com/web/dink)
* [Mind Wall](https://www.codedojo.com/?p=138) - 3D puzzle game
* [Tanked](https://www.rtsoft.com/pages/tanked.php) - 3D multiplayer tank combat game including four player split screen support as well as internet match making.
* Arduboy Simulator - Allows you to write and debug [Arduboy](arduboy.com) apps with MSVC as well as output HTML5 playable versions (included with [Proton SDK](https://www.arduboy.com)) [HTML5 Example game](http://www.rtsoft.com/arduman.html)

Credits and links
- [Proton SDK wiki/tutorial site](https://www.protonsdk.com)
- Seth A. Robinson (seth@rtsoft.com) (Wrote most of Proton SDK) ([Codedojo](https://www.codedojo.com), Seth's blog)
- Aki Koskinen (Contibutions to Linux support, SpriteAnim, documentation)
- [Clanlib team](https://github.com/sphair/ClanLib/blob/master/CREDITS) (Some math functions were taken from Clanlib)
- Dan Walma (contributions to SoftSurface)
- Fatalfeel's [Proton SDK forks](https://github.com/fatalfeel) for GLES 2 support and Cocos2D integration
- Vita platform support by @NabsiYa

# Want to contribute?

Feel free to submit a pull request! At this point the goal is that all changes be *non-breaking* to existing projects.
