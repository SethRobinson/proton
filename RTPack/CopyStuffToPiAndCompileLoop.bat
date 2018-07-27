call app_info_setup.bat
call ../SetupVars.bat
cd ..
:START
call CopyStuffToPi.bat
:Change -az to -avz for verbose to see what's copying...
rsync %RSYNC_OPTIONS% %APP_NAME% %LINUXUSER%@%RASPBERRYIP%:~/proton
echo Syncing app files over to the PI, takes a while the first time
ssh %LINUXUSER%@%RASPBERRYIP% "cd /home/%LINUXUSER%/proton/%APP_NAME%/linux;sh linux_compile.sh"
:pause
goto START
