@echo off

REM script to compile proton's RTConsole on Windows using the CMakeLists
REM you can also use vsc's inbuilt "Build" button. Just click on it!

if exist ..\build rd /s /q ..\build
mkdir ..\build
cd ..\build

REM need to use ninja generator since CMAKE_EXPORT_COMPILE_COMMANDS ON needs it
cmake -G Ninja ..\windows
cmake --build . --config Release

echo Copying binaries to ..\bin directory, run from there!
xcopy /y RTConsole.exe ..\..\bin

cd ..