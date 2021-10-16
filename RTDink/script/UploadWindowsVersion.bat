SET C_FILENAME=DinkSmallwoodHDInstaller.exe
:del %C_FILENAME%
:copy DinkSmallwoodHDInstaller.exe %C_FILENAME%
set d_fname=%C_FILENAME%
call FTPToSiteWin.bat
pause