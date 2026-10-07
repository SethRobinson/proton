@setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1
:Copy a couple glue Java files we share between Proton projects, don't edit these as they are overwritten here
copy ..\..\shared\android\v3_src\*.java app\src\main\java\com\rtsoft\RTAndroidApp

:Copy over graphics and sounds so they get included in the apk
SET ASSET_DIR=app\src\main\assets
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "AndroidGradle\app\src\main\assets" -Recurse
if errorlevel 1 exit /b 1

mkdir %ASSET_DIR%

mkdir %ASSET_DIR%\interface
IF EXIST ..\bin\interface xcopy ..\bin\interface %ASSET_DIR%\interface /E /F /Y

mkdir %ASSET_DIR%\audio

IF EXIST ..\bin\audio xcopy ..\bin\audio %ASSET_DIR%\audio /E /F /Y

mkdir %ASSET_DIR%\game
IF EXIST ..\bin\game xcopy ..\bin\game %ASSET_DIR%\game /E /F /Y