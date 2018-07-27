#this particular CMakeLists.txt does it all here, but more advanced samples do it in a more elegant way, check RTBareBones for that example

cmake_minimum_required (VERSION 2.6)

ENABLE_LANGUAGE(C)

project (RTConsole)
# additional compiler flags -DSIMPLE_PROFILING_ACTIVE
add_definitions(-D_CONSOLE -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DC_NO_ZLIB -DRTLINUX)

set(CMAKE_BUILD_TYPE "Release")

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
add_definitions(-ggdb -D_DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

IF (CMAKE_BUILD_TYPE MATCHES "Release")
add_definitions(-O3 -DNDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")

include_directories(../source)
include_directories(../../shared ../../shared/Network/enet/include)
include_directories(../../shared/util/boost ../../shared/ClanLib-2.0/Sources)
include_directories(/usr/include/mysql)


add_executable(RTConsole 
../source/main.cpp
../source/App.cpp


 ../../shared/PlatformSetup.cpp
 ../../shared/util/MiscUtils.cpp
 ../../shared/util/ResourceUtils.cpp
 ../../shared/util/CRandom.cpp
 ../../shared/util/MathUtils.cpp
 ../../shared/util/Variant.cpp

 ../../shared/Manager/VariantDB.cpp
 ../../shared/linux/LinuxUtils.cpp
 
 ../../shared/ClanLib-2.0/Sources/Core/Math/angle.cpp
 ../../shared/ClanLib-2.0/Sources/Core/Math/vec2.cpp
 ../../shared/ClanLib-2.0/Sources/Core/Math/vec3.cpp

 ../../shared/util/boost/libs/signals/src/connection.cpp
 ../../shared/util/boost/libs/signals/src/named_slot_map.cpp
 ../../shared/util/boost/libs/signals/src/signal_base.cpp
 ../../shared/util/boost/libs/signals/src/slot.cpp
 ../../shared/util/boost/libs/signals/src/trackable.cpp
../../shared/util/TextScanner.cpp

../../shared/FileSystem/FileSystem.cpp
../../shared/FileSystem/FileManager.cpp
../../shared/FileSystem/StreamingInstance.cpp
../../shared/FileSystem/StreamingInstanceFile.cpp
 )

link_directories(/usr/lib/mysql) 
target_link_libraries(RTConsole rt)
