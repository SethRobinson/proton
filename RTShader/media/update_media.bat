@setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1
echo You don't have to make media for this example, otherwise it will delete your bin/interface directory, but I want this on svn so it's easy to build this most simple example, even for
echo people who don't have windows to make the font.

pause

REM Make fonts

set PACK_EXE=..\..\.\shared\win\utils\RTPack.exe

REM Delete all existing packed textures from this dir
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "media\interface\*.rttex" -Recurse -FilesOnly
if errorlevel 1 exit /b 1

for /r %%f in (font*.txt) do %PACK_EXE% -make_font %%f

REM Process our images and textures and copy them into the bin directory

REM -pvrtc4 for compressed, -pvrt4444 or -pvrt8888 (32 bit)  for uncompressed

:cd game
:for /r %%f in (*.bmp *.png) do ..\%PACK_EXE%  -pvrt8888 %%f
:cd ..

cd interface || exit /b 1
for /r %%f in (*.bmp *.png) do ..\%PACK_EXE%  -pvrt8888 %%f
cd .. || exit /b 1

REM Custom things that don't need preprocessing

REM Final compression
for /r %%f in (*.rttex) do %PACK_EXE% %%f

REM Delete things we don't want copied
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "media\interface\font_*.rttex" -FilesOnly
if errorlevel 1 exit /b 1

REM Preserve the version-controlled bin/interface directory.

REM copy the stuff we care about

mkdir ..\bin\interface
xcopy interface ..\bin\interface /E /F /Y /EXCLUDE:exclude.txt

:Special case, delete the .rttex, for this one example, we only want a .bmp there
:del ..\bin\interface\test.rttex

REM Convert everything to lowercase, otherwise the iphone will choke on the files
REM for /r %%f in (*.*) do ..\media\LowerCase.bat  %%f

del icon.rttex
del default.rttex
pause
