@echo off
setlocal DisableDelayedExpansion
cd /d "%~dp0" || exit /b 1

REM script to compile proton's RTConsole on Windows using the CMakeLists
REM you can also use vsc's inbuilt "Build" button. Just click on it!

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\..\shared\win\utils\SafeRemove.ps1" -Root "%~dp0.." -RelativePath "build" -Recurse
if errorlevel 1 exit /b 1
mkdir ..\build
cd /d "%~dp0..\build" || exit /b 1

REM need to use ninja generator since CMAKE_EXPORT_COMPILE_COMMANDS ON needs it
cmake -G Ninja ..\windows
cmake --build . --config Release

echo Copying binaries to ..\bin directory, run from there!

cd ..
