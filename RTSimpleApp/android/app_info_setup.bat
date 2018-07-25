::scan the App.cpp file to get info about this product using some batch file tricks and put them into environmental vars for other scripts

set APP_NAME=rtsimpleapp
set SMALL_PACKAGE_NAME=rtsimpleapp
set COMPANY_PACKAGE_NAME=rtsoft
set PACKAGE_NAME=com.%COMPANY_PACKAGE_NAME%.%SMALL_PACKAGE_NAME%
set EMULATOR_AVD=@AVD_16
set ANDROID_EMU=emulator-5554

::Update/write our local.properties file with our ANDROID NDK dir
call android update project -p ./

:Use below to set your ant home dir if it isn't already an environmental var!
:set ANT_HOME=d:\pro\ant
copy ..\..\shared\android\util\javapp.jar %ANT_HOME%\lib\