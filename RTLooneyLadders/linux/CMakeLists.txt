cmake_minimum_required(VERSION 2.6)

project(rtlooneyladders)

include(../../shared/linux/Proton.cmake)

proton_include_all_components()
proton_use_sdl_audio()
proton_use_linearparticles()
proton_include_gamepad("iCade")

set(APP "../source")
include_directories("${APP}")
proton_set_sources("${APP}/App.cpp" "${APP}/Component/BuildingComponent.cpp" "${APP}/Component/CharComponent.cpp" "${APP}/Component/CharManagerComponent.cpp" "${APP}/Component/ExplosionComponent.cpp" "${APP}/Component/OverlayRenderComponentSpy.cpp" "${APP}/GUI/AboutMenu.cpp" "${APP}/GUI/GameMenu.cpp" "${APP}/GUI/MainMenu.cpp")
