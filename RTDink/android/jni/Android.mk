LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := RTDink
SHARED := ../../../shared
APP := ../../source
LOCAL_ARM_MODE := arm

COMPPATH := ../../../shared/Entity
CLANMATH := $(SHARED)/ClanLib-2.0/Sources/Core/Math
ZLIBPATH := $(SHARED)/util/zlib
PPATH := ../../../shared/Renderer/linearparticle/sources
PNGSRC :=  $(SHARED)/Irrlicht/source/Irrlicht/libpng
JPGSRC :=  $(SHARED)/Irrlicht/source/Irrlicht/jpeglib
LZMASRC :=  $(SHARED)/Irrlicht/source/Irrlicht/lzma

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

#release flags
#-DRT_CHARTBOOST_ENABLED -DRT_MOGA_ENABLED
SHARED_FLAGS := -DANDROID_NDK -DBUILD_ANDROID -DNDEBUG -DRT_JPG_SUPPORT -DRT_PNG_SUPPORT

#debug flags
#-DRT_CHARTBOOST_ENABLED -DRT_MOGA_ENABLED
#SHARED_FLAGS := -DANDROID_NDK -DBUILD_ANDROID -D_DEBUG -DRT_JPG_SUPPORT

LOCAL_CFLAGS := -DGC_BUILD_ANDROID $(SHARED_FLAGS)
LOCAL_CPPFLAGS := -DGC_BUILD_C $(SHARED_FLAGS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SHARED) \
 $(LOCAL_PATH)/$(APP) $(LOCAL_PATH)/$(SHARED)/ClanLib-2.0/Sources $(LOCAL_PATH)/$(SHARED)/util/boost
                
LOCAL_SRC_FILES := \
	$(SHARED)/PlatformSetup.cpp $(SHARED)/android/AndroidUtils.cpp ../temp_final_cpp_src/AndroidApp.cpp $(SHARED)/Audio/AudioManagerAndroid.cpp \
\
$(SHARED)/Audio/AudioManager.cpp \
$(CLANMATH)/angle.cpp $(CLANMATH)/mat3.cpp $(CLANMATH)/mat4.cpp $(CLANMATH)/rect.cpp $(CLANMATH)/vec2.cpp $(CLANMATH)/vec3.cpp $(CLANMATH)/vec4.cpp \
$(SHARED)/Entity/Entity.cpp $(SHARED)/Entity/Component.cpp $(SHARED)/GUI/RTFont.cpp $(SHARED)/Manager/Console.cpp $(SHARED)/FileSystem/FileManager.cpp \
$(SHARED)/Manager/GameTimer.cpp $(SHARED)/Manager/MessageManager.cpp $(SHARED)/Manager/ResourceManager.cpp $(SHARED)/Manager/VariantDB.cpp $(SHARED)/Math/rtPlane.cpp \
$(SHARED)/Math/rtRect.cpp $(SHARED)/Renderer/RenderBatcher.cpp $(SHARED)/Renderer/SoftSurface.cpp $(SHARED)/Renderer/Surface.cpp $(SHARED)/Renderer/SurfaceAnim.cpp \
$(SHARED)/util/CRandom.cpp $(SHARED)/util/GLESUtils.cpp $(SHARED)/util/MathUtils.cpp $(SHARED)/util/MiscUtils.cpp $(SHARED)/util/RenderUtils.cpp $(SHARED)/util/ResourceUtils.cpp \
$(SHARED)/util/Variant.cpp $(SHARED)/util/boost/libs/signals/src/connection.cpp $(SHARED)/util/boost/libs/signals/src/named_slot_map.cpp $(SHARED)/util/boost/libs/signals/src/signal_base.cpp \
$(SHARED)/util/boost/libs/signals/src/slot.cpp $(SHARED)/util/boost/libs/signals/src/trackable.cpp $(SHARED)/BaseApp.cpp $(SHARED)/FileSystem/FileSystem.cpp $(SHARED)/FileSystem/FileSystemZip.cpp \
$(SHARED)/util/unzip/unzip.c $(SHARED)/util/unzip/ioapi.c $(SHARED)/util/TextScanner.cpp $(SHARED)/Entity/EntityUtils.cpp \
$(SHARED)/Network/NetHTTP.cpp $(SHARED)/Network/NetSocket.cpp $(SHARED)/Network/NetUtils.cpp $(SHARED)/FileSystem/StreamingInstance.cpp \
$(SHARED)/FileSystem/StreamingInstanceZip.cpp $(SHARED)/FileSystem/StreamingInstanceFile.cpp $(SHARED)/util/archive/TarHandler.cpp $(SHARED)/util/bzip2/blocksort.c \
$(SHARED)/util/bzip2/bzlib.c $(SHARED)/util/bzip2/compress.c $(SHARED)/util/bzip2/crctable.c $(SHARED)/util/bzip2/decompress.c $(SHARED)/util/bzip2/huffman.c \
$(SHARED)/util/bzip2/randtable.c \
\
$(SHARED)/Gamepad/GamepadManager.cpp $(SHARED)/Gamepad/Gamepad.cpp $(SHARED)/Gamepad/GamepadiCade.cpp $(SHARED)/Gamepad/GamepadProvider.cpp $(SHARED)/Gamepad/GamepadProvideriCade.cpp \
$(SHARED)/Gamepad/GamepadProviderMoga.cpp $(SHARED)/Gamepad/GamepadMoga.cpp \
\
$(SHARED)/Manager/AdManager.cpp $(SHARED)/Ad/AdProvider.cpp $(SHARED)/Ad/AdProviderChartBoost.cpp \
\
$(SHARED)/Renderer/JPGSurfaceLoader.cpp \
\
$(JPGSRC)/jcapimin.c $(JPGSRC)/jcapistd.c $(JPGSRC)/jccoefct.c $(JPGSRC)/jccolor.c $(JPGSRC)/jcdctmgr.c $(JPGSRC)/jchuff.c $(JPGSRC)/jcinit.c $(JPGSRC)/jcmainct.c \
$(JPGSRC)/jcmarker.c $(JPGSRC)/jcmaster.c $(JPGSRC)/jcomapi.c $(JPGSRC)/jcparam.c $(JPGSRC)/jcphuff.c $(JPGSRC)/jcprepct.c $(JPGSRC)/jcsample.c $(JPGSRC)/jctrans.c \
$(JPGSRC)/jdapimin.c $(JPGSRC)/jdapistd.c $(JPGSRC)/jdatadst.c $(JPGSRC)/jdatasrc.c $(JPGSRC)/jdcoefct.c $(JPGSRC)/jdcolor.c $(JPGSRC)/jddctmgr.c \
$(JPGSRC)/jdhuff.c $(JPGSRC)/jdinput.c $(JPGSRC)/jdmainct.c $(JPGSRC)/jdmarker.c $(JPGSRC)/jdmaster.c $(JPGSRC)/jdmerge.c $(JPGSRC)/jdphuff.c $(JPGSRC)/jdpostct.c \
$(JPGSRC)/jdsample.c $(JPGSRC)/jdtrans.c $(JPGSRC)/jerror.c $(JPGSRC)/jfdctflt.c $(JPGSRC)/jfdctfst.c $(JPGSRC)/jfdctint.c $(JPGSRC)/jidctflt.c $(JPGSRC)/jidctfst.c \
$(JPGSRC)/jidctint.c $(JPGSRC)/jidctred.c $(JPGSRC)/jmemmgr.c $(JPGSRC)/jmemnobs.c $(JPGSRC)/jquant1.c $(JPGSRC)/jquant2.c $(JPGSRC)/jutils.c \
\
$(PNGSRC)/png.c $(PNGSRC)/pngerror.c $(PNGSRC)/pnggccrd.c $(PNGSRC)/pngget.c $(PNGSRC)/pngmem.c $(PNGSRC)/pngpread.c $(PNGSRC)/pngread.c \
$(PNGSRC)/pngrio.c $(PNGSRC)/pngrtran.c $(PNGSRC)/pngrutil.c $(PNGSRC)/pngset.c $(PNGSRC)/pngtrans.c $(PNGSRC)/pngvcrd.c $(PNGSRC)/pngwio.c $(PNGSRC)/pngwtran.c \
\
$(COMPPATH)/Button2DComponent.cpp $(COMPPATH)/FilterInputComponent.cpp $(COMPPATH)/FocusInputComponent.cpp $(COMPPATH)/FocusRenderComponent.cpp $(COMPPATH)/FocusUpdateComponent.cpp \
$(COMPPATH)/HTTPComponent.cpp $(COMPPATH)/InputTextRenderComponent.cpp $(COMPPATH)/InterpolateComponent.cpp $(COMPPATH)/OverlayRenderComponent.cpp $(COMPPATH)/ProgressBarComponent.cpp \
$(COMPPATH)/RectRenderComponent.cpp $(COMPPATH)/ScrollBarRenderComponent.cpp $(COMPPATH)/ScrollComponent.cpp $(COMPPATH)/TapSequenceDetectComponent.cpp $(COMPPATH)/TextBoxRenderComponent.cpp \
$(COMPPATH)/TextRenderComponent.cpp $(COMPPATH)/TouchStripComponent.cpp $(COMPPATH)/TrailRenderComponent.cpp $(COMPPATH)/TyperComponent.cpp $(COMPPATH)/UnderlineRenderComponent.cpp \
$(COMPPATH)/TouchHandlerComponent.cpp $(COMPPATH)/SelectButtonWithCustomInputComponent.cpp $(COMPPATH)/CustomInputComponent.cpp $(COMPPATH)/SliderComponent.cpp $(COMPPATH)/RenderClipComponent.cpp \
$(COMPPATH)/UnpackArchiveComponent.cpp $(COMPPATH)/ArcadeInputComponent.cpp $(COMPPATH)/EmitVirtualKeyComponent.cpp $(COMPPATH)/RenderScissorComponent.cpp \
\
\
$(PPATH)/L_Defination.cpp $(PPATH)/L_DroppingEffect.cpp $(PPATH)/L_EffectEmitter.cpp $(PPATH)/L_ExplosionEffect.cpp $(PPATH)/L_MotionController.cpp $(PPATH)/L_Particle.cpp \
$(PPATH)/L_ParticleEffect.cpp $(PPATH)/L_ParticleMem.cpp $(PPATH)/L_ParticleSystem.cpp $(PPATH)/L_ShootingEffect.cpp $(PPATH)/L_EffectManager.cpp \
\
$(APP)/App.cpp $(APP)/Component/ActionButtonComponent.cpp $(APP)/Component/CursorComponent.cpp $(APP)/Component/DragControlComponent.cpp \
$(APP)/Component/FPSControlComponent.cpp $(APP)/Component/InventoryComponent.cpp $(APP)/dink/dink.cpp $(APP)/dink/FFReader.cpp $(APP)/dink/misc_util.cpp $(APP)/dink/ScriptAccelerator.cpp \
$(APP)/video_gl.cpp \
$(APP)/GUI/AboutMenu.cpp $(APP)/GUI/BrowseMenu.cpp $(APP)/GUI/DebugMenu.cpp $(APP)/GUI/DMODInstallMenu.cpp \
$(APP)/GUI/EnterURLMenu.cpp $(APP)/GUI/DMODMenu.cpp $(APP)/GUI/GameMenu.cpp $(APP)/GUI/LoadMenu.cpp $(APP)/GUI/LogMenu.cpp $(APP)/GUI/MainMenu.cpp $(APP)/GUI/OptionsMenu.cpp \
$(APP)/GUI/PauseMenu.cpp $(APP)/GUI/PopUpMenu.cpp $(APP)/GUI/QuickTipMenu.cpp $(APP)/GUI/ReadTextMenu.cpp $(APP)/GUI/ExpiredMenu.cpp 


LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

include $(BUILD_SHARED_LIBRARY)
