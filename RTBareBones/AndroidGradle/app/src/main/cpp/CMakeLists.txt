# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("RTAndroidApp")


set(APP "../../../../../source")
set(SHARED "../../../../../../shared")
set(COMPPATH "${SHARED}/Entity")
set(CLANMATH "${SHARED}/ClanLib-2.0/Sources/Core/Math")

set(ZLIBPATH "${SHARED}/util/zlib")
set(PPATH "${SHARED}/Renderer/linearparticle/sources")
set(PNGSRC "${SHARED}/Irrlicht/source/Irrlicht/libpng")
set(JPGSRC "${SHARED}/Irrlicht/source/Irrlicht/jpeglib")
set(LZMASRC "${SHARED}/Irrlicht/source/Irrlicht/lzma")

include_directories("${SHARED}")
include_directories("${SHARED}/util/boost")
include_directories("${SHARED}/ClanLib-2.0/Sources")
include_directories("${APP}")

add_definitions(-Wno-error=format-security -DANDROID_NDK -DBOOST_ALL_NO_LIB -DBUILD_ANDROID -DPLATFORM_ANDROID )

message("Building ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
else()
    add_definitions(-DNDEBUG)
endif()


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.



add_library( # Sets the name of the library.
        RTAndroidApp

        # Sets the library as a shared library.
        SHARED


        ${SHARED}/PlatformSetup.cpp ${SHARED}/Android/AndroidApp.cpp ${SHARED}/android/AndroidUtils.cpp ${SHARED}/Audio/AudioManager.cpp
${CLANMATH}/angle.cpp ${CLANMATH}/mat3.cpp ${CLANMATH}/mat4.cpp ${CLANMATH}/rect.cpp ${CLANMATH}/vec2.cpp ${CLANMATH}/vec3.cpp ${CLANMATH}/vec4.cpp 
${SHARED}/Entity/Entity.cpp ${SHARED}/Entity/Component.cpp ${SHARED}/GUI/RTFont.cpp ${SHARED}/Manager/Console.cpp ${SHARED}/FileSystem/FileManager.cpp 
${SHARED}/Manager/GameTimer.cpp ${SHARED}/Manager/MessageManager.cpp ${SHARED}/Manager/ResourceManager.cpp ${SHARED}/Manager/VariantDB.cpp ${SHARED}/Math/rtPlane.cpp 
${SHARED}/Math/rtRect.cpp ${SHARED}/Renderer/RenderBatcher.cpp ${SHARED}/Renderer/SoftSurface.cpp ${SHARED}/Renderer/Surface.cpp ${SHARED}/Renderer/SurfaceAnim.cpp 
${SHARED}/util/CRandom.cpp ${SHARED}/util/GLESUtils.cpp ${SHARED}/util/MathUtils.cpp ${SHARED}/util/MiscUtils.cpp ${SHARED}/util/RenderUtils.cpp ${SHARED}/util/ResourceUtils.cpp 
${SHARED}/util/Variant.cpp ${SHARED}/util/boost/libs/signals/src/connection.cpp ${SHARED}/util/boost/libs/signals/src/named_slot_map.cpp ${SHARED}/util/boost/libs/signals/src/signal_base.cpp 
${SHARED}/util/boost/libs/signals/src/slot.cpp ${SHARED}/util/boost/libs/signals/src/trackable.cpp ${SHARED}/BaseApp.cpp ${SHARED}/FileSystem/FileSystem.cpp 
${SHARED}/FileSystem/FileSystemZip.cpp ${SHARED}/util/unzip/unzip.c ${SHARED}/util/unzip/ioapi.c ${SHARED}/FileSystem/StreamingInstance.cpp 
${SHARED}/FileSystem/StreamingInstanceZip.cpp ${SHARED}/FileSystem/StreamingInstanceFile.cpp 


${COMPPATH}/FocusInputComponent.cpp ${COMPPATH}/FocusUpdateComponent.cpp ${COMPPATH}/CustomInputComponent.cpp ${COMPPATH}/ArcadeInputComponent.cpp 


${APP}/App.cpp
        
        )

# if you  need to add antoher C++ library, you can do it in the format below, then don't forget to add
# the lib name to the target_link_libraries command below as well
#add_library(lib_crypto STATIC IMPORTED)
#set_target_properties(lib_crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/${SHARED}/android/optional_src/libs/libopenssl/${ANDROID_ABI}/libcrypto.a)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#Keep in mind the order below matters!  Low level? Then it goes to the right

target_link_libraries( # Specifies the target library.
        RTAndroidApp

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} dl GLESv1_CM z)