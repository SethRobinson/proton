:This has to be hand edited if you add/remove .cpp files to your project.  Running this .bat by double clicking in
:Windows explorer will build the html5 (javascript webbrowser version)
:See http://www.rtsoft.com/wiki/doku.php?id=proton:html5_setup for info on how to setup Emscripten

call ../app_info_setup.bat

:Set below to DEBUG=1 for debug mode builds - slower but way easier to see problems
SET DEBUG=0

SET SHARED=..\..\shared

SET APP=..\source

SET ARDUBOY=..\Arduboy

SET SHARED_ARDUBOY=%SHARED%\Arduboy


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

set SRC= %SHARED%\PlatformSetup.cpp  %SHARED%\html5\HTML5Main.cpp %SHARED%\html5\HTML5Utils.cpp ^
%SHARED%\Audio\AudioManager.cpp %SHARED%\Audio\AudioManagerSDL.cpp %CLANMATH%\angle.cpp %CLANMATH%\mat3.cpp %CLANMATH%\mat4.cpp %CLANMATH%\rect.cpp %CLANMATH%\vec2.cpp %CLANMATH%\vec3.cpp ^
%CLANMATH%\vec4.cpp %SHARED%\Entity\Entity.cpp %SHARED%\Entity\Component.cpp %SHARED%\GUI\RTFont.cpp %SHARED%\Manager\Console.cpp %SHARED%\FileSystem\FileManager.cpp ^
%SHARED%\Manager\GameTimer.cpp %SHARED%\Manager\MessageManager.cpp %SHARED%\Manager\ResourceManager.cpp %SHARED%\Manager\VariantDB.cpp %SHARED%\Math\rtPlane.cpp ^
%SHARED%\Math\rtRect.cpp %SHARED%\Renderer\RenderBatcher.cpp %SHARED%\Renderer\SoftSurface.cpp %SHARED%\Renderer\Surface.cpp %SHARED%\Renderer\SurfaceAnim.cpp ^
%SHARED%\util\CRandom.cpp %SHARED%\util\GLESUtils.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\MiscUtils.cpp %SHARED%\util\RenderUtils.cpp %SHARED%\util\ResourceUtils.cpp ^
%SHARED%\util\Variant.cpp %SHARED%\util\boost\libs\signals\src\connection.cpp %SHARED%\util\boost\libs\signals\src\named_slot_map.cpp %SHARED%\util\boost\libs\signals\src\signal_base.cpp ^
%SHARED%\util\boost\libs\signals\src\slot.cpp %SHARED%\util\boost\libs\signals\src\trackable.cpp %SHARED%\FileSystem\StreamingInstance.cpp %SHARED%\FileSystem\StreamingInstanceZip.cpp ^
%SHARED%\FileSystem\StreamingInstanceFile.cpp %SHARED%\BaseApp.cpp %SHARED%\util\unzip\unzip.c %SHARED%\util\unzip\ioapi.c %SHARED%\Entity\EntityUtils.cpp


REM **************************************** ZLIB SOURCE CODE FILES
set ZLIB_SRC=%ZLIBPATH%/deflate.c %ZLIBPATH%/gzio.c %ZLIBPATH%/infback.c %ZLIBPATH%/inffast.c %ZLIBPATH%/inflate.c %ZLIBPATH%/inftrees.c %ZLIBPATH%/trees.c %ZLIBPATH%/uncompr.c %ZLIBPATH%/zutil.c %ZLIBPATH%/adler32.c %ZLIBPATH%/compress.c %ZLIBPATH%/crc32.c

REM **************************************** ENGINE COMPONENT SOURCE CODE FILES
set COMPONENT_SRC=%COMPPATH%\Button2DComponent.cpp %COMPPATH%\FilterInputComponent.cpp %COMPPATH%\FocusInputComponent.cpp %COMPPATH%\FocusRenderComponent.cpp %COMPPATH%\FocusUpdateComponent.cpp ^
%COMPPATH%\HTTPComponent.cpp %COMPPATH%\InputTextRenderComponent.cpp %COMPPATH%\InterpolateComponent.cpp %COMPPATH%\OverlayRenderComponent.cpp %COMPPATH%\ProgressBarComponent.cpp ^
%COMPPATH%\RectRenderComponent.cpp %COMPPATH%\ScrollBarRenderComponent.cpp %COMPPATH%\ScrollComponent.cpp %COMPPATH%\TapSequenceDetectComponent.cpp %COMPPATH%\TextBoxRenderComponent.cpp ^
%COMPPATH%\TextRenderComponent.cpp %COMPPATH%\TouchStripComponent.cpp %COMPPATH%\TrailRenderComponent.cpp %COMPPATH%\TyperComponent.cpp %COMPPATH%\UnderlineRenderComponent.cpp ^
%COMPPATH%\TouchHandlerComponent.cpp %COMPPATH%\CustomInputComponent.cpp %COMPPATH%\SelectButtonWithCustomInputComponent.cpp %COMPPATH%\SliderComponent.cpp %COMPPATH%\EmitVirtualKeyComponent.cpp ^
%COMPPATH%\RenderScissorComponent.cpp %COMPPATH%\ArcadeInputComponent.cpp %COMPPATH%\TouchHandlerArcadeComponent.cpp %COMPPATH%\EmitVirtualKeyComponentAdvanced.cpp

SET ARDUBOY_SRC=%SHARED_ARDUBOY%\Arduboy.cpp %SHARED_ARDUBOY%\ab_printer.cpp %SHARED_ARDUBOY%\ArduboyPlaytune.cpp %SHARED_ARDUBOY%\EEPROM.cpp %SHARED_ARDUBOY%\MyEEPROM.cpp ^
%SHARED_ARDUBOY%\Print.cpp %SHARED_ARDUBOY%\ab_logo.c %SHARED_ARDUBOY%\Print.cpp %SHARED_ARDUBOY%\glcdfont.c %SHARED_ARDUBOY%\WString.cpp %SHARED_ARDUBOY%\WMath.cpp ^
%SHARED_ARDUBOY%\core\core.cpp %SHARED_ARDUBOY%\audio\audio.cpp


REM **************************************** APP SOURCE CODE FILES
set APP_SRC=%APP%\App.cpp %ARDUBOY%\ardu_main.cpp %ARDUBOY%\breakout_bitmaps.cpp
REM **************************************** END SOURCE

:unused so far: -s USE_GLFW=3 -s TOTAL_MEMORY=67108864 -s NO_EXIT_RUNTIME=1 -s FORCE_ALIGNED_MEMORY=1
:To skip font loading so it needs no resource files or zlib, add  -DC_NO_ZLIB
SET CUSTOM_FLAGS= -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DPLATFORM_HTML5 -DRT_USE_SDL_AUDIO -DNDEBUG -DC_GL_MODE -s LEGACY_GL_EMULATION=1 --memory-init-file 0 -Wno-switch -D__AVR_ATmega32U4__ -DRT_ARDUDEV -DRT_RUNS_IN_BACKGROUND -DRT_EMTERPRETER_ENABLED -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1

:unused:   -s FULL_ES2=1

IF %DEBUG% EQU 0 (
echo Compiling in release mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -O2 
) else (
echo Compiling in debug mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -D_DEBUG -s GL_UNSAFE_OPTS=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s EXCEPTION_DEBUG=1 -s DEMANGLE_SUPPORT=1 -s ALIASING_FUNCTION_POINTERS=0 -s SAFE_HEAP=1
)

SET INCLUDE_DIRS=-I%SHARED% -I%SHARED_ARDUBOY% -I%APP% -I%ARDUBOY% -I../../shared/util/boost -I../../shared/ClanLib-2.0/Sources -I../../shared/Network/enet/include ^
-I%ZLIBPATH% -I../../shared/win/include

del %APP_NAME%.js
del %APP_NAME%.html

call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%ZLIB_SRC% %SRC% -o temp.bc

call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%APP_SRC% %ARDUBOY_SRC% %COMPONENT_SRC% temp.bc ^
--embed-file ../bin/interface@interface/ --embed-file ../bin/audio@audio/ -o %APP_NAME%.html
del temp.bc

REM Make sure the file compiled ok
if not exist %APP_NAME%.js beeper.exe /p

IF "%1" == "nopause" (
echo no pause wanted
) else (
echo Compile complete.
pause
)



