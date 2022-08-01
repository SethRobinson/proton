@echo off
for /f "tokens=2 delims==" %%G in ('wmic os get localdatetime /value') do set datetime=%%G

set year=%datetime:~0,4%
set month=%datetime:~4,2%
set day=%datetime:~6,2%

echo %year%-%month%-%day%