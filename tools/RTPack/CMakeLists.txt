cmake_minimum_required(VERSION 2.6)

project(RTPack)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  set(PVR_ARCH_PATH "Linux_x86_64")
else(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  set(PVR_ARCH_PATH "Linux_x86_32")
endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
set(PVRTEXLIB_PATH "${CMAKE_SOURCE_DIR}/PVRTexLib/${PVR_ARCH_PATH}/libPVRTexLib.a")
if(NOT EXISTS ${PVRTEXLIB_PATH})
  message(FATAL_ERROR "Required archive ${PVRTEXLIB_PATH} not found.\nInstall PVRTexLib under ${CMAKE_SOURCE_DIR} in order to proceed.")
endif(NOT EXISTS ${PVRTEXLIB_PATH})


set(SHARED "../../shared")

add_definitions(-DRTLINUX -DBOOST_ALL_NO_LIB -DC_GL_MODE -DCLANLIB_1)

include_directories("${SHARED}")
include_directories("${SHARED}/util/boost")

include(FindPkgConfig)
pkg_check_modules(CLANCORE REQUIRED clanCore-1.0)
pkg_check_modules(CLANDISPLAY REQUIRED clanDisplay-1.0)

if(CLANCORE_FOUND)
  # link_directories needs to be used BEFORE the target is created
  link_directories(${CLANCORE_LIBRARY_DIRS})
endif(CLANCORE_FOUND)

if(CLANDISPLAY_FOUND)
  # link_directories needs to be used BEFORE the target is created
  link_directories(${CLANDISPLAY_LIBRARY_DIRS})
endif(CLANDISPLAY_FOUND)

add_executable(RTPack ClanlibUtils.cpp FontPacker.cpp main.cpp TexturePacker.cpp "${SHARED}/util/MiscUtils.cpp" "${SHARED}/util/ResourceUtils.cpp" "${SHARED}/linux/LinuxUtils.cpp")

if(CLANCORE_FOUND)
  include_directories(${CLANCORE_INCLUDE_DIRS})
  target_link_libraries(RTPack ${CLANCORE_LIBRARIES})
endif(CLANCORE_FOUND)

if(CLANDISPLAY_FOUND)
  include_directories(${CLANDISPLAY_INCLUDE_DIRS})
  target_link_libraries(RTPack ${CLANDISPLAY_LIBRARIES})
endif(CLANDISPLAY_FOUND)


find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_link_libraries(RTPack ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

# Configure PVRTexLib for inclusion
add_library(pvrtexlib STATIC IMPORTED)
set_target_properties(pvrtexlib PROPERTIES IMPORTED_LOCATION "${PVRTEXLIB_PATH}")
target_link_libraries(RTPack pvrtexlib)


target_link_libraries(RTPack rt)
