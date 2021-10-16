echo For security reasons, Chrome won't properly download data from a file// address.  By running this python based webserver chrome testing will work.  I think it requires Python 3+ to be installed, this might happen automatically when you setup the Emscripten stuff

set CURPATH=%cd%
cd ..
call app_info_setup.bat
:um, why does the emsdk_env.bat not fully work unless I'm in the emscripten dir?  Whatever, we'll move there and then back
cd %EMSCRIPTEN_ROOT%
call emsdk_env.bat
:Move back to original directory
cd %CURPATH%

emrun %APP_NAME%.html
pause

exit

:Another method that could be used, using a python server
start http://localhost:8000/%APP_NAME%.html

echo Ctrl-Pause to kill the server
python -m http.server --bind 127.0.0.1 8000
pause
