#this particular CMakeLists.txt does it all here, but more advanced samples do it in a more elegant way, check RTBareBones for that example

cmake_minimum_required (VERSION 2.6)

ENABLE_LANGUAGE(C)

project (RTConsole)

include(../rtconsole.cmake)

# additional compiler flags -DSIMPLE_PROFILING_ACTIVE
add_definitions(-D_CONSOLE -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DC_NO_ZLIB -DRTLINUX)

#CMAKE_RUNTIME_OUTPUT_DIRECTORY and CMAKE_EXPORT_COMPILE_COMMANDS are set by rtconsole.cmake above
set(CMAKE_BUILD_TYPE "Release")

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
add_definitions(-ggdb -D_DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

IF (CMAKE_BUILD_TYPE MATCHES "Release")
add_definitions(-O3 -DNDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")

include_directories(../../shared/Network/enet/include)
include_directories(/usr/include/mysql)

add_executable(RTConsole 
    ../source/main.cpp
    ../source/App.cpp

    ${SHARED_SOURCES}

    ../../shared/linux/LinuxUtils.cpp
)

link_directories(/usr/lib/mysql) 
target_link_libraries(RTConsole rt)