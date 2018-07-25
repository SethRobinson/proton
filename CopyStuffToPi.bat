:make sure you set the IP of your rasperry in SetupVars.bat!
call SetupVars.bat
:To test if your ssh is setup to logon without a password, do the below first
:ssh -v pi@%RASPBERRYIP%

echo Syncing proton base over to the PI, this can take serveral minutes the first time... it isn't crashed, just wait!
rsync %RSYNC_OPTIONS% shared %LINUXUSER%@%RASPBERRYIP%:~/proton
