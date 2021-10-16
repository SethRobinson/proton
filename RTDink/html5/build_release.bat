:Set below to DEBUG=1 for debug mode builds - slower but way easier to see problems. Disables the ASYNC stuff as that doesn't seem to play
:well with the rest.  It uses Emscripten default html setup which doesn't allow uploading/downloading though.

SET DEBUG=0

:Set to 1, this causes some internal build changes, and the CustomMainFullTemplate.html to be copied to <AppName>.html instead of the default emscripten one
:Set to 0, you can easily see LogMsg's as they appear in the text window under the app area, so that might be better for debugging

SET USE_HTML5_CUSTOM_MAIN=1
:If 1, this is used:


SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMain4-3AspectRatioTemplate.html
:SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMain3-2AspectRatioTemplate.html
:SET CUSTOM_TEMPLATE=..\..\shared\html5\templates\CustomMainFullTemplate.html


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

:Oh, we better build our media just in case
cd ../media
:call update_media.bat
cd ../html5


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

set SRC= %SHARED%\PlatformSetup.cpp  %SHARED%\html5\HTML5Main.cpp %SHARED%\html5\HTML5Utils.cpp %SHARED%\html5\NetHTTP_HTML5.cpp ^
%SHARED%\Audio\AudioManager.cpp %CLANMATH%\angle.cpp %CLANMATH%\mat3.cpp %CLANMATH%\mat4.cpp %CLANMATH%\rect.cpp %CLANMATH%\vec2.cpp %CLANMATH%\vec3.cpp ^
%CLANMATH%\vec4.cpp %SHARED%\Entity\Entity.cpp %SHARED%\Entity\Component.cpp %SHARED%\GUI\RTFont.cpp %SHARED%\Manager\Console.cpp %SHARED%\FileSystem\FileManager.cpp ^
%SHARED%\Manager\GameTimer.cpp %SHARED%\Manager\MessageManager.cpp %SHARED%\Manager\ResourceManager.cpp %SHARED%\Manager\VariantDB.cpp %SHARED%\Math\rtPlane.cpp %SHARED%\Math\WeightRand.cpp ^
%SHARED%\Math\rtRect.cpp %SHARED%\Renderer\RenderBatcher.cpp %SHARED%\Renderer\SoftSurface.cpp %SHARED%\Renderer\Surface.cpp %SHARED%\Renderer\SurfaceAnim.cpp ^
%SHARED%\util\CRandom.cpp %SHARED%\util\GLESUtils.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\MiscUtils.cpp %SHARED%\util\RenderUtils.cpp %SHARED%\util\ResourceUtils.cpp ^
%SHARED%\util\Variant.cpp %SHARED%\util\boost\libs\signals\src\connection.cpp %SHARED%\util\boost\libs\signals\src\named_slot_map.cpp %SHARED%\util\boost\libs\signals\src\signal_base.cpp ^
%SHARED%\util\boost\libs\signals\src\slot.cpp %SHARED%\util\boost\libs\signals\src\trackable.cpp %SHARED%\BaseApp.cpp %SHARED%\util\TextScanner.cpp %SHARED%\Entity\EntityUtils.cpp ^
%SHARED%\Network\NetSocket.cpp %SHARED%\Network\NetUtils.cpp %SHARED%\Audio\AudioManagerFMODStudio.cpp %SHARED%\FileSystem\StreamingInstance.cpp %SHARED%\FileSystem\FileSystem.cpp ^
%SHARED%\FileSystem\StreamingInstanceZip.cpp %SHARED%\FileSystem\StreamingInstanceFile.cpp %SHARED%\FileSystem\FileSystemZip.cpp %SHARED%\util\unzip\unzip.c %SHARED%\util\unzip\ioapi.c %SHARED%\util\PrimeSearch.cpp ^
%SHARED%\Manager\AdManager.cpp %SHARED%/Renderer/JPGSurfaceLoader.cpp %SHARED%\util\archive\TarHandler.cpp %SHARED%\Gamepad\GamepadManager.cpp ^
%SHARED%\Gamepad\GamepadProvideriCade.cpp %SHARED%\Gamepad\Gamepad.cpp %SHARED%\Gamepad\GamepadProvider.cpp %SHARED%\util\bzip2\blocksort.c ^
%SHARED%\util\bzip2\bzlib.c %SHARED%\util\bzip2\crctable.c %SHARED%\util\bzip2\decompress.c %SHARED%\util\bzip2\huffman.c %SHARED%\util\bzip2\randtable.c ^
%SHARED%/util/bzip2/compress.c %SHARED%\Gamepad\GamepadiCade.cpp

set COMPONENT_SRC=%COMPPATH%\Button2DComponent.cpp %COMPPATH%\FilterInputComponent.cpp %COMPPATH%\FocusInputComponent.cpp %COMPPATH%\FocusRenderComponent.cpp %COMPPATH%\FocusUpdateComponent.cpp ^
%COMPPATH%\HTTPComponent.cpp %COMPPATH%\InputTextRenderComponent.cpp %COMPPATH%\InterpolateComponent.cpp %COMPPATH%\OverlayRenderComponent.cpp %COMPPATH%\ProgressBarComponent.cpp ^
%COMPPATH%\RectRenderComponent.cpp %COMPPATH%\ScrollBarRenderComponent.cpp %COMPPATH%\ScrollComponent.cpp %COMPPATH%\TapSequenceDetectComponent.cpp %COMPPATH%\TextBoxRenderComponent.cpp ^
%COMPPATH%\TextRenderComponent.cpp %COMPPATH%\TouchStripComponent.cpp %COMPPATH%\TrailRenderComponent.cpp %COMPPATH%\TyperComponent.cpp %COMPPATH%\UnderlineRenderComponent.cpp ^
%COMPPATH%\TouchHandlerComponent.cpp %COMPPATH%\CustomInputComponent.cpp %COMPPATH%\SelectButtonWithCustomInputComponent.cpp %COMPPATH%\SliderComponent.cpp %COMPPATH%\EmitVirtualKeyComponent.cpp ^
%COMPPATH%\RenderClipComponent.cpp %COMPPATH%\ArcadeInputComponent.cpp %COMPPATH%\UnpackArchiveComponent.cpp


REM **************************************** ZLIB SOURCE CODE FILES
set ZLIB_SRC=%ZLIBPATH%/deflate.c %ZLIBPATH%/gzio.c %ZLIBPATH%/infback.c %ZLIBPATH%/inffast.c %ZLIBPATH%/inflate.c %ZLIBPATH%/inftrees.c %ZLIBPATH%/trees.c %ZLIBPATH%/uncompr.c %ZLIBPATH%/zutil.c %ZLIBPATH%/adler32.c %ZLIBPATH%/compress.c %ZLIBPATH%/crc32.c

REM ****************************** PNG SUPPORT 
SET PNG_SRC=%PNGSRC%/png.c %PNGSRC%/pngerror.c %PNGSRC%/pnggccrd.c %PNGSRC%/pngget.c %PNGSRC%/pngmem.c %PNGSRC%/pngpread.c %PNGSRC%/pngread.c ^
%PNGSRC%/pngrio.c %PNGSRC%/pngrtran.c %PNGSRC%/pngrutil.c %PNGSRC%/pngset.c %PNGSRC%/pngtrans.c %PNGSRC%/pngvcrd.c %PNGSRC%/pngwio.c %PNGSRC%/pngwtran.c


REM **************************************** PARTICLE SYSTEM SOURCE CODE FILES
set PARTICLE_SRC=%PPATH%/L_Defination.cpp %PPATH%/L_DroppingEffect.cpp %PPATH%/L_EffectEmitter.cpp %PPATH%/L_ExplosionEffect.cpp %PPATH%/L_MotionController.cpp %PPATH%/L_Particle.cpp ^
%PPATH%/L_ParticleEffect.cpp %PPATH%/L_ParticleMem.cpp %PPATH%/L_ParticleSystem.cpp %PPATH%/L_ShootingEffect.cpp %PPATH%/L_EffectManager.cpp

REM **************************************** JPEG SOURCE CODE FILES
set JPG_SRC=%JPGSRC%\jcapimin.c %JPGSRC%\jcapistd.c %JPGSRC%\jccoefct.c %JPGSRC%\jccolor.c %JPGSRC%\jcdctmgr.c %JPGSRC%\jchuff.c %JPGSRC%\jcinit.c %JPGSRC%\jcmainct.c ^
%JPGSRC%\jcmarker.c %JPGSRC%\jcmaster.c %JPGSRC%\jcomapi.c %JPGSRC%\jcparam.c %JPGSRC%\jcphuff.c %JPGSRC%\jcprepct.c %JPGSRC%\jcsample.c %JPGSRC%\jctrans.c ^
%JPGSRC%\jdapimin.c %JPGSRC%\jdapistd.c %JPGSRC%\jdatadst.c %JPGSRC%\jdatasrc.c %JPGSRC%\jdcoefct.c %JPGSRC%\jdcolor.c %JPGSRC%\jddctmgr.c ^
%JPGSRC%\jdhuff.c %JPGSRC%\jdinput.c %JPGSRC%\jdmainct.c %JPGSRC%\jdmarker.c %JPGSRC%\jdmaster.c %JPGSRC%\jdmerge.c %JPGSRC%\jdphuff.c %JPGSRC%\jdpostct.c ^
%JPGSRC%\jdsample.c %JPGSRC%\jdtrans.c %JPGSRC%\jerror.c %JPGSRC%\jfdctflt.c %JPGSRC%\jfdctfst.c %JPGSRC%\jfdctint.c %JPGSRC%\jidctflt.c %JPGSRC%\jidctfst.c ^
%JPGSRC%\jidctint.c %JPGSRC%\jidctred.c %JPGSRC%\jmemmgr.c %JPGSRC%\jmemnobs.c %JPGSRC%\jquant1.c %JPGSRC%\jquant2.c %JPGSRC%\jutils.c



REM **************************************** APP SOURCE CODE FILES
set APP_SRC=%APP%\App.cpp %APP%\Component\ActionButtonComponent.cpp %APP%\Component\CursorComponent.cpp %APP%\Component\DragControlComponent.cpp ^
%APP%\Component\FPSControlComponent.cpp %APP%\Component\InventoryComponent.cpp %APP%\dink\dink.cpp %APP%\dink\FFReader.cpp %APP%\dink\misc_util.cpp %APP%\dink\ScriptAccelerator.cpp ^
%APP%\video_gl.cpp ^
%APP%\GUI\AboutMenu.cpp %APP%\GUI\BrowseMenu.cpp %APP%\GUI\DebugMenu.cpp %APP%\GUI\DMODInstallMenu.cpp ^
%APP%\GUI\EnterURLMenu.cpp %APP%\GUI\DMODMenu.cpp %APP%\GUI\GameMenu.cpp %APP%\GUI\LoadMenu.cpp %APP%\GUI\LogMenu.cpp %APP%\GUI\MainMenu.cpp %APP%\GUI\OptionsMenu.cpp ^
%APP%\GUI\PauseMenu.cpp %APP%\GUI\PopUpMenu.cpp %APP%\GUI\QuickTipMenu.cpp %APP%\GUI\ReadTextMenu.cpp %APP%\GUI\ExpiredMenu.cpp 
REM **************************************** END SOURCE

:unused so far: -s USE_GLFW=3 -s NO_EXIT_RUNTIME=1 -s FORCE_ALIGNED_MEMORY=1 -s EMTERPRETIFY=1  -s EMTERPRETIFY_ASYNC=1 -DRT_EMTERPRETER_ENABLED -s TOTAL_MEMORY=16MB  -s PRECISE_F32=2 
:To skip font loading so it needs no resource files or zlib, add  -DC_NO_ZLIB
SET CUSTOM_FLAGS= -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DPLATFORM_HTML5 -DRT_USE_SDL_AUDIO -DRT_JPG_SUPPORT ^
-DRT_PNG_SUPPORT -DC_GL_MODE -s LEGACY_GL_EMULATION=1 -DPLATFORM_HTML5  -s ALLOW_MEMORY_GROWTH=1 ^
-Wno-c++11-compat-deprecated-writable-strings --ignore-dynamic-linking --memory-init-file 0 ^
-Wno-switch -Wno-writable-strings -Wno-shift-negative-value -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap']

:unused:   -s FULL_ES2=1 --emrun

IF %USE_HTML5_CUSTOM_MAIN% EQU 1 (
:add this define so we'll manually call mainf from the html later instead of it being auto
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -DRT_HTML5_USE_CUSTOM_MAIN -s EXPORTED_FUNCTIONS=['_mainf','_PROTON_SystemMessage','_PROTON_GUIMessage'] 
SET FINAL_EXTENSION=js
) else (
SET FINAL_EXTENSION=html
)

:-s EMTERPRETIFY=1  -s EMTERPRETIFY_ASYNC=1 -DRT_EMTERPRETER_ENABLED -s WASM=1
IF %DEBUG% EQU 0 (
echo Compiling in release mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -O2 -DNDEBUG
) else (
echo Compiling in debug mode
:removed -s SAFE_HEAP=1 , causes alignment error with FMOD  -g4 
set EMCC_DEBUG=1
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -D_DEBUG -s GL_UNSAFE_OPTS=0 -s SAFE_HEAP=1 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s EXCEPTION_DEBUG=1 -s STACK_OVERFLOW_CHECK=2 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s ALIASING_FUNCTION_POINTERS=0 --emrun 
)

SET INCLUDE_DIRS=-I%SHARED% -I%APP% -I../../shared/util/boost -I../../shared/ClanLib-2.0/Sources -I../../shared/Network/enet/include ^
-I%ZLIBPATH% -I../../shared/html5

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
%ZLIB_SRC% %JPG_SRC% %PNG_SRC% %PARTICLE_SRC% -o temp.bc

:../../shared/html5/fmodstudio/api/studio/lib/fmodstudio.bc
call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%APP_SRC% %SRC% %COMPONENT_SRC% temp.bc ../../shared/html5/fmodstudio/api/lowlevel/lib/fmod.bc --exclude-file .svn ^
--preload-file ../bin/interface@interface/ --preload-file ../bin/audio@audio/ --preload-file ../bin/dink_html5@dink/ --js-library %SHARED%\html5\SharedJSLIB.js -o %APP_NAME%.%FINAL_EXTENSION%

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
