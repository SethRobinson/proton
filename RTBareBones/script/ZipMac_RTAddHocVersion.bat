set D_APPNAME=RTBareBones
set D_BUILDNAME=AddHoc

set D_FILE_NAME=iPhone_%D_APPNAME%_AdHoc_%DATE:~4,2%_%DATE:~7,2%
cd ..
del %D_FILE_NAME%.zip
rename %D_APPNAME%AdHoc.zip %D_FILE_NAME%.zip
set d_fname=%D_FILE_NAME%.zip
call script\FTPToSite.bat
cd script
pause