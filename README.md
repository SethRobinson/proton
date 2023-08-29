# Proton SDK

## For tutorials and more info, visit [The Proton SDK wiki](https://www.protonsdk.com)

License: [BSD style with attribution required](https://github.com/SethRobinson/proton/blob/master/license.txt)

Seth's GL/GLES messy multi-platform C++ game SDK.  Can output to **Windows**, **Linux** (including the **Raspberry Pi**), **HTML5**, **OS X**,  **iOS**, **Android**

A component based toolbox of useful things built up over the last ten years.  Instead of a giant .lib you link only the .cpp files used when possible to simplify multiplatform support as well as keep code size down.

It's kind of an SDL-like on steroids (while also being able to target SDL2 for setup/input/audio itself when needed) but generally gets the best results with its own native implementations of things. For example, it can target the following audio subsystems: SDL2_mixer, Audiere, FMOD, FMODStudio, Native iOS, Native Android, Denshion, Native Flash

It's designed with a "Write stuff in Windows with Visual Studio 2017, then compile/export to other platforms as needed" mentality.

Deprecated platforms no longer actively supported:  Flash, BBX, WebOS

### 8/29/2023 Note

I had to make a breaking change - I updated the Boost library to the latest for proper C++20 support and it doesn't support signal anymore, just signals2.


If you're updating an old project, When you get this error:

1>c1xx : fatal  error C1083: Cannot open source file: '..\..\shared\util\boost\libs\signals\src\connection.cpp': No such file or directory
1>named_slot_map.cpp
1>c1xx : fatal  error C1083: Cannot open source file: '..\..\shared\util\boost\libs\signals\src\named_slot_map.cpp': No such file or directory
1>signal_base.cpp
1>c1xx : fatal  error C1083: Cannot open source file: '..\..\shared\util\boost\libs\signals\src\signal_base.cpp': No such file or directory
1>slot.cpp
1>c1xx : fatal  error C1083: Cannot open source file: '..\..\shared\util\boost\libs\signals\src\slot.cpp': No such file or directory
1>trackable.cpp
1>c1xx : fatal  error C1083: Cannot open source file: '..\..\shared\util\boost\libs\signals\src\trackable.cpp': No such file or directory

Remove references to those files, they don't exist anymore, signals2 is header-only, no source needed.

If you get errors like "1>D:\projects\proton\UGT\Source\App.h(132,9): error C2039: 'signal': is not a member of 'boost'" in your code, you'll need to change it.

From this:

	boost::signal<void(void)> m_sig_target_language_changed;

To this:

	boost::signals2::signal<void(void)> m_sig_target_language_changed;


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
