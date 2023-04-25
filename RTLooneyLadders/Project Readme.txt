This example is a basic game (not a great one..) but shows how to link all kinds of input up using ArcadeInputComponent
and GamepadManager. (which is hooked to ArcadeInputComponent so you get directional movement in a unified way)

* Can control game with touch screen, any gamepads connected, and keyboard
* Music only works on Android/Win because there are only .ogg files, iOS would need .mp3 versions or FMOD enabled
* There is a little gamepad tester on the main menu as well.

This game enables all available gamepads at once.  In a real game, you'd probably want to have fine-tuned on/off
type settings in the options.  Tested with Windows and iOS builds only at this point.  Gradle/OSX builds not setup because Seth lazy. Linux build probably doesn't work either.

Don't forget to run media/update_media.bat first.

For the documentation, tutorials, wiki, and forums, please visit:

www.protonsdk.com

-Seth (seth@rtsoft.com)