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
project(RTSimpleApp)

include(../../shared/linux/Proton.cmake)

proton_include_all_components()
proton_use_jpeg_support()
proton_use_sdl_audio()
proton_use_linearparticles()

set(APP "../source")
include_directories("${APP}")
proton_set_sources("${APP}/App.cpp" "${APP}/Component/ParticleTestComponent.cpp" "${APP}/GUI/DebugMenu.cpp" "${APP}/GUI/EnterNameMenu.cpp" "${APP}/GUI/MainMenu.cpp" "${APP}/GUI/ParticleTestMenu.cpp" "${APP}/GUI/AboutMenu.cpp" "${APP}/GUI/TouchTestMenu.cpp" "${APP}/Component/TouchTestComponent.cpp")
