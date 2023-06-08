#this particular CMakeLists.txt does it all here, but more advanced samples do it in a more elegant way, check RTBareBones for that example

cmake_minimum_required (VERSION 2.6)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(ISRASPBERRYPIE FALSE)
set(RASPBERRYPI_GLES11 OFF)

ENABLE_LANGUAGE(C)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h ISRASPBERRYPIE)

#if(ISRASPBERRYPIE)
OPTION(RASPBERRYPI_GLES11 "Compile for Raspberry PI GLES" ON)
#endif(ISRASPBERRYPIE)

#OPTION(RASPBERRYPI_OPENGL "Compile for Rasperry PI OpenGL" ON)

project (RTPack)
include(../../shared/linux/Proton.cmake)
# additional compiler flags -DSIMPLE_PROFILING_ACTIVE
add_definitions(-D_CONSOLE -DRTLINUX -DRT_NO_PVR)

set(APP "../source")
include_directories("${APP}")

proton_use_jpeg_support()
proton_use_png_support()

proton_set_sources_console("${APP}/App.cpp")

#add more sources
target_sources(RTPack PUBLIC 
${APP}/main.cpp
${APP}/FontPacker.cpp
${APP}/main.cpp
${APP}/TexturePacker.cpp

${PROTON_SHARED}/Renderer/JPGSurfaceLoader.cpp
 
 )

target_link_libraries(RTPack rt)
