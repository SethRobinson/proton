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

project(RTDink)

include("${VITASDK}/share/vita.cmake" REQUIRED)
include(../../shared/vita/Proton.cmake)

set(VITA_APP_NAME "RTDink")
set(VITA_TITLEID  "RTDINK123")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

proton_include_components(UnpackArchiveComponent)
proton_include_all_components()
proton_use_jpeg_support()
proton_use_linearparticles()
proton_use_sdl_audio()
proton_use_zipfilesystem()
proton_use_ad_framework()

proton_include_gamepad(Vita iCade)

set(APP "../source")
set(DINK "../source/dink")
set(COMPONENT "../source/Component")
set(GUI "../source/GUI")

proton_set_sources("${APP}/App.cpp" "${APP}/video_gl.cpp"
                   "${DINK}/dink.cpp" "${DINK}/FFReader.cpp" "${DINK}/misc_util.cpp" "${DINK}/ScriptAccelerator.cpp" "${DINK}/update_frame.cpp"
                   "${GUI}/AboutMenu.cpp" "${GUI}/BrowseMenu.cpp" "${GUI}/DebugMenu.cpp" "${GUI}/DMODInstallMenu.cpp" "${GUI}/DMODMenu.cpp" "${GUI}/EnterURLMenu.cpp" "${GUI}/ExpiredMenu.cpp" "${GUI}/GameMenu.cpp" "${GUI}/LoadMenu.cpp" "${GUI}/LogMenu.cpp" "${GUI}/MainMenu.cpp" "${GUI}/OptionsMenu.cpp" "${GUI}/pauseMenu.cpp" "${GUI}/PopUpMenu.cpp" "${GUI}/QuickTipMenu.cpp" "${GUI}/ReadTextMenu.cpp"
                   "${COMPONENT}/ActionButtonComponent.cpp" "${COMPONENT}/CursorComponent.cpp" "${COMPONENT}/DragControlComponent.cpp" "${COMPONENT}/FPSControlComponent.cpp" "${COMPONENT}/InventoryComponent.cpp")

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)