echo Makes a zip of all third party libs from the /shared dir (ie, tapjoy, chartboost, fmod, win gles drivers, etc), 
echo these items can't be put in the proton shared dir so this is a way to give all those items to someone else
echo on your team as a big zip, assuming you added them all.

REM ** Make sure american code page is used, otherwise the %DATE environmental var might be wrong
CHCP 437

set FNAME=ProtonThirdPartyLibs_%DATE:~4,2%_%DATE:~7,2%.zip

del %FNAME% > NULL
rmdir /q /s ztemp
mkdir ztemp
mkdir ztemp\shared

:android stuff
mkdir ztemp\shared\android
xcopy shared\android\optional_src ztemp\shared\android\optional_src /E /F /Y /I

:remove the part that is under version control
rmdir /q /s ztemp\shared\android\optional_src\com\android

:ios stuff
mkdir ztemp\shared\iOS
REM xcopy shared\iOS\60BeatGamepad ztemp\shared\iOS\60BeatGamepad /E /F /Y /I
REM xcopy shared\iOS\ChartBoost ztemp\shared\iOS\ChartBoost /E /F /Y /I
xcopy shared\iOS\fmod ztemp\shared\iOS\fmod /E /F /Y /I
REM xcopy shared\iOS\TapjoyConnectVirtualGoodsSDK_iOS ztemp\shared\iOS\TapjoyConnectVirtualGoodsSDK_iOS /E /F /Y /I

:win stuff
mkdir ztemp\shared\win
xcopy shared\win\fmod ztemp\shared\win\fmod /E /F /Y /I
xcopy shared\win\powerVR ztemp\shared\win\powerVR /E /F /Y /I

:mysql stuff
REM xcopy shared\mysql ztemp\shared\mysql /E /F /Y /I

:make final zip, leaving out the .svn crap

cd ztemp
..\shared\win\utils\7za.exe a -r -tzip  ..\%FNAME% * -x!.svn
cd ..
REM rmdir /q /s ztemp
pause