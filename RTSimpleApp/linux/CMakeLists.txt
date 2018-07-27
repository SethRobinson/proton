cmake_minimum_required(VERSION 2.6)

ENABLE_LANGUAGE(C)

#if(ISRASPBERRYPIE)
OPTION(RASPBERRYPI_GLES11 "Compile for Raspberry PI GLES" ON)
#endif(ISRASPBERRYPIE)

set(CMAKE_BUILD_TYPE "Release") #use Release or Debug
project(RTSimpleApp)

include(../../shared/linux/Proton.cmake)

add_definitions(-DRT_LINUX -RT_RASPBERRY)

proton_include_all_components()
proton_use_jpeg_support()
proton_use_sdl_audio()
proton_use_linearparticles()

set(APP "../source")
include_directories("${APP}")
proton_set_sources("${APP}/App.cpp" "${APP}/Component/ParticleTestComponent.cpp" "${APP}/GUI/DebugMenu.cpp" "${APP}/GUI/EnterNameMenu.cpp" "${APP}/GUI/MainMenu.cpp" "${APP}/GUI/ParticleTestMenu.cpp" "${APP}/GUI/AboutMenu.cpp" "${APP}/GUI/TouchTestMenu.cpp" "${APP}/Component/TouchTestComponent.cpp")
