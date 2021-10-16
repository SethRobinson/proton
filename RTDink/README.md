# Dink Smallwood HD

-- To just download working versions to play Dink Smallwood:

Visit https://www.rtsoft.com/pages/dink.php for installers for Windows, Mac, iOS, Android

-- How to compile and run the source to create dink.exe on Windows using Visual Studio 2017:

* First, be able to compile and run the proton example RTSimpleApp.  More info at www.protonsdk.com on installing and setting up proton
* Move the RTDink directory checkout to a sub directory of your proton dir. (it works exactly like a proton example)
* Sign up at fmod.com and download FMod Studio for Windows.  Unzip to <proton dir>\shared\win\fmodstudio, so you should have a proton\shared\win\fmodstudio\api dir, etc.
* Install Visual Studio 2017 (Community version works fine and is free) and open RTDink\windows_vs2017\iPhoneRTDink.sln
* By default, Proton SDK's main.cpp is setup to compile for iPhone most likely.  Assuming you'd rather not have an iPhone build, search the project for "//WORK: Change device emulation here" and right under that,
	change it from string desiredVideoMode = "iPhone Landscape"; or whatever it was to "string desiredVideoMode = "Windows"; instead.  (this is where you can emulate many devices and sizes)
* Set the profile to "Release GL" and "Win32".  (or "Debug GL" is ok too)  Compile.  If it worked, you should have a dink.exe created in DinkHD/bin.
* Install DinkHD from rtsoft.com. (media is not included on here, so this is a way to get it..)  Overwrite its dink.exe and fmod.dll (as that is probably different now) with your new one.  It should run!


Use the "Debug GL" or "Release GL" solution configuations in 32 bit.  Debug GL 64 bit is also setup (just to test - as I actually don't package a 64 bit version for Windows, just iOS)

--- Have a bugfix or patch?! Please send it over to Seth!  Please note that any submission (code, media, translations, ect) must be 100% compatible with the license as listed in the source license

See script/installer/readme.txt for what's new info.

-- Note about various ports

* While this is the source code used for the mobile and html5 versions too, not everything is included to build those versions