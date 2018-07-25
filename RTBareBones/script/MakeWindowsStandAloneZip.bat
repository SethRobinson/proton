call vcvars32.bat
del ..\bin\winRTBareBonesl.exe
devenv ..\windows\iphoneRTBareBones.sln /build "Release GL" 

echo Let's zip it up
cd ..\bin
del memleaks.log
del fmod.log
del save.dat
del *.pdb
set d_fname=iPhoneRTDScroll_Windows_%DATE:~4,2%_%DATE:~7,2%.zip
..\..\shared\win\utils\7za.exe a -x!*.cfg -x!libgles_cm.dll -x!libEGL.dll -x!fmodexL.dll -r -tzip ..\%d_fname%
cd ..
call script\FTPToSite.bat
cd script
pause