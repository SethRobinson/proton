cd ..\android
call ..\app_info_setup.bat
call buildRelease.bat
@ECHO ON
cd bin
set D_FILE_NAME=%APP_NAME%-release.apk
copy %D_FILE_NAME% ..\..\script
cd ..\..\script
set d_fname=%D_FILE_NAME%
call FTPToSiteWin.bat

pause