@echo off

REM script to compile proton's RTConsole on Windows using the CMakeLists
REM you can also use vsc's inbuilt "Build" button. Just click on it!

if exist build\RTConsole.exe del build\RTConsole.exe
if exist build rd /s /q build
mkdir build
cd build

cmake ..
cmake --build . --config Release
REM -- /j 4

echo Copying binaries to ..\bin directory, run from there!
xcopy /y Release\RTConsole.exe ..\..\bin

cd ..