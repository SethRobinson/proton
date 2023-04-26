cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

ENABLE_LANGUAGE(C)

set(CMAKE_BUILD_TYPE "Debug")

project(RTBareBones)

include("${VITASDK}/share/vita.cmake" REQUIRED)
include(../../shared/psp2/Proton.cmake)

set(VITA_APP_NAME "RTBareBones")
set(VITA_TITLEID  "PROTO4321")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

proton_include_components(FocusInputComponent)
proton_include_components(CustomInputComponent ArcadeInputComponent)

set(APP "../source")
proton_set_sources("${APP}/App.cpp")

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)

add_custom_target(send
        COMMAND echo destroy | nc 192.168.1.199 1338
        COMMAND curl -T eboot.bin ftp://192.168.1.199:1337/ux0:/app/${VITA_TITLEID}/
        COMMAND echo launch ${VITA_TITLEID} | nc 192.168.1.199 1338
)