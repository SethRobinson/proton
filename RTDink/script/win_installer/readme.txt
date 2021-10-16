Dink Smallwood HD for Windows 7, 8, 10.

Requires OpenGL, if you have any problems running this, try installing the latest video drivers for your video card/chip.

To change screen size, drag the window borders around.  (Hold shift while dragging to toggle aspect-ratio lock)
To toggle a psuedo full screen mode, click Full Screen Toggle in the options.  (Or hit Alt-Enter)

NOTE: Quick saves might give a "Can't load old version" error if the data format has changed.  However, normal Dink saves (using save machines, etc) will always work.

Controls:

F4 - Quick save
F10 - Quick load
Control - Attack
Arrow keys - Movement
Space - Talk
Enter - Inventory
Shift - Magic
Escape - Bring up classic dink escape menu
F1 - Bring up Dink HD escape menu
Drag on window borders - scale the play area (when windowed)
(Hold) Tab - Turbo mode, entire game runs 3x speed
(Hold) Shift + Tab - Super Turbo mode, entire game runs up to 6x speed

NOTE: If you have a controller (like an xbox 360 pad) plugged in when you start the game, you can use that instead of keyboard, but you still need to use the
mouse to navigate the initial menus to start the game

Supported command line options:

-game <dmod directory> (Example:  dink.exe -game c:\dmods\island ) (this also sets -dmodpath automatically to the dmods parent directory)
-dmodpath or --refdir <dir containing DMOD dirs> (Example:  dink.exe -game c:\dmods )

Note:  If a .dmod file is put in the Dink HD directory (where the .exe is) it will be automatically installed and then deleted

To report a bug, find or post a thread about Dink HD on dinknetwork.com and and please include the following information if you can:

- Dink files are installed to C:\Users\<user name>\AppData\Local\DinkSmallwoodHD by default.

- Description of the bug and how to recreate it
- If the bug takes work to get to, it would be great if you could "Quick save" in the game and add a URL to download the quicksave.dat file.

quicksave.dat should be located in C:\Users\<user name>\AppData\Local\DinkSmallwoodHD for the main game and C:\Users\<user name>\AppData\Local\DinkSmallwoodHD\dmods\<dmod dir> for DMODs.

- The DMOD name (and URL to download it if possible)
- Windows version you're playing on
- Screen mode/resolution
- If the game crashes, please include the log.txt file as it may contain information about the crash.

-Seth (seth@rtsoft.com)
www.rtsoft.com

------ Change log for 1.7.0 ----------

* Upgraded source projects to Visual Studio 2017 (free community edition works fine)
* (bugfix) No longer forces reload of all graphics when saving the state
* (bugfix) No longer forces reload of all graphics and audio when losing focus/going into the background
* (bugfix) Fixed issue where screen scrolling could bug out if window was resized on windows
* Latest state data to "Continue" persists even if the game crashes now
* Added support for 64 bit builds (still fully compatible with existing save states/games)
* Added support for aspect ratio correction (can be disabled by enabling "Fullscreen stretch" in the options)
* Added IPV6 support
* (DinkC) Goto statements no longer require a ; at the end to work
* (iOS) Now compatible with iOS 11
* (Windows) Installer no longer requires admin privileges, now installs to local user's appdata dir (C:\UserName\AppData\Local\DinkSmallwoodHD)
* (Windows) .exe and installer are now signed with RTsoft's distribution certificate
* (Windows) Now checks for new versions on startup
* (Windows) Now writes the stack trace to log.txt if the game crashes (helps debug problems)

------ Change log for 1.7.1 ----------

(big thanks to Redink1 for most of these bug reports and the fancy shadows patch!)

* (Windows) Fixed app icon
* (Windows) Mouse-controlled screens in DMODs control better and hide the original mouse cursor
* (Windows) Now remembers screen size and fullscreen mode (don't forget, you can drag the window corners around to customize the screen size as well)
* (Windows) Input URL input area gets focus by default when installing a DMOD by URL
* (bugfix) Can no longer tap F8 during a game load to load a save state too early which can freeze the game
* (bugfix) Fixed issue where 32 bit tilebitmaps would go wonky when reloading the surface
* Default color under status bar is now black, fixes issue when transparent colors are used in the stats area, random garbage would show through
* (windows) Version # is now shown in title bar
* (bugfix) Fixed some issues with how default offsets are calculated, it fixed some problems where sprites would be in the wrong place in certain DMODs
* (bugfix, windows) "Smoothing" no longer incorrectly turns on if you lose/regain focus
* (windows) Added "Windowed Borderless fullscreen mode" toggle, defaults to off.  It will try to do 640X480 at your native monitor resolution by default now on
	a clean install. If you've dragged the dink window to a weird size it won't be able to go fullscreen which is kind of weird, but it does give a clear error message. It should probably
	enumerate graphic modes and choose one if the current one is invalid, or let the user choose, meh
* Added redink1's "soft shadow improvement" patch
* (proton) Fixed issue with blitting alpha things to the background, fixed the soft shadows for things rendered into the background

------ Change log for 1.7.2 ----------

* Changed "FPS lock:" to "Lock to 30 FPS:" in options, makes it more clear it's actually bad to use and not vsync
* (Windows) Handles alt-tab and clicking on a different monitor while in native fullscreen modes better
* Added "Ghost walk toggle" to in-game cheat menu.  Allows you to walk through solid objects and screenlocks
* (DinkC) Added support for Dan's load_tile() command
* (Windows) Added support for -game <dmod directory> parm to load a DMOD from anywhere on your HD. It also sets 
	the active DMOD dir to the DMODs parent directory for that session.  Automatic state autosave, save/continue and quicksave/quickload work as expected by saving data to its directory
* (bugfix) Save states now properly load even if the DMOD directory has been cut and pasted to a new place
* (bugfix) Full state saves now properly setup backgrounds with correct vision modifications instead of assuming it was 0
- Due to minor changes in the save state stuff, I've versioned it so the game will refuse to load old versions (as always, this doesn't affect the normal save files, just the full state saves HD does)
* (DinkC) fill_screen works better and colors 0 and 255 are no longer sometimes reversed
* (bugfix) M can now be used to turn off the map, previously it only worked to turn it on
* (bugfix) Status no longer incorrectly draws over full screen bitmaps (like the map in Mystery Island) and is properly reconstructed in full save states

------ Change log for 1.7.3 ----------

* (DinkC) DinkC version is now reported as 110 instead of 109
* (Bugfix) Fixed issue with set_frame_frame, the DMOD Mayhem now works
* (Bugfix) Fixed another incorrect color issue with fill_screen (This fixed Initiation's title screen)
* (Bugfix) Fixed horrible issue where the last line in a script might not get run.  This fixed the intro screen to Lost Forest Romp
* (Bugfix) Rewrote some nasty code that I could no longer understand, THE LAST QUEST (part 2)'s cool room slide/warp effect now works right
* Added "Debug DinkC Toggle" to cheat menu, it shows collision boxes and causes the log (log.txt or choose view log from debug menu)
* If fullscreen is at a weird size it will now just force it to 1024X768.  If that fails, well, it won't be good
- Note:  Save state format has been changed again, so old save states won't load

------ Change log for 1.7.4 ----------

* (Bugfix) Fixed a script crash.  This bug was also in the legacy dink
* (Windows) "Tap to continue" is no longer shown when waiting for a keypress, but will still be shown on touch-devices
* Fixed bug where any DMOD directory starting with "dink" was ignored from the internal DMOD listing screen.  Oops, that was a pretty bad one, sorry dinkcrft.
* (Bugfix) Fixed issue where default .ini animations wouldn't get applied correctly in some cases, this fixed a slight offset problem with Stone Of Balance
* Fixed problem with glitchy black lines during screen transitions
* (Bugfix) Fixed issue with playmidi and filenames that start with numbers over 1000, this fixed a problem with 1812over.mid not playing in Mayhem
* Added 1920X1080 screen mode selector in the Options menu
* No longer applying Dan's checkboard fix to tile screens (they don't need it I think..), and also added a hack so it will ignore main-02 through main-04.bpm (the dialog box) as it made it look weird
* Fixed some issues related to dynamic 8 bit to 32 bit surface changes (this can happen if you turn on the new shadows after starting to play or the game suddenly hits a 32 bit image)
- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.7.5 ----------

* Properly handles an illegal frame # sent to sp_frame instead of crashing
* (Bugfix) Fixed issue with not being able to set long delays in sprites (fixed dancing knight in Bugmania)
* (Bugfix) Fixed DinkC "!=" command which was renamed "!" for some reason. This fixed the goto in Bugmania
* Enabled support for >= and <= in DinkC, not sure why it was commented out before, they were active in 1.08
* (Bugfix) Status bar will now be drawn even when mouse mode is active if needed (it now shows up in Bugmania)
* Space no longer selects dialog, you'll have to use ENTER or CONTROL like on 1.08
* Mouse can now be used to select dialog options
* Fixed mouse issue with Bugmania in the town where it wasn't sending button down messages
* Made escape open the Dink HD menu even when wait_for_button is being used.  This may be a problem if any DMODs require ESCAPE to be a button that is used though
- Note:  Save state format has been changed again, so old save states won't load

------ Change log for 1.7.6 ----------

* Made Dink HD work with DFArc. Either you can put DFArc in the Dink HD dir and add "dmods" as the additional dmod directory so it can find
mods installed by Dink HD too, or you can keep Dink HD completely separate, and enter the full path/exe to Dink HD's .exe in DFArc config and that works too.

It's kind of weird that Dink HD puts its mods in a subdir called "dmods" as compared to the original Dink which useds its root dir.  I could change that but.. meh.

* Added support for -window and -debug from the command line.  Dink HD remembers the last setting already, so -window would only have any effect
if it was last used as fullscreen

* Added Dan Walma's improved shadow patch

------ Change log for 1.7.7 ----------

* Sprite blits that hang out of the game area are now rejected in some situations, to match functionality of Dink 1.08. (Bloop the fish on 1.08 looked right because it was getting ddraw errors when trying to draw the "blank" numbers because they were too big for the screen)
* A LOAD_SEQUENCE done after a LOAD_SEQUENCE_NOW in dink.ini is now ignored to match V1.08 functionality.  This fixes the issue of a fish turning into an old man in Bloop the fish
* Alt-Q now closes Dink, to match 1.08
* Fixed issue with bubble magic not having a transparent background in Bloop the fish (was due to some changes to make fake magic/weapon icons work on mobiles.. I'll have to redo that later)
* Fixed issue with slow nagivation in Broken Windows due to too much logging about scripts being run
* (Bugfix) copy_bmp_to_screen issue where it might try to blit a 24 bit image onto 8 bit fixed (fixed incorrect images in Broken Windows)
* (Bugfix) key-<key num>.c scripts are no longer sometimes loaded multiple times
* Some tweaks to keyboard input so Broken Windows typing area works better, "," and "." are supported.  Escape is as well but it also brings up Dink HD's menu which isn't great, but at least it works
* (Bugfix) Default transparency for LEFTALIGN things is now correctly white instead of sometimes not transparent
* The / divider on the experience display ( ns-11.bmp ) is now always "white transparent". This WAS the original way (check the Dink jewel case screenshots) but somehow in 1.08 and beyond this stopped being transparent.  Fixing this fixed the white box in Bug Mania
* (Bugfix) Fixed "way too jumpy megapotion" bug

- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.7.8 ----------

* Fixed issue where launching dmods from DFArc with a dmod path not in DFArc's dir or Dink HD's dmod dir would fail
* Fixed compatibility so Freedink.exe and the dink.exe from 1.08 can be run from Dink HD's dir (previously, start.c had been hacked to skip stuff for the HD version,
  it now checks version and if it's being run as a dmod or not and does it right)
* If you run the HD "dink" dir as a mod it now works as expected (it plays the normal game, including the old start up screen)
* Now including story/*.c instead of the .d files.  No reason to hide them, right? (note: if you install over HD without uninstalling, the old .d files will still be there.  But .c files are checked
first so it should be ok anyway)
* DMODs now default to the original .midi files rather than the .ogg CD music, as it changed the desired "feel" of some DMODs to switch to CD music.  Running with -game dink causes Dink itself to
  not use CD music as well, so for people who don't like CD music at all there's an option to play the original game with midis only
* (Bugfix) Dink HD will no longer try to grab missing sprites from the base dink dir, this fixes Dink's pushing anim in Zoltron
* (Bugfix) dmod installer no longer chokes on zero byte files (I'm looking at you, 9gems)
* (Bugfix) Fixed memory leak and possible crash related to script loading
* (Bugfix) Fixed issue with the checkerboard shadow processing where the 8 bit RLE decoder failed because it was expecting an 8 bit target (most bmps don't use RLE, but Alliance does in places)
* (Bugfix) Fixed issue with loading certain 16 bit bmps incorrectly, fixed tilted text in Infinidink
* Went ahead and decided to accept 252,252,252 as alpha if it's index 255 on an 8bit sprite.  This fixed the white background issue with Alliance dmod as well as Dinkcraft
* FEATURE: Can now download all DMODs from Dink Network directly from inside the game via Dan's php interface.  Can sort by rating, latest update date, or alphabetically

- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.7.9 ----------

* Dink now continues to download and install dmods when in the background, but everywhere else pauses automatically
* After installing a dmod, you can now choose to play it now or go back to the browse dmod list
* DMOD data downloaded from dink network is now cached for that session (it's not going to change so quickly, so why stress DN.com if we don't have to)
* (Bugfix) "installing <dmod>..." text message is no longer truncated in a weird way sometimes
* (Bugfix) Fixed another issue where a base graphic could fill in with a missing .bmp in a sequence when it shouldn't
* Huh, turns out .D files are always loaded before .C files.  I switched back to including .D files, otherwise if you didn't do a clean install my start.c changes don't show up
* Fixed it so freedink.exe and the old dink can be run directly from the Dink HD directory without crashing.  I didn't actually test it before, they didn't like / instead of \ in my .ini previously.
I also added the CD and splash.bmp that were missing
* Quick tip that pops up when playing the normal game now talks about the F1/F8 instant state save/load feature instead of talking about pressing the "pause icon" which doesn't even exist on the Windows build

- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.8.0 ----------

* Applied 16 bit bmp loading fix to "fancy shadows" version too, forgot before
* (Bugfix) Dink is no longer sometimes incorrectly shown on screen right as a dmod is started
* (Windows, DinkC, Performance) Logging code rewritten, it's was horribly slow before.  If a DMOD spammed "debug" messages it would previously drastically slow down the entire dmod
* (Bugfix) LOAD_SEQUENCE and LOAD_SEQUENCE_NOW fixes, fixes stuff in many dmods
* Improved mouse handling on dialog menus, no longer accidently select the first option of the dink menu if you click slow after using HD's escape menu to get there

- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.8.1 ----------

* Escape now brings up the classic dink escape menu, the Dink HD menu was just too intrusive.  It will show a text message that you can hit Shift-Escape to bring up the HD menu as well
* Made dialog box background look better by making it 32 bit with alpha, no more "cracks" at some resolutions.  Custom dialogs may still get line cracks at non 640X480 but whatever
* Default resolution is now 1024X768 fullscreen instead of 640X480
* dmod listing is packed tighter and now sorted alphabetically by dmod name, not directory name

------ Change log for 1.8.2 ----------

* Changed release compiling a bit to enable C7 compatible debug info, may help with stack traces
* Oops, turned max sprites back up to 300 from 100, don't ask me how I managed that one
* (DinkC) A line of over 200 characters won't corrupt memory (new limit is 512) - this fixed a crash in Grasp of Darkness
* (DinkC) external commands no longer generate bogus errors in the log.txt due to how 1.08 added weird overloading stuff
* Now shows the dmod's splash.bmp during loading
* Fixed issue where music might not play on the title screen
* Raised "memory of dead background sprites" to 300 from 100 for Windows, up to 200 for mobile
* Doubled memory Dink is allowed to use before uncaching graphics not used recently (could probably be disabled entirely for Windows but whatever)

------ Change log for 1.8.3 ----------

* Added "If a .dmod file is put in the Dink HD directory (where the .exe is), it will be automatically installed and then deleted" to readme.txt
* Dialog box rendering was off a few pixels due to me forgetting to remove some tests I did, fixed
* Black at index 255 and white at index 0 is now forced during bmp loading.  Windows does it, photoshop doesn't, but older versions of Dink (directx) seemed to do
it so going with that
* Invalid sprite sent to freeze/unfreeze will no longer crash the game (bug was also in original 1.08)
* Added hardening to check valid input on many script functions, will stop "sometimes" crashes based on bad scripting as well as log them
* Crash logging should show correct function names. I mean it this time!

------ Change log for 1.8.4 ----------

* Some changes to Dink's speed calculations, it's now basically locked at "you better get 60 fps or the whole game will slow down" - the
original system is REALLY bad and the timing of Dink vs monsters can vary wildly based on your framerate.  So I sort of had to "choose" a correct
speed and go with that, I chose a pretty fast speed sort of arbitrarily because it "felt" ok.  There isn't really a "correct" speed so I do expect problems with some dmods due to them expecting
a specific speed, but by choosing rather snappy speed hopefully mystery island and most of them will be finishable
* Related to the above, holding tab no longer lets you cheat by moving relatively faster than monsters
* Playing location aware sounds on a script not attached to a sprite will no longer cause crashes (should fix crash in malachi the jerk)

------ Change log for 1.8.5 ----------

* Tweaked speed a bit, sort of aligned it so the cutscenes in Initiation play right

------ Change log for 1.8.6 ----------

* (Windows) Running "dink.exe c:\temp\ACoolDMOD.dmod" from the command line will now install it, then run it.  (original dmod file won't be deleted)  Just seemed weird that this didn't exist so added it
* DMOD installer progress now switches to showing MB instead of K if the size is big
* BUGFIX: Fixed map loading bug that could crash the game (I added this one recently trying to clean up code to use more consts rather than magic #s, but there is always the risk I stupidely break something!)
* load_tile no longer instantly takes effect but requires a draw_screen or moving screens, this matches how 1.08 worked.  Loading a save state or resizing the window will cause it to happen early, but hey, close enough
* BUGFIX: Fixed extra nasty bug where logic on certain things like charging your magic would pause the amount of time you used TAB to skip time

- Note:  Just to be safe, save state version has changed, so old save states won't load

------ Change log for 1.8.7 ----------

* Savestates should now properly restore the background in dmods that use mouse controls and tiles at the same time
* Some tweaks with when mouse buttons are processed (only in mouse cursor modes or a dialog menu)
* Misc work on touch controls to prepare for the mobile releases
* Added some names to credits (if you notice I added anybody twice or would rather be credited a different way or not at all, let me know!)
* Fixed bug that could sort of over-write random data if more than 100 sprites were active.. wow, bad

------ Change log for 1.8.8 ----------

* Mobile GUI tweaks
* Added option to disable screen scroll transitions, one of my Android test devices has the slowest glread in the world, it's like 1 full second to walk to the next screen because of it.  This might make dmods that 
use timing (Myster island camera sequence for example) easier though, not sure.
* Fixed issue where behavior of two load_sequence commands in a row on the same sequence differed from 1.08 (in 1.08, max frame is set to the second one as long as the first one wasn't actually loaded yet.  key now shows up in TGKA)
* Fixed regression where offsets in Mayhem were incorrect on the hand mouse pointer
* Upped max frames per seq to 100 instead of 50, fixes Echoes of the Ancient.  I should probably make it dynamic but changing one const number is much easier!
* Now gives a clear warning in the log.txt if a seq tries to go beyond what it can handle and will truncate it (better than breaking the whole sequence as it did before)
* Added support for a weird bmp header type, fixes incorrect palette issue in the dmod The Orb of Darkness
* Fixed life/exp/health to correctly interpolate between values like the original dink, huge thanks to Dan's detailed bug report on this, he even made a mod which made testing the fix a breeze!
* BUGFIX: .dmod files detected in the main Dink HD dir are now properly deleted after auto-installing them
* (Windows) Key binding changes: F1 now brings up the Dink Menu (Shift-Escape was .. weird..) and quicksave was moved to F4

- Note:  Save state version has changed, old save states won't load!

------ Change log for 1.89 ----------

 * Added Dan Walma's .png support (untested)
 * Another bugfix in the .dmod decompresser, fixes issue with Attack of the veggies
 * (Windows) Holding Shift in addition to Tab will cause "super turbo" mode, speeding up the game faster than just holding Tab.  (Secret tip:  If you really want to see speed, hold down the Control key too)

 
------ Change log for 1.90 ----------

 * iOS version recompiled as Release instead of debug, oops
 * OSX version released as a code-signed secure download from rtsoft.com.  Removed from Mac App Store.  It supports notifying the user about
 new versions, same as the Windows version.  Like the windows version, the window can be stretched to any size, but it lacks a full screen option.

 ------ Change log for 1.91 ----------

* This release fixes CVE-2018-0496: Sylvain Beucler and Dan Walma discovered several directory traversal issues in DFArc, an extension manager for the Dink Smallwood game, allowing an attacker to overwrite arbitrary files on the user's system. (While this doesn't use DFArc, some of the same "zip slip" issues applied)
* Applied Dan Walma's fix so DinkHD would not incorrectly interpret sp_frame(x, -1) as sp_frame(x, 1), this fixes an issue with the Malachi the Jerk DMOD

 ------ Change log for 1.92 ----------
 * (Bugfix) Fixed issue that caused graphical glitches in the DMOD Revolution
 * Looping sounds like fire/save machines now properly pause when app loses focus
 