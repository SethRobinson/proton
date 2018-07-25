call app_info_setup.bat
:adb wait-for-device -s %ANDROID_DEVICE1%
adb install -r bin\%APP_NAME%-release.apk
pause