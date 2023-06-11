call app_info_setup.bat
cd ..
:START
echo Sync up the entire proton tree
call CopyStuffToPi.bat
:Change -az to -avz for verbose to see what's copying...
echo Syncing app directory to pi, hope you have WSL and rsync installed... this can take a while the first time.

wsl rsync -v %RSYNC_OPTIONS% /mnt/d/projects/proton/%APP_NAME% %LINUXUSER%@%RASPBERRYIP%:~/proton

echo Now let's tell the PI to compile stuff locally
ssh %LINUXUSER%@%RASPBERRYIP% "cd /home/%LINUXUSER%/proton/%APP_NAME%/linux;sh linux_compile.sh"
:pause
goto START
