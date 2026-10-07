@setlocal DisableDelayedExpansion
REM Delete packed textures only below this app's media directory.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "media\*.rttex" -Recurse -FilesOnly
if errorlevel 1 exit /b 1
