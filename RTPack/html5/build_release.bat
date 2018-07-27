IMPORTANT:  Don't use this, the html5 dir from RTBareBones or RTSimpleApp use a new format which is much better and I'm too lazy to update this project so far

call ../app_info_setup.bat

:Your root emscripten dir should be in the path so this works:
call emsdk_env.bat

SET SHARED=..\..\shared

SET APP=..\source

SET COMPPATH=%SHARED%\Entity
SET CLANMATH=%SHARED%\ClanLib-2.0\Sources\Core\Math
SET ZLIBPATH=%SHARED%\util\zlib

:PNGSRC :=  $(SHARED)\Irrlicht\source\Irrlicht\libpng

:JPGSRC :=  $(SHARED)\Irrlicht\source\Irrlicht\jpeglib

:LZMASRC :=  $(SHARED)\Irrlicht\source\Irrlicht\lzma

:PPATH := $(SHARED)\Renderer\linearparticle\sources

SET CUSTOM_FLAGS=-D_CONSOLE -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DC_NO_ZLIB -DPLATFORM_HTML5


SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -O1
SET INCLUDE_DIRS=-I%SHARED% -I%APP% -I../../shared/util/boost -I../../shared/ClanLib-2.0/Sources -I../../shared/Network/enet/include


call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
../source/main.cpp ^
../source/App.cpp ^
%SHARED%\PlatformSetup.cpp %SHARED%\util\MiscUtils.cpp %SHARED%\util\ResourceUtils.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\CRandom.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\Variant.cpp ^
%SHARED%\Manager/VariantDB.cpp %SHARED%\linux/LinuxUtils.cpp ^
..\..\shared\ClanLib-2.0\Sources\Core\Math\angle.cpp ^
..\..\shared\ClanLib-2.0\Sources\Core\Math\vec2.cpp ^
..\..\shared\ClanLib-2.0\Sources\Core\Math\vec3.cpp ^
..\..\shared\util\boost\libs\signals\src\connection.cpp ^
..\..\shared\util\boost\libs\signals\src\named_slot_map.cpp ^
..\..\shared\util\boost\libs\signals\src\signal_base.cpp ^
..\..\shared\util\boost\libs\signals\src\slot.cpp ^
..\..\shared\util\boost\libs\signals\src\trackable.cpp ^
..\..\shared\util\TextScanner.cpp ^
..\..\shared\FileSystem\FileSystem.cpp ^
..\..\shared\FileSystem\FileManager.cpp ^
..\..\shared\FileSystem\StreamingInstance.cpp ^
..\..\shared\FileSystem\StreamingInstanceFile.cpp ^
--embed-file ../bin/interface@interface/ -o %APP_NAME%.html



pause
