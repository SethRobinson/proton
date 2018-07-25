call build_prepare.bat
call ant debug

:and finally, load it into the emulator

REM Waiting for device to get ready...
:adb wait-for-device -s emulator-5554

REM Installing...
:adb -s %ANDROID_EMU% install -r bin\%APP_NAME%-debug.apk
call InstallOnDefaultPhone.bat
:adb logcat
pause
