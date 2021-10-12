cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

ENABLE_LANGUAGE(C)

set(CMAKE_BUILD_TYPE "Release")
add_definitions(-DPLATFORM_VITA -DRT_USING_OSMESA)

project(RTSimpleApp)

include("${VITASDK}/share/vita.cmake" REQUIRED)
include(../../shared/vita/Proton.cmake)

set(VITA_APP_NAME "RTSimpleApp")
set(VITA_TITLEID  "PROTONSDK")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

proton_include_all_components()
proton_use_jpeg_support()
proton_use_linearparticles()
proton_use_sdl_audio()

set(APP "../source")
include_directories("${APP}")
proton_set_sources("${APP}/App.cpp" "${APP}/GUI/DebugMenu.cpp" "${APP}/GUI/EnterNameMenu.cpp" "${APP}/GUI/ParticleTestMenu.cpp" "${APP}/GUI/MainMenu.cpp" "${APP}/GUI/AboutMenu.cpp" "${APP}/GUI/TouchTestMenu.cpp" "${APP}/Component/TouchTestComponent.cpp" "${APP}/Component/ParticleTestComponent.cpp")

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)