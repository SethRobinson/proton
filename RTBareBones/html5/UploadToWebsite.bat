SET _FTP_USER_=rtsoft
SET _FTP_SITE_=rtsoft.com
SET WEB_SUB_DIR=temp


set CURPATH=%cd%
cd ..
call app_info_setup.bat
cd %CURPATH%

if not exist %APP_NAME%.js beeper.exe /p
:Get rid of files we don't actually need
del %APP_NAME%.js.orig.js
del temp.bc
:SSH transfer, this assumes you have ssh and valid keys setup already

ssh %_FTP_USER_%@%_FTP_SITE_% "mkdir ~/www/%WEB_SUB_DIR%"
ssh %_FTP_USER_%@%_FTP_SITE_% "rm -rf ~/www/%WEB_SUB_DIR%/WebLoaderData"
rsync -avzr --chmod=Du=rwx,Dgo=rx,Fu=rw,Fgo=r  -e "ssh" %APP_NAME%*.* %_FTP_USER_%@%_FTP_SITE_%:www/%WEB_SUB_DIR%
rsync -avzr --chmod=Du=rwx,Dgo=rx,Fu=rw,Fgo=r  -e "ssh" WebLoaderData %_FTP_USER_%@%_FTP_SITE_%:www/%WEB_SUB_DIR%

:Let's go ahead an open a browser to test it
start http://www.%_FTP_SITE_%/%WEB_SUB_DIR%/%APP_NAME%.html
exit

:Old way, non ssh
REM get our ftp logon info
:call d:\projects\SetFTPLogonInfo.bat

:ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/%WEB_SUB_DIR% %APP_NAME%*
:ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/%WEB_SUB_DIR% WebLoaderData

:echo Files uploaded:  http://www.%_FTP_SITE_%/%WEB_SUB_DIR%

:Let's go ahead an open a browser to test it
:start http://www.%_FTP_SITE_%/%WEB_SUB_DIR%/%APP_NAME%.html


