#objective was to make this include and define the same stuff as the mvc project while keeping the CMakeLists on date with the one in linux/
#you can ignore this file if you're editing your project on Microsoft Visual Studio and not Visual Studio Code.

cmake_minimum_required(VERSION 2.6)
project(RTConsole)

include(../rtconsole.cmake)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(SOURCES
    ../source/main.cpp
    ../source/App.cpp
    ${SHARED_SOURCES}
    ../../shared/win/WinUtils.cpp
)

#CMAKE_EXPORT_COMPILE_COMMANDS is set by rtconsole.cmake above
set(CMAKE_BUILD_TYPE "Release")

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-ggdb -D_DEBUG)
ENDIF()

IF (CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions(-O3 -DNDEBUG)
ENDIF()

add_executable(RTConsole ${SOURCES})
target_link_libraries(RTConsole ws2_32.lib Winmm.lib)
