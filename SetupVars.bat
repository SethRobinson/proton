:This is only used by windows build scripts for working on the raspberry pie currently
IF DEFINED _PROTON_VARS_SET_ (goto skip)
SET _PROTON_VARS_SET_=true

:Need Grysnc linux tools for windows, used for ssh and rsyncing stuff: http://sourceforge.net/projects/grsync-win/
SET PATH=C:\tools\Grsync\bin;%PATH%

:don't want to file windows junk. Change -az to -avz for verbose to see what's copying...
SET RSYNC_OPTIONS=-az --chmod=Du=rwx,Dgo=rx,Fu=rw,Fgo=r --exclude='*.ncb' --exclude='*.lib' --exclude='*.exe' --exclude='*.dll' --exclude='*.pdb' --exclude='*.obj' --exclude='*.o' --exclude='*.ilk' --exclude='*.so' --exclude='.svn' --exclude='*.pch' --exclude='*.ipch' --exclude='.git'

:Need to know where your PI is, to ssh to (in my case, it's a local ip)
SET RASPBERRYIP=192.168.1.54

:Name we should use when we ssh on
SET LINUXUSER=pi

:As for password, setup a rsa key pair so you don't need one (kind of tricky.. find a tutorial somewhere)
:skip
