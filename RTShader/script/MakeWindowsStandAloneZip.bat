@setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1
call vcvars32.bat
del ..\bin\winRTShaderl.exe
devenv ..\windows\iphoneRTShader.sln /build "Release GL" 

echo Let's zip it up
cd /d "%~dp0..\bin" || exit /b 1
del memleaks.log
del fmod.log
del save.dat
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "bin\*.pdb" -FilesOnly
if errorlevel 1 exit /b 1
set d_fname=iPhoneRTDScroll_Windows_%DATE:~4,2%_%DATE:~7,2%.zip
..\..\shared\win\utils\7za.exe a -x!*.cfg -x!libgles_cm.dll -x!libEGL.dll -x!fmodexL.dll -r -tzip ..\%d_fname%
cd ..
call script\FTPToSite.bat
cd script
pause