call app_info_setup.bat
cd ..
:START
call CopyStuffToPi.bat
:Change -az to -avz for verbose to see what's copying...
wsl rsync -v %RSYNC_OPTIONS% /mnt/d/projects/proton/%APP_NAME% %LINUXUSER%@%RASPBERRYIP%:~/proton

echo Now let's tell the PI to compile stuff locally
ssh %LINUXUSER%@%RASPBERRYIP% "cd /home/%LINUXUSER%/proton/%APP_NAME%/linux;sh linux_compile.sh"

#echo Now run it
#ssh %LINUXUSER%@%RASPBERRYIP% "DISPLAY=:0 sh amigo.sh"

:pause
goto START
