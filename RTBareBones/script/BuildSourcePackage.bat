set FNAME=%RTBACKUP%\iPhone\iPhoneWinSimpleFramework_%DATE:~4,2%_%DATE:~7,2%.zip
cd ..\..
del %FNAME%
shared\win\utils\7za.exe a -r -tzip %FNAME% shared\* RTBareBones\* -x!*.o -x!*.so  -x!*.o.d -x!*.pch -x!*.sdf -x!build -x!*.rttex -x!*.zip -x!*.svn -x!*.ncb -x!*.bsc -x!*.pdb -x!*.sbr -x!*.ilk -x!*.idb -x!*.obj -x!*.DS_Store -x!._* -x!*.ilk -x!oglespcviewer.cfg -x!shared\win\powerVR  -x!shared\mac\FMOD  -x!PVRT*.* -x!Common-Debug -x!flash\build -x!GLESUtils.* -x!ResourceUtils.* -x!*.pkg -x!*.DS_Store  -x!shared\mysql -x!shared\FliteTTS -x!*.apk -x!android/assets
shared\win\utils\7za.exe a -tzip %FNAME% "Programmer Readme.txt"
pause