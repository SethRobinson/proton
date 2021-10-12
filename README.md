# Proton SDK

[[Sample]](https://www.youtube.com/watch?v=Nm8G_4_VMiw)

## Compiling

to compile this project you must have [VitaSDK](https://vitasdk.org/) installed.
after you have installed the vitaSDK you can locate `RTBareBones` or `RTSimpleApp` and find `vita` folder which contains `compile.sh` you can use that script to compile the sample after its complete you can find your built binaries at `bin`.

## Running

to run Proton SDK to vita we must manually copy files to the vita to begin with we must generate our media so find your preferred sample and then locate `media` folder you must create folder called `game` in `media` then run `update_media.bat` or `update_media.sh` if you're using linux instead of WSL after the script is done running you should see our files at `bin` after they are there you can copy them to your vita to `ux0:/proton/` using your preferred method (`ftp`, `usb`) after we have our assets at the correct folder we can copy our `vpk` to the vita and then install it and after its done installing run it and you should have proton sdk running on PS Vita!

## TODO List
- Joysticks
- Keypad
- Text Input
- ???

## Credits
- @NabsiYa - Porting Proton over to PS Vita
- @SethRobinson - Making proton SDK

# Want to contribute?

Feel free to submit a pull request!
