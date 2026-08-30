:Copy a couple glue Java files we share between Proton projects, don't edit these as they are overwritten here
copy ..\..\shared\android\v3_src\*.java app\src\main\java\com\rtsoft\RTAndroidApp

:Copy over graphics and sounds so they get included in the apk
SET ASSET_DIR=app\src\main\assets
rmdir app\src\main\assets /S /Q

mkdir %ASSET_DIR%

mkdir %ASSET_DIR%\interface
IF EXIST ..\bin\interface xcopy ..\bin\interface %ASSET_DIR%\interface /E /F /Y

mkdir %ASSET_DIR%\audio

IF EXIST ..\bin\audio xcopy ..\bin\audio %ASSET_DIR%\audio /E /F /Y

mkdir %ASSET_DIR%\game
IF EXIST ..\bin\game xcopy ..\bin\game %ASSET_DIR%\game /E /F /Y