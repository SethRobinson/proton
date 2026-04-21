#objective was to make this include and define the same stuff as the mvc project while keeping the CMakeLists on date with the one in linux/
#you can ignore this file if you're editing your project on Microsoft Visual Studio and not Visual Studio Code.

cmake_minimum_required(VERSION 2.6)

project(RTConsole)

add_definitions(-D_CONSOLE -DBOOST_ALL_NO_LIB- -DC_NO_ZLIB -D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_BUILD_TYPE "Release")

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    IF (MSVC)
        add_definitions(/Zi -D_DEBUG)
    ELSE()
        #gcc or etc
        add_definitions(-ggdb -D_DEBUG)
    ENDIF()
ENDIF()

IF (CMAKE_BUILD_TYPE MATCHES "Release")
    IF (MSVC)
        add_definitions(/O2 -DNDEBUG) 
    ELSE()
        #gcc or etc
        add_definitions(-O3 -DNDEBUG)
    ENDIF()
ENDIF()

include_directories(../../shared)
include_directories(../../shared/util/boost)
include_directories(../../shared/ClanLib-2.0/Sources)

set(SOURCES
    ../source/main.cpp
    ../source/App.cpp

    ../../shared/PlatformSetup.cpp
    ../../shared/util/MiscUtils.cpp
    ../../shared/util/ResourceUtils.cpp
    ../../shared/util/CRandom.cpp
    ../../shared/util/MathUtils.cpp
    ../../shared/util/Variant.cpp

    ../../shared/Manager/VariantDB.cpp
    ../../shared/win/WinUtils.cpp
    ../../shared/util/TextScanner.cpp

    ../../shared/ClanLib-2.0/Sources/Core/Math/angle.cpp
    ../../shared/ClanLib-2.0/Sources/Core/Math/vec2.cpp
    ../../shared/ClanLib-2.0/Sources/Core/Math/vec3.cpp
    
    ../../shared/FileSystem/FileSystem.cpp
    ../../shared/FileSystem/FileManager.cpp
    ../../shared/FileSystem/StreamingInstance.cpp
    ../../shared/FileSystem/StreamingInstanceFile.cpp
)

add_executable(RTConsole ${SOURCES})

target_link_libraries(RTConsole ws2_32.lib Winmm.lib)