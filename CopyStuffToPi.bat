:make sure you set the IP of your rasperry in SetupVars.bat!
call base_setup.bat
:To test if your ssh is setup to logon without a password, do the below first
:ssh -v pi@%RASPBERRYIP%

echo Syncing proton base over to the PI, this can take serveral minutes the first time... it isn't crashed, just wait!
wsl rsync -v %RSYNC_OPTIONS% /mnt/d/projects/proton/shared %LINUXUSER%@%RASPBERRYIP%:~/proton
