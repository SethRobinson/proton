@setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1
set D_APPNAME=RTShader
set D_BUILDNAME=AddHoc

set D_FILE_NAME=iPhone_%D_APPNAME%_AdHoc_%DATE:~4,2%_%DATE:~7,2%
cd /d "%~dp0.." || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "iPhone_RTShader_AdHoc_%DATE:~4,2%_%DATE:~7,2%.zip" -FilesOnly
if errorlevel 1 exit /b 1
rename %D_APPNAME%AdHoc.zip %D_FILE_NAME%.zip
set d_fname=%D_FILE_NAME%.zip
call script\FTPToSite.bat
cd script
pause