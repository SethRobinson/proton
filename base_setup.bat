:This is only used by windows build scripts for working on the raspberry pie currently
IF DEFINED _PROTON_VARS_SET_ (goto skip)
SET _PROTON_VARS_SET_=true

:This file is likely run by all project's app_info_setup.bat file, it allows you to set some global paths for all proton projects if needed.
Echo Setting defaults and US code page
CHCP 437

SET UNITY_EXE=C:\pro\unity5\Editor\Unity.exe
SET EMSCRIPTEN_ROOT=C:\pro\emscripten
:Need Grysnc linux tools for windows, used for ssh and rsyncing stuff: :http://sourceforge.net/projects/grsync-win/
SET PATH=C:\tools\Grsync\bin;%PATH%


: Below is used to find the Raspberry pi and command line utils like ssh, don't really need to set this stuff up but it makes various scripts work that Seth uses

:don't want to copy file windows junk. Change -az to -avz for verbose to see what's copying...
SET RSYNC_OPTIONS=-az --chmod=Du=rwx,Dgo=rx,Fu=rw,Fgo=r --exclude='*.ncb' --exclude='*.lib' --exclude='*.exe' --exclude='*.dll' --exclude='*.pdb' --exclude='*.obj' --exclude='*.o' --exclude='*.ilk' --exclude='*.so' --exclude='.svn' --exclude='*.pch' --exclude='*.ipch' --exclude='.git'

:Need to know where your PI is, to ssh to (in my case, it's a local ip)
SET RASPBERRYIP=192.168.1.12

:Name we should use when we ssh on
SET LINUXUSER=pi

:As for password, setup a rsa key pair so you don't need one (kind of tricky.. find a tutorial somewhere)
:skip