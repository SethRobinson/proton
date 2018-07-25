call ../app_info_setup.bat

REM get our ftp logon info
call d:\projects\SetFTPLogonInfo.bat

if not exist %APP_NAME%.html beeper.exe /p
if not exist %APP_NAME%.js beeper.exe /p

ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/ %APP_NAME%*

echo File uploaded:  http://www.%_FTP_SITE_%/%APP_NAME%.html

:Let's go ahead an open a browser to test it
start http://www.%_FTP_SITE_%/%APP_NAME%.html


