
REM This will FTP the latest build into the correct dir at RTSOFT

if "%d_fname%" == "" ( 
   echo d_fname not set.
   beeper.exe /p
   exit
  )

REM get our ftp logon info
call ../../SetFTPLogonInfo.bat

REM create script for the FTP process

if exist ftp.tmp del ftp.tmp

echo user %_FTP_USER_% %_FTP_PASS_% >> ftp.tmp
echo cd www >> ftp.tmp
echo cd temp2 >> ftp.tmp
echo binary >> ftp.tmp
echo put %d_fname% >> ftp.tmp
echo quit >> ftp.tmp


echo http://www.rtsoft.com/temp2/%d_fname%

ftp -n -i -d -s:ftp.tmp %_FTP_SITE_%

del ftp.tmp

REM remove environmental vars we used
set C_FILENAME=
call ../../KillFTPLogonInfo.bat

