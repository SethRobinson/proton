LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#disable thumb mode
LOCAL_ARM_MODE := arm
LOCAL_MODULE := rtbarebones
SHARED := ../../../shared
APP := ../../source
LOCAL_ARM_MODE := arm



COMPPATH := ../../../shared/Entity
CLANMATH := $(SHARED)/ClanLib-2.0/Sources/Core/Math
ZLIBPATH := $(SHARED)/util/zlib
PPATH := ../../../shared/Renderer/linearparticle/sources

#release flags

#LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

#debug mode, bigger files, more debug info, slow
MY_FLAGS := -D_DEBUG

#release mode, smaller files, runs faster
MY_FLAGS := -DNDEBUG

#release flags
SHARED_FLAGS := -DANDROID_NDK -DBUILD_ANDROID -DNDEBUG $(MY_FLAGS)

LOCAL_CFLAGS := -DGC_BUILD_ANDROID $(SHARED_FLAGS)
LOCAL_CPPFLAGS := -DGC_BUILD_C $(SHARED_FLAGS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SHARED) \
 $(LOCAL_PATH)/$(APP) $(LOCAL_PATH)/$(SHARED)/ClanLib-2.0/Sources $(LOCAL_PATH)/$(SHARED)/util/boost
                
LOCAL_SRC_FILES := \
	$(SHARED)/PlatformSetup.cpp ../temp_final_cpp_src/AndroidApp.cpp $(SHARED)/android/AndroidUtils.cpp $(SHARED)/Audio/AudioManager.cpp \
$(CLANMATH)/angle.cpp $(CLANMATH)/mat3.cpp $(CLANMATH)/mat4.cpp $(CLANMATH)/rect.cpp $(CLANMATH)/vec2.cpp $(CLANMATH)/vec3.cpp $(CLANMATH)/vec4.cpp \
$(SHARED)/Entity/Entity.cpp $(SHARED)/Entity/Component.cpp $(SHARED)/GUI/RTFont.cpp $(SHARED)/Manager/Console.cpp $(SHARED)/FileSystem/FileManager.cpp \
$(SHARED)/Manager/GameTimer.cpp $(SHARED)/Manager/MessageManager.cpp $(SHARED)/Manager/ResourceManager.cpp $(SHARED)/Manager/VariantDB.cpp $(SHARED)/Math/rtPlane.cpp \
$(SHARED)/Math/rtRect.cpp $(SHARED)/Renderer/RenderBatcher.cpp $(SHARED)/Renderer/SoftSurface.cpp $(SHARED)/Renderer/Surface.cpp $(SHARED)/Renderer/SurfaceAnim.cpp \
$(SHARED)/util/CRandom.cpp $(SHARED)/util/GLESUtils.cpp $(SHARED)/util/MathUtils.cpp $(SHARED)/util/MiscUtils.cpp $(SHARED)/util/RenderUtils.cpp $(SHARED)/util/ResourceUtils.cpp \
$(SHARED)/util/Variant.cpp $(SHARED)/util/boost/libs/signals/src/connection.cpp $(SHARED)/util/boost/libs/signals/src/named_slot_map.cpp $(SHARED)/util/boost/libs/signals/src/signal_base.cpp \
$(SHARED)/util/boost/libs/signals/src/slot.cpp $(SHARED)/util/boost/libs/signals/src/trackable.cpp $(SHARED)/BaseApp.cpp $(SHARED)/FileSystem/FileSystem.cpp \
$(SHARED)/FileSystem/FileSystemZip.cpp $(SHARED)/util/unzip/unzip.c $(SHARED)/util/unzip/ioapi.c $(SHARED)/FileSystem/StreamingInstance.cpp \
$(SHARED)/FileSystem/StreamingInstanceZip.cpp $(SHARED)/FileSystem/StreamingInstanceFile.cpp \
\
\
$(COMPPATH)/FocusInputComponent.cpp $(COMPPATH)/FocusUpdateComponent.cpp $(COMPPATH)/CustomInputComponent.cpp $(COMPPATH)/ArcadeInputComponent.cpp \
\
\
$(APP)/App.cpp


LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

include $(BUILD_SHARED_LIBRARY)
