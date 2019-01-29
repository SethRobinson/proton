cmake_minimum_required(VERSION 2.6)

ENABLE_LANGUAGE(C)

include(CheckIncludeFile)
set (CMAKE_REQUIRED_INCLUDES "/opt/vc/include")
CHECK_INCLUDE_FILE(bcm_host.h ISRASPBERRYPIE)

if(ISRASPBERRYPIE)
OPTION(RASPBERRYPI_GLES11 "Compile for Raspberry PI GLES" ON)
message(STATUS "Detected as PI")
add_definitions(-DRT_RASPBERRY)
endif(ISRASPBERRYPIE)

add_definitions(-DRT_LINUX)

set(CMAKE_BUILD_TYPE "Release") #use Release or Debug

project(RTBareBones)
include(../../shared/linux/Proton.cmake)

proton_include_components(FocusInputComponent)
proton_include_components(CustomInputComponent ArcadeInputComponent)

set(APP "../source")
proton_set_sources("${APP}/App.cpp")
target_link_libraries(RTBareBones)
