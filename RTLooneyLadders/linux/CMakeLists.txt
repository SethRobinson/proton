cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../bin)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()



add_definitions(-DRT_LINUX)


project(RTLooneyLadders)

include(../../shared/linux/Proton.cmake)

proton_include_all_components()
proton_use_sdl_audio()
proton_use_linearparticles()
proton_include_gamepad("SDL2")

set(APP "../source")
include_directories("${APP}")
proton_set_sources("${APP}/App.cpp" "${APP}/Component/BuildingComponent.cpp" "${APP}/Component/CharComponent.cpp" 
"${APP}/Component/CharManagerComponent.cpp" "${APP}/Component/ExplosionComponent.cpp" "${APP}/Component/OverlayRenderComponentSpy.cpp"
"${APP}/GUI/AboutMenu.cpp" "${APP}/GUI/GameMenu.cpp" "${APP}/GUI/MainMenu.cpp" "${APP}/GUI/IntroMenu.cpp"  "${APP}/GUI/ControllerTestMenu.cpp")
