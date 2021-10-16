SET C_FILENAME=DinkSmallwoodHDInstallerBeta.exe
:del %C_FILENAME%
:copy DinkSmallwoodHDInstaller.exe %C_FILENAME%
set d_fname=%C_FILENAME%
call FTPToSiteWin.bat
pause