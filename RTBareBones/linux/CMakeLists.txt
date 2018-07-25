cmake_minimum_required(VERSION 2.6)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h ISRASPBERRYPIE)

#if(ISRASPBERRYPIE)
OPTION(RASPBERRYPI_GLES11 "Compile for Raspberry PI GLES" ON)
#endif(ISRASPBERRYPIE)

#OPTION(RASPBERRYPI_OPENGL "Compile for Rasperry PI OpenGL" ON)


set(CMAKE_BUILD_TYPE "Release") #use Release or Debug

#for raspberry directory
#link_directories("/usr/lib/arm-linux-gnueabihf")

project(rtbarebones)
include(../../shared/linux/Proton.cmake)
#include(../source)

proton_include_components(FocusInputComponent)
proton_include_components(CustomInputComponent ArcadeInputComponent)

set(APP "../source")
proton_set_sources("${APP}/App.cpp")
target_link_libraries(rtbarebones)
