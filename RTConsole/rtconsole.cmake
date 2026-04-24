#shared CMake settings for RTConsole so windows & linux are easier to be updated.
#modify this for everything that needs sharing, platform specific configuration
#should happen at the linux/ or windows/ directories.

add_definitions(-D_CONSOLE -DBOOST_ALL_NO_LIB -DC_NO_ZLIB)

#put binary in RTConsole/bin/. CMAKE_CURRENT_LIST_DIR is the dir of this .cmake (RTConsole/), so this works no matter which platform CMakeLists.txt includes us.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    #note - this always goes 1 dir too much backwards since it gets
    #loaded by the parent cmakelists which are 1 dir upwards from this
    ../source
    ../../shared
    ../../shared/util/boost
    ../../shared/ClanLib-2.0/Sources
)

set(SHARED_SOURCES
    ../../shared/PlatformSetup.cpp
    ../../shared/util/MiscUtils.cpp
    ../../shared/util/ResourceUtils.cpp
    ../../shared/util/CRandom.cpp
    ../../shared/util/MathUtils.cpp
    ../../shared/util/Variant.cpp
    ../../shared/Manager/VariantDB.cpp
    ../../shared/util/TextScanner.cpp
    ../../shared/ClanLib-2.0/Sources/Core/Math/angle.cpp
    ../../shared/ClanLib-2.0/Sources/Core/Math/vec2.cpp
    ../../shared/ClanLib-2.0/Sources/Core/Math/vec3.cpp
    ../../shared/FileSystem/FileSystem.cpp
    ../../shared/FileSystem/FileManager.cpp
    ../../shared/FileSystem/StreamingInstance.cpp
    ../../shared/FileSystem/StreamingInstanceFile.cpp
)
