@setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1
SET _FTP_USER_=rtsoft
SET _FTP_SITE_=rtsoft.com
SET WEB_SUB_DIR=web/rtsimpleapp

set "CURPATH=%~dp0"
cd /d "%~dp0.." || exit /b 1
set "APP_NAME="
call app_info_setup.bat
if not defined APP_NAME exit /b 1
if not "%APP_NAME%"=="RTSimpleApp" exit /b 1
cd /d "%~dp0" || exit /b 1

if not exist %APP_NAME%.js %RT_UTIL%\beeper.exe /p
:Get rid of files we don't actually need
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "html5\RTSimpleApp.js.orig.js" -FilesOnly
if errorlevel 1 exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "html5\temp.bc" -FilesOnly
if errorlevel 1 exit /b 1
:SSH transfer, this assumes you have ssh and valid keys setup already
ssh %_FTP_USER_%@%_FTP_SITE_% "mkdir ~/www/%WEB_SUB_DIR%"
ssh %_FTP_USER_%@%_FTP_SITE_% "sh -s -- rtsimpleapp" < "%~dp0..\..\shared\linux\clean_web_loader.sh"
if errorlevel 1 exit /b 1
scp %APP_NAME%*.* %_FTP_USER_%@%_FTP_SITE_%:www/%WEB_SUB_DIR%
scp -r WebLoaderData %_FTP_USER_%@%_FTP_SITE_%:www/%WEB_SUB_DIR%
:scp from Windows can create dirs/files without group read, and Apache runs in
:the rtsoft group, so fix modes or WebLoaderData 403s and the loader never runs
ssh %_FTP_USER_%@%_FTP_SITE_% "chmod -R u=rwX,go=rX ~/www/%WEB_SUB_DIR%"

:Let's go ahead an open a browser to test it
start http://www.%_FTP_SITE_%/%WEB_SUB_DIR%/%APP_NAME%.html

pause



