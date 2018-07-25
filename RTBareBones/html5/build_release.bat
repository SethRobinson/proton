:Set below to DEBUG=1 for debug mode builds - slower but way easier to see problems. Disables the ASYNC stuff as that doesn't seem to play
:well with the rest

SET DEBUG=0

:Set to 1, this causes some internal build changes, and the CustomMainFullTemplate.html to be copied to <AppName>.html instead of the default emscripten one
:Set to 0, you can easily see LogMsg's as they appear in the text window under the app area, so that might be better for debugging

SET USE_HTML5_CUSTOM_MAIN=1
:If 1, set one of these as well:

:SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMain4-3AspectRatioTemplate.html
:SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMain3-2AspectRatioTemplate.html
SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMainFullTemplate.html


set CURPATH=%cd%
cd ..
call app_info_setup.bat
:um, why does the emsdk_env.bat not fully work unless I'm in the emscripten dir?  Whatever, we'll move there and then back
cd %EMSCRIPTEN_ROOT%
call emsdk_env.bat
:Move back to original directory
cd %CURPATH%


where /q emsdk_env.bat

if ERRORLEVEL 1 (
    ECHO You need the environmental EMSCRIPTEN_ROOT set.  This should be set in setup_base.bat in proton's main dir, then called from app_info_setup.bat.
     beeper
     pause
     exit
) 

where /q sed

if ERRORLEVEL 1 (
    ECHO You need the utility sed in your path if you want insert.bat to work. Install tortoisegit, I think it comes with that.
     beeper
     pause
     exit
) 


SET SHARED=..\..\shared

SET APP=..\source

SET COMPPATH=%SHARED%\Entity
SET CLANMATH=%SHARED%\ClanLib-2.0\Sources\Core\Math
SET ZLIBPATH=%SHARED%\util\zlib
set PPATH=%SHARED%\Renderer\linearparticle\sources
set COMPPATH=%SHARED%\Entity
set PNGSRC=%SHARED%\Irrlicht\source\Irrlicht\libpng
set JPGSRC=%SHARED%\Irrlicht\source\Irrlicht\jpeglib
set LZMASRC=%SHARED%\Irrlicht\source\Irrlicht\lzma


:unused, for networking
:%SHARED%\Network\NetHTTP.cpp %SHARED%\Network\NetSocket.cpp %SHARED%\Network\NetUtils.cpp


set SRC= %SHARED%\html5\HTML5Main.cpp %SHARED%\html5\HTML5Utils.cpp %SHARED%\PlatformSetup.cpp ^
%SHARED%\Audio\AudioManager.cpp %CLANMATH%\angle.cpp %CLANMATH%\mat3.cpp %CLANMATH%\mat4.cpp %CLANMATH%\rect.cpp %CLANMATH%\vec2.cpp %CLANMATH%\vec3.cpp ^
%CLANMATH%\vec4.cpp %SHARED%\Entity\Entity.cpp %SHARED%\Entity\Component.cpp %SHARED%\GUI\RTFont.cpp %SHARED%\Manager\Console.cpp %SHARED%\FileSystem\FileManager.cpp ^
%SHARED%\Manager\GameTimer.cpp %SHARED%\Manager\MessageManager.cpp %SHARED%\Manager\ResourceManager.cpp %SHARED%\Manager\VariantDB.cpp %SHARED%\Math\rtPlane.cpp ^
%SHARED%\Math\rtRect.cpp %SHARED%\Renderer\RenderBatcher.cpp %SHARED%\Renderer\SoftSurface.cpp %SHARED%\Renderer\Surface.cpp %SHARED%\Renderer\SurfaceAnim.cpp ^
%SHARED%\util\CRandom.cpp %SHARED%\util\GLESUtils.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\MiscUtils.cpp %SHARED%\util\RenderUtils.cpp %SHARED%\util\ResourceUtils.cpp ^
%SHARED%\util\Variant.cpp %SHARED%\util\boost\libs\signals\src\connection.cpp %SHARED%\util\boost\libs\signals\src\named_slot_map.cpp %SHARED%\util\boost\libs\signals\src\signal_base.cpp ^
%SHARED%\util\boost\libs\signals\src\slot.cpp %SHARED%\util\boost\libs\signals\src\trackable.cpp %SHARED%\FileSystem\StreamingInstance.cpp %SHARED%\FileSystem\StreamingInstanceZip.cpp ^
%SHARED%\FileSystem\StreamingInstanceFile.cpp %SHARED%\BaseApp.cpp %SHARED%\util\unzip\unzip.c %SHARED%\util\unzip\ioapi.c

REM **************************************** ENGINE COMPONENT SOURCE CODE FILES
set COMPONENT_SRC=%COMPPATH%\CustomInputComponent.cpp %COMPPATH%\FocusInputComponent.cpp %COMPPATH%\FocusUpdateComponent.cpp %COMPPATH%\ArcadeInputComponent.cpp


REM **************************************** ZLIB SOURCE CODE FILES
set ZLIB_SRC=%ZLIBPATH%/deflate.c %ZLIBPATH%/gzio.c %ZLIBPATH%/infback.c %ZLIBPATH%/inffast.c %ZLIBPATH%/inflate.c %ZLIBPATH%/inftrees.c %ZLIBPATH%/trees.c %ZLIBPATH%/uncompr.c %ZLIBPATH%/zutil.c %ZLIBPATH%/adler32.c %ZLIBPATH%/compress.c %ZLIBPATH%/crc32.c

REM **************************************** APP SOURCE CODE FILES
set APP_SRC=%APP%\App.cpp
REM **************************************** END SOURCE

:unused so far: -s USE_GLFW=3 -s NO_EXIT_RUNTIME=1 -s FORCE_ALIGNED_MEMORY=1 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -DRT_EMTERPRETER_ENABLED
:To skip font loading so it needs no resource files or zlib, add  -DC_NO_ZLIB
:-Wno-c++11-compat-deprecated-writable-strings -Wno-shift-negative-value

SET CUSTOM_FLAGS=-DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DPLATFORM_HTML5 -DRT_USE_SDL_AUDIO -DRT_JPG_SUPPORT -DC_GL_MODE -s WASM=1 -s LEGACY_GL_EMULATION=1 -s TOTAL_MEMORY=16MB -Wno-c++11-compat-deprecated-writable-strings -Wno-shift-negative-value -s ALLOW_MEMORY_GROWTH=1

SET SECOND_CUSTOM_FLAGS=-
:unused:   -s FULL_ES2=1 --emrun

IF %USE_HTML5_CUSTOM_MAIN% EQU 1 (
:add this define so we'll manually call mainf from the html later instead of it being auto
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -DRT_HTML5_USE_CUSTOM_MAIN -s EXPORTED_FUNCTIONS=['_mainf','_PROTON_SystemMessage','_PROTON_GUIMessage'] -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
SET FINAL_EXTENSION=js
) else (
SET FINAL_EXTENSION=html
)

IF %DEBUG% EQU 0 (
echo Compiling in release mode
: -DRT_EMTERPRETER_ENABLED 
SET CUSTOM_FLAGSS=%CUSTOM_FLAGS% -O2 -DNDEBUG -DPLATFORM_HTML5 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -DRT_EMTERPRETER_ENABLED 
) else (
echo Compiling in debug mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -D_DEBUG -s GL_UNSAFE_OPTS=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s EXCEPTION_DEBUG=1 -s DEMANGLE_SUPPORT=1 -s ALIASING_FUNCTION_POINTERS=0 --emrun
)


SET INCLUDE_DIRS=-I%SHARED% -I%APP% -I../../shared/util/boost -I../../shared/ClanLib-2.0/Sources -I../../shared/Network/enet/include ^
-I%ZLIBPATH%

:compile some libs into a separate thing, otherwise our list of files is too long and breaks stuff

del %APP_NAME%.js*
del %APP_NAME%.html
del %APP_NAME%.wasm*
del %APP_NAME%.data

del %APP_NAME%.mem
del temp.bc


:grab our shared WebLoaderData, this has default graphics and scripts that handle various emscripten/html5 communication
:if you need to customize it, you can stop copying these and customize yours instead
mkdir WebLoaderData
copy /Y ..\..\shared\html5\templates\WebLoaderData .\WebLoaderData

call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%SRC% %APP_SRC% %COMPONENT_SRC% %ZLIB_SRC% ^
--preload-file ../bin/interface@interface/  --js-library %SHARED%\html5\SharedJSLIB.js -o %APP_NAME%.%FINAL_EXTENSION%

REM Make sure the file compiled ok
if not exist %APP_NAME%.js beeper.exe /p

IF %USE_HTML5_CUSTOM_MAIN% EQU 1 (
sed 's/RTTemplateName/%APP_NAME%/g' %CUSTOM_TEMPLATE% > %APP_NAME%.html
) 

IF "%1" == "nopause" (
echo no pause wanted
) else (
echo Compile complete.
pause
)



